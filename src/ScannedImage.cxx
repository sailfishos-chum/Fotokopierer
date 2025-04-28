/*
 * Copyright (c) 2018 Frank Fischer <frank-fischer@shadow-soft.de>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see  <http://www.gnu.org/licenses/>
 */

#include "ScannedImage.hxx"

#include "Convert.hxx"
#include "ScannedImageProvider.hxx"
#include "Util.hxx"

#include <QDebug>
#include <QtGui/QImageReader>
#include <QtGui/QPixmap>

#include <opencv2/imgproc/imgproc.hpp>

namespace
{
/// The valid states of the image transformation.
///
/// The state indicates which steps of the whole image processing are
/// up-to-date. If the state of the image is requested that is not
/// up-to-date, all required transformations will be updated.
enum ImageState {
    /// Only the original image is valid.
    Original = 0,
    /// The image has been rotated.
    Rotated,
    /// The image has been cut and perspectively transformed.
    Cut,
    /// The image has been decolorized.
    Colorized,
};
}

struct ScannedImage::Data {
    /// The ID.
    QString id;

    ImageState state = Original;

    /// The original image.
    cv::Mat original;

    /// Image should be scaled.
    bool scaled;

    /// The rotation angle.
    double rotAngle = 0;
    /// The rotated image.
    cv::Mat rotated;

    /// The top left point of the perspective quadrangle.
    QPointF topleft = {0, 0};
    /// The top right point of the perspective quadrangle.
    QPointF topright = {0, 0};
    /// The bottom right point of the perspective quadrangle.
    QPointF bottomright = {0, 0};
    /// The bottom left point of the perspective quadrangle.
    QPointF bottomleft = {0, 0};

    /// The perspectively transformed (cut) image.
    cv::Mat cut;

    double contrast;
    double brightness;
    double details;

    ScannedImage::ColorMode colormode;

    /// The colorized image.
    cv::Mat colorized;

    /// Return the guessed aspect ratio of the current cut region.
    static double getAspectRatio(const QPointF& tl,
                                 const QPointF& tr,
                                 const QPointF& br,
                                 const QPointF& bl);

    QList<QPointF> autoDetectCutRect();

    const cv::Mat& getRotatedImage();
    const cv::Mat& getCutImage();
    const cv::Mat& getColorizedImage();
};

ScannedImage::ScannedImage(QObject* parent) : QObject(parent), d(new Data)
{
    d->id = ScannedImageProvider::instance->registerImage(this);
}

ScannedImage::~ScannedImage()
{
    ScannedImageProvider::instance->unregisterImage(this, d->id);
}

void ScannedImage::loadFile(const QString& fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    d->original = QImageToCvMat(reader.read());
    d->state = Original;
    d->scaled = true;
    emit originalChanged();
    emit imageChanged();
}

QString ScannedImage::originalImage() const
{
    return d->id + QLatin1String("/original");
}

QString ScannedImage::image() const
{
    return d->id;
}

void ScannedImage::setAngle(double angle)
{
    auto rotAngle = (int)angle % 360;
    if (rotAngle < 0) {
        rotAngle += 360;
    }
    if (rotAngle != d->rotAngle) {
        d->rotAngle = rotAngle;
        d->state = (ImageState)std::min((int)d->state, Rotated - 1);
        emit angleChanged();
    }
}

double ScannedImage::angle() const
{
    return d->rotAngle;
}

void ScannedImage::setContrast(double contrast)
{
    contrast = std::max(0.0, std::min(1.0, contrast));
    if (contrast != d->contrast) {
        d->contrast = contrast;
        d->state = (ImageState)std::min((int)d->state, Colorized - 1);
        emit contrastChanged();
    }
}

double ScannedImage::contrast() const
{
    return d->contrast;
}

void ScannedImage::setBrightness(double brightness)
{
    brightness = std::max(0.0, std::min(1.0, brightness));
    if (brightness != d->brightness) {
        d->brightness = brightness;
        d->state = (ImageState)std::min((int)d->state, Colorized - 1);
        emit brightnessChanged();
    }
}

double ScannedImage::brightness() const
{
    return d->brightness;
}

void ScannedImage::setDetails(double details)
{
    details = std::max(0.0, std::min(1.0, details));
    if (details != d->details) {
        d->details = details;
        d->state = (ImageState)std::min((int)d->state, Colorized - 1);
        emit detailsChanged();
    }
}

double ScannedImage::details() const
{
    return d->details;
}

void ScannedImage::setColorMode(ColorMode colorMode)
{
    if (colorMode != d->colormode) {
        d->colormode = colorMode;
        d->state = (ImageState)std::min((int)d->state, Colorized - 1);
        emit colorModeChanged();
    }
}

ScannedImage::ColorMode ScannedImage::colorMode() const
{
    return d->colormode;
}

bool ScannedImage::setCutBox(const QPointF& topleft,
                             const QPointF& topright,
                             const QPointF& bottomright,
                             const QPointF& bottomleft)
{
    static Util util;

    if (!util.isConvex(topleft, topright, bottomright, bottomleft)) {
        return false;
    }
    d->topleft = topleft;
    d->topright = topright;
    d->bottomright = bottomright;
    d->bottomleft = bottomleft;
    d->state = (ImageState)std::min((int)d->state, Cut - 1);
    return true;
}

QVariantList ScannedImage::autoDetectCutRect()
{
    auto pts = d->autoDetectCutRect();
    QVariantList lst;
    for (auto p : pts) {
        lst << p;
    }
    return lst;
}

QImage ScannedImage::getOriginal()
{
    return cvMatToQImage(d->original);
}

QImage ScannedImage::getRotated()
{
    return cvMatToQImage(d->getRotatedImage());
}

QImage ScannedImage::getCut()
{
    return cvMatToQImage(d->getCutImage());
}

QImage ScannedImage::getColorized()
{
    return cvMatToQImage(d->getColorizedImage());
}

/// Return the aspect ratio of the original image.
///
/// The aspect ratio is guessed from the four projection points
/// \f$(x_{i}, y_{i})\f$ for $i \in \{tl,tr,br,bl\}\f$ as follows.
///
/// Given the four points on the z=1 plane and the camera at (0,0,0),
/// the original points are at \f$ \alpha_{i} p_{i} = \alpha_i (x_i,
/// y_i, 1) \f$ and so on. As the four points form a parallelogram the
/// satisfy the three equations \f$ \alpha_{br} p_{br} = \alpha_{tl}
/// p_{bl} + \alpha_{tr} p_{tr} - \alpha_{tl} p_{tl} \f$. Solving
/// these equations give a linear representation of the four points
/// \f$ p_i = \beta q_i \f$ for some fixed points \f$ q_i \f$. The
/// aspect ratio can then be computed as \f$ \frac{\|q_{tr} -
/// q_{tl}\|}{\|q_{bl} - q_{tl}\|} \f$, which is the value returned by
/// this function.
double ScannedImage::Data::getAspectRatio(const QPointF& tl,
                                          const QPointF& tr,
                                          const QPointF& br,
                                          const QPointF& bl)
{
    double a_tl = (br.x() - tl.x());
    double a_tr = (tr.x() - br.x());
    double a_bl = (bl.x() - br.x());

    double b_tl = (br.y() - tl.y());
    double b_tr = (tr.y() - br.y());
    double b_bl = (bl.y() - br.y());

    // pivot, maybe swap equations
    if (std::abs(a_tl) < std::abs(b_tl)) {
        std::swap(a_tl, b_tl);
        std::swap(a_tr, b_tr);
        std::swap(a_bl, b_bl);
    }

    double c_bl = b_bl - b_tl / a_tl * a_bl;
    double c_tr = b_tr - b_tl / a_tl * a_tr;

    double d_tr = a_tl * c_bl;
    double d_bl = -c_tr * a_tl;
    double d_tl = c_tr * a_bl - a_tr * c_bl;
    // double d_br = -c_tr * a_tl + a_tl * c_bl - c_tr * a_bl + a_tr * c_bl;

    double norm_horiz =
        (std::pow(d_tr * tr.x() - d_tl * tl.x(), 2) +
         std::pow(d_tr * tr.y() - d_tl * tl.y(), 2) + std::pow(d_tr - d_tl, 2));

    double norm_vert =
        (std::pow(d_bl * bl.x() - d_tl * tl.x(), 2) +
         std::pow(d_bl * bl.y() - d_tl * tl.y(), 2) + std::pow(d_bl - d_tl, 2));

    return std::sqrt(norm_horiz / norm_vert);
}

QList<QPointF> ScannedImage::Data::autoDetectCutRect()
{
    auto img_cut = getRotatedImage();
    auto width = img_cut.cols;
    auto height = img_cut.rows;

    cv::Mat img_gray;
    cv::cvtColor(img_cut, img_gray, cv::COLOR_BGR2GRAY);

    cv::blur(img_gray, img_gray, {3, 3});
    cv::Mat img_edges;
    cv::Canny(img_gray, img_edges, 10, 40);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(
        img_edges, lines, 1, CV_PI / 180, 80, 30, img_gray.cols / 10);

    // partition the lines according to their angles into horizontal
    // and vertical ones
    auto mid = std::partition(lines.begin(), lines.end(), [](auto& line) {
        return std::abs(line[0] - line[2]) > std::abs(line[1] - line[3]);
    });

    // horizontal scores
    auto h_score = [height](auto& l) {
        auto len =
            std::sqrt(std::pow(l[0] - l[2], 2) + std::pow(l[1] - l[3], 2));
        auto pos = (l[1] + l[3] - height) / 2.0;
        return len * pos;
    };

    // vertical scores
    auto v_score = [width](auto& l) {
        auto len =
            std::sqrt(std::pow(l[0] - l[2], 2) + std::pow(l[1] - l[3], 2));
        auto pos = (l[0] + l[2] - width) / 2.0;
        return len * pos;
    };

    std::sort(lines.begin(), mid, [&](auto& l1, auto& l2) {
        return h_score(l1) < h_score(l2);
    });

    std::sort(mid, lines.end(), [&](auto& l1, auto& l2) {
        return v_score(l1) < v_score(l2);
    });

    QLineF top_line, bottom_line, left_line, right_line;

    if (mid != lines.begin()) {
        top_line.setLine(lines[0][0], lines[0][1], lines[0][2], lines[0][3]);
        bottom_line.setLine(mid[-1][0], mid[-1][1], mid[-1][2], mid[-1][3]);
    } else {
        top_line.setLine(0, 0, width, 0);
        bottom_line.setLine(0, height, width, height);
    }

    if (mid != lines.end()) {
        left_line.setLine((*mid)[0], (*mid)[1], (*mid)[2], (*mid)[3]);
        right_line.setLine(lines.end()[-1][0],
                           lines.end()[-1][1],
                           lines.end()[-1][2],
                           lines.end()[-1][3]);
    } else {
        left_line.setLine(0, 0, 0, height);
        right_line.setLine(width, 0, width, height);
    }

    QPointF topleft, topright, bottomright, bottomleft;
    if (!top_line.intersect(left_line, &topleft)) {
        topleft = {0, 0};
    }
    if (!top_line.intersect(right_line, &topright)) {
        topright = {(qreal)width, 0};
    }
    if (!bottom_line.intersect(left_line, &bottomleft)) {
        bottomleft = {0, (qreal)height};
    }
    if (!bottom_line.intersect(right_line, &bottomright)) {
        bottomright = {(qreal)width, (qreal)height};
    }

    auto project = [width, height](auto& p) {
        p.setX(std::max(0.0, std::min(1.0, p.x() / width)));
        p.setY(std::max(0.0, std::min(1.0, p.y() / height)));
    };

    project(topleft);
    project(topright);
    project(bottomleft);
    project(bottomright);

    // img->rotated = img_cut.clone();
    // for (auto line : lines) {
    //     std::cout << line << std::endl;
    //     cv::line(img->rotated,
    //              {line[0], line[1]},
    //              {line[2], line[3]},
    //              CV_RGB(255, 0, 0),
    //              5,
    //              cv::LINE_8);
    // }

    // cv::line(img->rotated,
    //          {top_line[0], top_line[1]},
    //          {top_line[2], top_line[3]},
    //          CV_RGB(0, 255, 0),
    //          10,
    //          cv::LINE_8);

    // cv::line(img->rotated,
    //          {bottom_line[0], bottom_line[1]},
    //          {bottom_line[2], bottom_line[3]},
    //          CV_RGB(0, 255, 0),
    //          10,
    //          cv::LINE_8);

    // cv::line(img->rotated,
    //          {left_line[0], left_line[1]},
    //          {left_line[2], left_line[3]},
    //          CV_RGB(0, 255, 0),
    //          10,
    //          cv::LINE_8);

    // cv::line(img->rotated,
    //          {right_line[0], right_line[1]},
    //          {right_line[2], right_line[3]},
    //          CV_RGB(0, 255, 0),
    //          10,
    //          cv::LINE_8);

    return {topleft, topright, bottomright, bottomleft};
}

const cv::Mat& ScannedImage::Data::getRotatedImage()
{
    if (original.data && state < Rotated) {
        auto fac =
            scaled
                ? std::max(0.01, 1e3 / std::max(original.cols, original.rows))
                : 1.0;
        cv::resize(original, rotated, cv::Size(), fac, fac, cv::INTER_AREA);
        switch ((int)rotAngle % 360) {
            case 90:
                cv::rotate(rotated, rotated, cv::ROTATE_90_CLOCKWISE);
                break;
            case 180:
                cv::rotate(rotated, rotated, cv::ROTATE_180);
                break;
            case 270:
                cv::rotate(rotated, rotated, cv::ROTATE_90_COUNTERCLOCKWISE);
                break;
            default:
                break;
        }
        state = Rotated;
    }
    return rotated;
}

const cv::Mat& ScannedImage::Data::getCutImage()
{
    if (original.data && state < Cut) {
        auto rotated = getRotatedImage();

        float w = rotated.cols;
        float h = rotated.rows;

        cv::Point2f tl(topleft.x() * w, topleft.y() * h);
        cv::Point2f tr(topright.x() * w, topright.y() * h);
        cv::Point2f br(bottomright.x() * w, bottomright.y() * h);
        cv::Point2f bl(bottomleft.x() * w, bottomleft.y() * h);

        auto ratio = getAspectRatio(QPointF{tl.x - w / 2, tl.y - h / 2} / 100,
                                    QPointF{tr.x - w / 2, tr.y - h / 2} / 100,
                                    QPointF{br.x - w / 2, br.y - h / 2} / 100,
                                    QPointF{bl.x - w / 2, bl.y - h / 2} / 100);

        qDebug() << "Approximate aspect ratio: " << ratio;

        float height = std::max(cv::norm(tr - tl), cv::norm(br - bl));
        float width = height * ratio;

        cv::Point2f src[4] = {tl, tr, br, bl};
        cv::Point2f dst[4] = {
            {0, 0}, {width - 1, 0}, {width - 1, height - 1}, {0, height - 1}};

        auto M = cv::getPerspectiveTransform(src, dst);
        cv::warpPerspective(rotated, cut, M, {(int)width, (int)height});
    }

    return cut;
}

const cv::Mat& ScannedImage::Data::getColorizedImage()
{
    if (original.data && state < Colorized) {
        auto img_cut = getCutImage();

        state = Colorized;

        // Apply contrast and brightness transform.
        auto contrast = std::pow(4.0, this->contrast * 2 - 1);
        auto brightness =
            128 - contrast * 128 + (2 * this->brightness - 1) * 128;
        cv::Mat img_bright;
        img_cut.convertTo(img_bright, -1, contrast, brightness);

        // Compute a gray-scale image.
        cv::Mat img_gray;
        cv::cvtColor(img_bright, img_gray, cv::COLOR_BGR2GRAY);

        if (colormode == Gray) {
            colorized = img_gray;
            return colorized;
        }

        // Threshold filter for background mask.
        cv::Mat bg_mask;
        {
            int d = std::max(this->details * 50, 3.0);
            if (d % 2 == 0) d += 1;

            cv::adaptiveThreshold(img_gray,
                                  bg_mask,
                                  255,
                                  cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                                  cv::THRESH_BINARY,
                                  d,
                                  5);
        }

        if (colormode == BlackAndWhite) {
            colorized = bg_mask;
            return colorized;
        }

        /// Set background to white
        cv::Mat img_col;
        img_bright.convertTo(img_col, CV_32F);
        img_col.setTo(cv::Scalar(255, 255, 255), bg_mask);

        std::vector<cv::Point3f> points;
        {
            auto itimg = img_col.begin<cv::Point3f>();
            auto itimgend = img_col.end<cv::Point3f>();
            auto itmask = bg_mask.begin<uchar>();
            for (; itimg != itimgend; ++itimg, ++itmask) {
                if (*itmask == 0) {
                    points.push_back(*itimg);
                }
            }
        }

        if (!points.empty()) {
            std::vector<int> labels;
            cv::Mat centers;
            cv::kmeans(
                points,
                8,
                labels,
                cv::TermCriteria(
                    cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0),
                3,
                cv::KMEANS_PP_CENTERS,
                centers);

            // stretch colors
            auto min =
                *std::min_element(centers.begin<float>(), centers.end<float>());
            auto max =
                *std::max_element(centers.begin<float>(), centers.end<float>());

            centers = 255 * (centers - min) / (max - min);
            cv::Mat ucenters;
            centers.convertTo(ucenters, CV_8U);

            auto itimg = img_col.begin<cv::Vec3b>();
            auto itimgend = img_col.end<cv::Vec3b>();
            auto itmask = bg_mask.begin<uchar>();
            auto itpnts = labels.begin();
            for (; itimg != itimgend; ++itimg, ++itmask) {
                if (*itmask == 0) {
                    *itimg = ucenters.row(*itpnts);
                    ++itpnts;
                }
            }
        }

        img_col.convertTo(colorized, CV_8U);
        return colorized;
    } else {
        return colorized;
    }
}

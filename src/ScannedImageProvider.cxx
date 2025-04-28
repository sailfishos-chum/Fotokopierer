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

#include "ScannedImageProvider.hxx"

#include "Convert.hxx"
#include "Util.hxx"

#include <QDebug>
#include <QtCore/QMap>
#include <QtGui/QImageReader>
#include <QtGui/QPixmap>

#include <opencv2/imgproc/imgproc.hpp>

#include <cmath>

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

struct ImageSet {
    ImageState state = Original;

    /// The original image.
    cv::Mat original;

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

    ScannedImageProvider::ColorMode colormode;

    /// The colorised image.
    cv::Mat colorized;
};
}

static const cv::Mat& getRotatedImage(ImageSet& img);
static const cv::Mat& getCutImage(ImageSet& img);
static const cv::Mat& getColorizedImage(ImageSet& img);

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
static double getAspectRatio(const QPointF& tl,
                             const QPointF& tr,
                             const QPointF& br,
                             const QPointF& bl);

ScannedImageProvider* ScannedImageProvider::instance = nullptr;

struct ScannedImageProvider::Data {
    QMap<QString, ImageSet> images;
    int64_t next_id = 0;
};

ScannedImageProvider::ScannedImageProvider()
    : QQuickImageProvider(ImageType::Image), d(new Data)
{
}

ScannedImageProvider::~ScannedImageProvider() {}

QImage ScannedImageProvider::requestImage(const QString& id,
                                          QSize* size,
                                          const QSize& requestedSize)
{
    auto toks = id.split(QLatin1Char('/'));

    if (toks.size() < 1) {
        // TODO: return ERROR picture
        return {};
    }

    auto img = d->images.find(toks[0]);
    if (img == d->images.end()) {
        // TODO: return ERROR picture
        return {};
    }

    if (toks[1] == QLatin1String("original")) {
        return cvMatToQImage(img->original);
    } else if (toks[1] == QLatin1String("scaled")) {
        auto angle = 0.0;
        if (toks.size() == 3) {
            angle = toks[2].toFloat();
        }
        setAngle(toks[0], angle);
        return cvMatToQImage(getRotatedImage(*img));
    } else if (toks[1] == QLatin1String("cut")) {
        auto colormode = Colored;

        if (toks.size() >= 3) {
            if (toks[2] == QLatin1String("colored")) {
                colormode = Colored;
            } else if (toks[2] == QLatin1String("gray")) {
                colormode = Gray;
            } else if (toks[2] == QLatin1String("bw")) {
                colormode = BlackAndWhite;
            } else {
                qWarning() << "Unknown color mode: " << toks[2];
            }
        }

        auto contrast = toks.size() >= 4 ? toks[3].toFloat() / 100 : 0.5;
        auto brightness = toks.size() >= 5 ? toks[4].toFloat() / 100 : 0.5;
        auto details = toks.size() >= 6 ? toks[5].toFloat() / 100 : 0.5;

        setColorMode(toks[0], colormode);
        setContrast(toks[0], contrast);
        setBrightness(toks[0], brightness);
        setDetails(toks[0], details);

        return cvMatToQImage(getColorizedImage(*img));
    } else {
        // TODO: return ERROR picture
        return {};
    }
}

QString ScannedImageProvider::loadImage(const QString& fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    auto pic = QImageToCvMat(reader.read());

    if (pic.data == nullptr) {
        return {};
    }

    auto id = QString::number(d->next_id);

    ImageSet img;
    img.state = Original;
    img.original = std::move(pic);
    d->images[id] = std::move(img);

    return id;
}

void ScannedImageProvider::setAngle(const QString& image, double angle)
{
    auto img = d->images.find(image);
    if (img != d->images.end()) {
        auto rotAngle = (int)angle % 360;
        if (rotAngle < 0) {
            rotAngle += 360;
        }
        if (rotAngle != img->rotAngle) {
            img->rotAngle = rotAngle;
            img->state = (ImageState)std::min((int)img->state, Rotated - 1);
        }
    } else {
        qWarning() << "(setAngle) Unknown image id: " << image;
    }
}

bool ScannedImageProvider::setCutBox(const QString& image,
                                     const QPointF& topleft,
                                     const QPointF& topright,
                                     const QPointF& bottomright,
                                     const QPointF& bottomleft)
{
    static Util util;

    auto img = d->images.find(image);
    if (img != d->images.end()) {
        if (!util.isConvex(topleft, topright, bottomright, bottomleft)) {
            return false;
        }
        img->topleft = topleft;
        img->topright = topright;
        img->bottomright = bottomright;
        img->bottomleft = bottomleft;
        img->state = (ImageState)std::min((int)img->state, Cut - 1);
        return true;
    } else {
        qWarning() << "(setCutBox) Unknown image id: " << image;
        return false;
    }
}

void ScannedImageProvider::setColorMode(const QString& image,
                                        ColorMode colormode)
{
    auto img = d->images.find(image);
    if (img != d->images.end()) {
        if (colormode != img->colormode) {
            img->colormode = colormode;
            img->state = (ImageState)std::min((int)img->state, Colorized - 1);
        }
    } else {
        qWarning() << "(setColorMode) Unknown image id: " << image;
    }
}

void ScannedImageProvider::setContrast(const QString& image, double contrast)
{
    auto img = d->images.find(image);
    if (img != d->images.end()) {
        contrast = std::max(0.0, std::min(1.0, contrast));
        if (contrast != img->contrast) {
            img->contrast = contrast;
            img->state = (ImageState)std::min((int)img->state, Colorized - 1);
        }
    } else {
        qWarning() << "(set_contrast) Unknown image id: " << image;
    }
}

void ScannedImageProvider::setBrightness(const QString& image,
                                         double brightness)
{
    auto img = d->images.find(image);
    if (img != d->images.end()) {
        brightness = std::max(0.0, std::min(1.0, brightness));
        if (brightness != img->brightness) {
            img->brightness = brightness;
            img->state = (ImageState)std::min((int)img->state, Colorized - 1);
        }
    } else {
        qWarning() << "(set_brightness) Unknown image id: " << image;
    }
}

void ScannedImageProvider::setDetails(const QString& image, double details)
{
    auto img = d->images.find(image);
    if (img != d->images.end()) {
        details = std::max(0.0, std::min(1.0, details));
        if (details != img->details) {
            img->details = details;
            img->state = (ImageState)std::min((int)img->state, Colorized - 1);
        }
    } else {
        qWarning() << "(set_details) Unknown image id: " << image;
    }
}

double getAspectRatio(const QPointF& tl,
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

const cv::Mat& getRotatedImage(ImageSet& img)
{
    if (img.state < Rotated) {
        auto factor = std::max(
            0.01, 1000.0 / std::max(img.original.cols, img.original.rows));
        cv::resize(img.original,
                   img.rotated,
                   cv::Size(),
                   factor,
                   factor,
                   cv::INTER_AREA);
        switch ((int)img.rotAngle % 360) {
            case 90:
                cv::rotate(img.rotated, img.rotated, cv::ROTATE_90_CLOCKWISE);
                break;
            case 180:
                cv::rotate(img.rotated, img.rotated, cv::ROTATE_180);
                break;
            case 270:
                cv::rotate(
                    img.rotated, img.rotated, cv::ROTATE_90_COUNTERCLOCKWISE);
                break;
            default:
                break;
        }
        img.state = Rotated;
    }
    return img.rotated;
}

const cv::Mat& getCutImage(ImageSet& img)
{
    if (img.state < Cut) {
        auto rotated = getRotatedImage(img);

        float w = rotated.cols;
        float h = rotated.rows;

        cv::Point2f tl(img.topleft.x() * w, img.topleft.y() * h);
        cv::Point2f tr(img.topright.x() * w, img.topright.y() * h);
        cv::Point2f br(img.bottomright.x() * w, img.bottomright.y() * h);
        cv::Point2f bl(img.bottomleft.x() * w, img.bottomleft.y() * h);

        // float width = cv::max(cv::norm(tr - tl), cv::norm(br - bl));
        // float height = cv::max(cv::norm(tr - br), cv::norm(tl - bl));
        auto ratio = getAspectRatio(QPointF{tl.x - w / 2, tl.y - h / 2} / 100,
                                    QPointF{tr.x - w / 2, tr.y - h / 2} / 100,
                                    QPointF{br.x - w / 2, br.y - h / 2} / 100,
                                    QPointF{bl.x - w / 2, bl.y - h / 2} / 100);

        qDebug() << "Approximate aspect ratio: " << ratio;

        float height = cv::max(cv::norm(tr - br), cv::norm(tl - bl));
        float width = height * ratio;

        cv::Point2f src[4] = {tl, tr, br, bl};
        cv::Point2f dst[4] = {
            {0, 0}, {width - 1, 0}, {width - 1, height - 1}, {0, height - 1}};

        auto M = cv::getPerspectiveTransform(src, dst);
        cv::warpPerspective(rotated, img.cut, M, {(int)width, (int)height});
    }

    return img.cut;
}

const cv::Mat& getColorizedImage(ImageSet& img)
{
    if (img.state < Colorized) {
        auto img_cut = getCutImage(img);

        img.state = Colorized;

        // Apply contrast and brightness transform.
        auto contrast = std::pow(4.0, img.contrast * 2 - 1);
        auto brightness = 128 - contrast * 128 + (2 * img.brightness - 1) * 128;
        cv::Mat img_bright;
        img_cut.convertTo(img_bright, -1, contrast, brightness);

        // Compute a gray-scale image.
        cv::Mat img_gray;
        cv::cvtColor(img_bright, img_gray, cv::COLOR_BGR2GRAY);

        if (img.colormode == ScannedImageProvider::Gray) {
            img.colorized = img_gray;
            return img.colorized;
        }

        // Threshold filter for background mask.
        cv::Mat bg_mask;
        {
            int d = std::max(img.details * 50, 3.0);
            if (d % 2 == 0) d += 1;

            cv::adaptiveThreshold(img_gray,
                                  bg_mask,
                                  255,
                                  cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                                  cv::THRESH_BINARY,
                                  d,
                                  5);
        }

        if (img.colormode == ScannedImageProvider::BlackAndWhite) {
            img.colorized = bg_mask;
            return img.colorized;
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

        img_col.convertTo(img.colorized, CV_8U);
        return img.colorized;
    } else {
        return img.colorized;
    }
}

QList<QPointF> ScannedImageProvider::autoDetectCutRect(const QString& image)
{
    auto img = d->images.find(image);
    if (img == d->images.end()) {
        qWarning() << "(autoDetectCutRect) Unknown image id: " << image;
        return {};
    }

    auto img_cut = getRotatedImage(*img);
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

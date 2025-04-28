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
#include <QtGui/QPixmap>

#include <opencv2/imgcodecs/imgcodecs.hpp>
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
        auto angle = 0.0;
        if (toks.size() == 4 && toks[2] == QLatin1String("rotate")) {
            angle = toks[3].toFloat();
        }
        set_angle(toks[0], angle);
        return cvMatToQImage(img->original);
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

        auto contrast = toks.size() >= 4 ? toks[3].toFloat() : 0.5;
        auto brightness = toks.size() >= 5 ? toks[4].toFloat() : 0.5;
        auto details = toks.size() >= 6 ? toks[5].toFloat() : 0.5;

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
    auto pic = cv::imread(fileName.toStdString());

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

void ScannedImageProvider::set_angle(const QString& image, double angle)
{
    auto img = d->images.find(image);
    if (img != d->images.end()) {
        auto rotAngle = (int)angle % 360;
        if (rotAngle != img->rotAngle) {
            img->rotAngle = rotAngle;
            img->state = (ImageState)std::min((int)img->state, Rotated - 1);
        }
    } else {
        qWarning() << "(set_angle) Unknown image id: " << image;
    }
}

bool ScannedImageProvider::set_cut_box(const QString& image,
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
        qWarning() << "(set_cut_image) Unknown image id: " << image;
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
        qDebug() << "FACTOR " << factor << " " << img.rotated.cols << " "
                 << img.rotated.rows;
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

        float width = cv::max(cv::norm(tr - tl), cv::norm(br - bl));
        float height = cv::max(cv::norm(tr - br), cv::norm(tl - bl));

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

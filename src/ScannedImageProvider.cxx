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
    Rotated = 1,
    /// The image has been cut and perspectively transformed.
    Cut = 2,
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

    /// The colourised image.
    cv::Mat colorized;
};
}

static const cv::Mat& getRotatedImage(ImageSet& img);
static const cv::Mat& getCutImage(ImageSet& img);

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
        set_angle(toks[0], toks[3].toFloat());
        return cvMatToQImage(img->original);
    } else if (toks[1] == QLatin1String("cut")) {
        return cvMatToQImage(getCutImage(*img));
    } else if (toks[1] == QLatin1String("colorized")) {
        return cvMatToQImage(img->colorized);
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
        return true;
    } else {
        qWarning() << "(set_cut_image) Unknown image id: " << image;
        return false;
    }
}

const cv::Mat& getRotatedImage(ImageSet& img)
{
    if (img.state < Rotated) {
        switch ((int)img.rotAngle % 360) {
            case 90:
                cv::rotate(img.original, img.rotated, cv::ROTATE_90_CLOCKWISE);
                break;
            case 180:
                cv::rotate(img.original, img.rotated, cv::ROTATE_180);
                break;
            case 270:
                cv::rotate(
                    img.original, img.rotated, cv::ROTATE_90_COUNTERCLOCKWISE);
                break;
            default:
                img.rotated = img.original;
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
        if (rotated.data == nullptr) {
            return img.cut;
        }

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

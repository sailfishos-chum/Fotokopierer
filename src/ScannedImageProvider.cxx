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

#include <QtCore/QMap>
#include <QtGui/QPixmap>

#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace
{
struct ImageSet {
    cv::Mat original;
    cv::Mat cut;
    cv::Mat colorized;
};
}

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
        auto orig = cvMatToQImage(img->original);
        if (toks.size() == 4 && toks[2] == QLatin1String("rotate")) {
            auto angle = toks[3].toFloat();
            QTransform transform;
            transform.rotate(angle);
            return orig.transformed(transform);
        } else {
            return orig;
        }
    } else if (toks[1] == QLatin1String("cut")) {
        return cvMatToQImage(img->cut);
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
    d->images[id] = {.original = pic, .cut = {}, .colorized = {}};

    return id;
}

void ScannedImageProvider::set_cut_image(const QString& image,
                                         double angle,
                                         const QPointF& topleft,
                                         const QPointF& topright,
                                         const QPointF& bottomright,
                                         const QPointF& bottomleft)
{
    auto img = d->images.find(image);
    if (img != d->images.end()) {
        float w = img->original.cols;
        float h = img->original.rows;

        cv::Point2f tl(topleft.x() * w, topleft.y() * h);
        cv::Point2f tr(topright.x() * w, topright.y() * h);
        cv::Point2f br(bottomright.x() * w, bottomright.y() * h);
        cv::Point2f bl(bottomleft.x() * w, bottomleft.y() * h);

        float width = cv::max(cv::norm(tr - tl), cv::norm(br - bl));
        float height = cv::max(cv::norm(tr - br), cv::norm(tl - bl));

        cv::Point2f src[4] = {tl, tr, br, bl};
        cv::Point2f dst[4] = {
            {0, 0}, {width - 1, 0}, {width - 1, height - 1}, {0, height - 1}};

        auto M = cv::getPerspectiveTransform(src, dst);
        cv::warpPerspective(
            img->original, img->cut, M, {(int)width, (int)height});
    }
}

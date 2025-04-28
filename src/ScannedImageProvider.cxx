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

#include "ScannedImage.hxx"

#include <QDebug>
#include <QtCore/QMap>

#include <cassert>

ScannedImageProvider* ScannedImageProvider::instance = nullptr;

struct ScannedImageProvider::Data {
    QMap<QString, ScannedImage*> images;
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
        return (*img)->getOriginal();
    } else if (toks[1] == QLatin1String("scaled")) {
        auto angle = 0.0;
        if (toks.size() == 3) {
            angle = toks[2].toFloat();
        }
        (*img)->setAngle(angle);
        return (*img)->getRotated();
    } else if (toks[1] == QLatin1String("cut")) {
        auto colormode = ScannedImage::Colored;

        if (toks.size() >= 3) {
            if (toks[2] == QLatin1String("colored")) {
                colormode = ScannedImage::Colored;
            } else if (toks[2] == QLatin1String("gray")) {
                colormode = ScannedImage::Gray;
            } else if (toks[2] == QLatin1String("bw")) {
                colormode = ScannedImage::BlackAndWhite;
            } else {
                qWarning() << "Unknown color mode: " << toks[2];
            }
        }

        auto contrast = toks.size() >= 4 ? toks[3].toFloat() / 100 : 0.5;
        auto brightness = toks.size() >= 5 ? toks[4].toFloat() / 100 : 0.5;
        auto details = toks.size() >= 6 ? toks[5].toFloat() / 100 : 0.5;

        (*img)->setColorMode(colormode);
        (*img)->setContrast(contrast);
        (*img)->setBrightness(brightness);
        (*img)->setDetails(details);

        auto i = (*img)->getColorized();
        qDebug() << "getColorized " << id << " " << i.width() << " "
                 << i.height();
        return i;
    } else {
        // TODO: return ERROR picture
        return {};
    }
}

QString ScannedImageProvider::registerImage(ScannedImage* image)
{
    assert(image != nullptr);

    auto id = QString::number(d->next_id++);
    d->images[id] = image;

    return id;
}

void ScannedImageProvider::unregisterImage(ScannedImage* image,
                                           const QString& id)
{
    auto it = d->images.find(id);
    assert(it != d->images.end());
    assert(*it == image);
    d->images.erase(it);
}

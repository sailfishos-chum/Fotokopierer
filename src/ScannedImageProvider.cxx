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

#include <QtCore/QMap>
#include <QtGui/QPixmap>

namespace
{
struct ImageSet {
    QPixmap original;
    QPixmap cut;
    QPixmap colorized;
};
}

ScannedImageProvider* ScannedImageProvider::instance = nullptr;

struct ScannedImageProvider::Data {
    QMap<QString, ImageSet> images;
    int64_t next_id = 0;
};

ScannedImageProvider::ScannedImageProvider() : QQuickImageProvider(ImageType::Pixmap), d(new Data)
{
}

ScannedImageProvider::~ScannedImageProvider() {}

QPixmap ScannedImageProvider::requestPixmap(const QString& id, QSize* size,
                                            const QSize& requestedSize)
{
    auto toks = id.split(QLatin1Char('/'));

    if (toks.size() < 1 || toks.size() > 2) {
        // TODO: return ERROR picture
        return {};
    }

    auto img = d->images.find(toks[0]);
    if (img == d->images.end()) {
        // TODO: return ERROR picture
        return {};
    }

    if (toks.size() == 1 || toks[1] == QLatin1String("original")) {
        return img->original;
    } else if (toks[2] == QLatin1String("cut")) {
        return img->cut;
    } else if (toks[2] == QLatin1String("colorized")) {
        return img->colorized;
    } else {
        // TODO: return ERROR picture
        return {};
    }
}

QString ScannedImageProvider::loadImage(const QString& fileName)
{
    auto pic = QPixmap(fileName);

    if (pic.isNull()) {
        return {};
    }

    auto id = QString::number(d->next_id);
    d->images[id] = {.original = pic, .cut = {}, .colorized = {}};

    return id;
}

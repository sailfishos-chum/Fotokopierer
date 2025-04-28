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

#include "PlainImage.hxx"

#include <QtGui/QImage>

#include <QDebug>

struct PlainImage::Data {
    QImage image;
    bool scale = true;
};

PlainImage::PlainImage() : d(new Data) {}

PlainImage::~PlainImage() = default;

bool PlainImage::scale() const
{
    return d->scale;
}

QImage PlainImage::sourceImage() const
{
    return d->image.isNull() ? BaseImage::sourceImage() : d->image;
}

void PlainImage::setScale(bool enabled)
{
    if (enabled != d->scale) {
        d->scale = enabled;
        emit scaleChanged();
    }
}

void PlainImage::loadFile(const QString& file_name)
{
    qDebug() << "Load plain " << file_name;
    QImage image(file_name);
    if (image.isNull()) {
        qDebug() << "Failed";
        emit loadFailed();
    } else {
        qDebug() << "Success";
        d->image = std::move(image);
        emit sourceChanged();
        updateImage();
    }
}

QImage PlainImage::transform(const QImage& image)
{
    if (d->scale && std::max(image.width(), image.height()) > 1000) {
        if (image.width() > image.height()) {
            return image.scaledToWidth(1000);
        } else {
            return image.scaledToHeight(1000);
        }
    } else {
        return image;
    }
}

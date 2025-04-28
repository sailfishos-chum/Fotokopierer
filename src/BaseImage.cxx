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

#include "BaseImage.hxx"

#include <QtGui/QPainter>

struct BaseImage::Data {
    qreal painted_width = 0;          ///< width of the painted image (after scaling)
    qreal painted_height = 0;         ///< height of the painted image (after scaling)
    QImage source_image;              ///< cache the original image before the transformation
    QImage image;                     ///< the transformed image
    BaseImage* base_image = nullptr;  ///< pointer to the source image producer
};

BaseImage::BaseImage() : d(new Data)
{
    connect(this, &BaseImage::imageChanged, [this]() { this->update(); });
}

BaseImage::~BaseImage() {}

void BaseImage::paint(QPainter* painter)
{
    if (d->image.width() > 0 && d->image.height() > 0) {
        auto wratio = static_cast<qreal>(width()) / d->image.width();
        auto hratio = static_cast<qreal>(height()) / d->image.height();
        auto ratio = std::min(wratio, hratio);
        d->painted_width = d->image.width() * ratio;
        d->painted_height = d->image.height() * ratio;
        emit paintedSizeChanged();
        painter->drawImage(QRectF{(width() - d->painted_width) / 2,
                                  (height() - d->painted_height) / 2,
                                  d->painted_width,
                                  d->painted_height},
                           d->image);
    }
}

qreal BaseImage::paintedWidth() const
{
    return d->painted_width;
}

qreal BaseImage::paintedHeight() const
{
    return d->painted_height;
}

void BaseImage::setSource(BaseImage* base_image)
{
    if (base_image == d->base_image) return;

    if (d->base_image) {
        disconnect(d->base_image, &BaseImage::imageChanged, this, &BaseImage::updateImage);
    }

    d->base_image = base_image;

    if (d->base_image) {
        d->source_image = d->base_image->image();
        qDebug() << "Connect " << (void*)d->base_image << " " << (void*)this;
        connect(d->base_image, &BaseImage::imageChanged, this, &BaseImage::updateImage);
        updateImage();
    } else {
        d->source_image = {};
    }

    emit sourceChanged();
}

BaseImage* BaseImage::source()
{
    return d->base_image;
}

QImage BaseImage::sourceImage() const
{
    return d->source_image;
}

QImage BaseImage::image() const
{
    return d->image;
}

void BaseImage::updateImage()
{
    emit imageChanging();
    if (d->base_image) d->source_image = d->base_image->image();
    d->image = transform(d->source_image);
    emit imageChanged();
    update();
}

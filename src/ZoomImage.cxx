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

#include "ZoomImage.hxx"

#include "BaseImage.hxx"

#include <QtGui/QImage>
#include <QtGui/QPainter>

struct ZoomImage::Data {
    QPointF viewSize = {0.1, 0.1};
    QPointF center;
    QColor cross_color = Qt::red;
    QColor border_color = Qt::white;
    BaseImage* source = nullptr;
};

ZoomImage::ZoomImage() : d(new Data) {}

ZoomImage::~ZoomImage() = default;

QPointF ZoomImage::viewSize() const
{
    return d->viewSize;
}

void ZoomImage::setViewSize(const QPointF& viewSize)
{
    // all ratio coordinates must be in [0,1]
    QPointF vs = QPointF{qBound<qreal>(0, viewSize.x(), 1), qBound<qreal>(0, viewSize.y(), 1)};
    if (vs != d->viewSize) {
        d->viewSize = vs;
        emit viewSizeChanged();
        update();
    }
}

QPointF ZoomImage::center() const
{
    return d->center;
}

void ZoomImage::setCenter(const QPointF& center)
{
    QPointF c = QPointF{qBound<qreal>(0, center.x(), 1), qBound<qreal>(0, center.y(), 1)};
    if (c != d->center) {
        d->center = c;
        emit centerChanged();
        update();
    }
}

QColor ZoomImage::borderColor() const
{
    return d->border_color;
}

void ZoomImage::setBorderColor(const QColor& color)
{
    if (color != d->border_color) {
        d->border_color = color;
        emit borderColorChanged();
        update();
    }
}

QColor ZoomImage::crossColor() const
{
    return d->cross_color;
}

void ZoomImage::setCrossColor(const QColor& color)
{
    if (color != d->cross_color) {
        d->cross_color = color;
        emit crossColorChanged();
        update();
    }
}

void ZoomImage::setSource(BaseImage* source)
{
    if (source == d->source) return;

    if (d->source) {
        disconnect(d->source, &BaseImage::imageChanged, this, &ZoomImage::updateImage);
    }

    d->source = source;

    if (d->source) {
        connect(d->source, &BaseImage::imageChanged, this, &ZoomImage::updateImage);
        update();
    }

    emit sourceChanged();
}

BaseImage* ZoomImage::source()
{
    return d->source;
}

void ZoomImage::paint(QPainter* p)
{
    if (!d->source) return;

    auto w = width();
    auto h = height();

    // will background with black
    p->fillRect(0, 0, w, h, Qt::black);

    // get the source image
    QImage image = d->source->image();
    auto iw = image.width();
    auto ih = image.height();

    // prepare the clipping path (a circle) so that the picture is only shown
    // within the preview area.
    auto linewidth = std::min(width(), height()) / 20;
    QPainterPath clip;
    clip.addEllipse(0, 0, width(), height());

    // draw the part of image with clipping, note that we use ration coordinates
    p->setClipPath(clip);
    p->drawImage(QRectF{0, 0, w, h},
                 image,
                 QRectF(iw * (d->center.x() - d->viewSize.x() / 2),
                        ih * (d->center.y() - d->viewSize.y() / 2),
                        iw * d->viewSize.x(),
                        ih * d->viewSize.y()));

    // disable clipping for drawing the boundary
    p->setClipping(false);

    // draw the boundary
    QPen pen(d->border_color);
    pen.setWidth(linewidth);
    p->setPen(pen);
    p->drawEllipse(linewidth / 2, linewidth / 2, w - linewidth, h - linewidth);

    // draw the cross
    p->setPen(d->cross_color);
    p->drawLine(w / 2, h / 2 - h / 6, w / 2, h / 2 + h / 6);
    p->drawLine(w / 2 - w / 6, h / 2, w / 2 + w / 6, h / 2);
}

void ZoomImage::updateImage()
{
    update();
}

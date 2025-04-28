/*
 * Copyright (c) 2018, 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include "Filter.hxx"

#include <QtGui/QImage>
#include <QtGui/QPainter>

struct ZoomImage::Data {
    QPointF viewSize = {0.1, 0.1};
    QPointF center;
    QColor cross_color = Qt::red;
    QColor border_color = Qt::white;

    Scanner* image = nullptr;
    Scanner::FilterType filter_type = Scanner::FilterType::None;
    Filter* filter = nullptr;
};

ZoomImage::ZoomImage(QQuickItem* parent)
    : QQuickPaintedItem(parent), d(new Data) {}

ZoomImage::~ZoomImage() = default;

QPointF ZoomImage::viewSize() const
{
    return d->viewSize;
}

void ZoomImage::setViewSize(QPointF viewSize)
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

void ZoomImage::setCenter(QPointF center)
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

void ZoomImage::setImage(Scanner* image)
{
    if (image != d->image) {
        d->image = image;
        updateFilter();
        emit imageChanged();
        update();
    }
}

Scanner* ZoomImage::image() const
{
    return d->image;
}

void ZoomImage::setFilterType(Scanner::FilterType filter_type)
{
    if (filter_type != d->filter_type) {
        d->filter_type = filter_type;
        updateFilter();
        emit filterTypeChanged();
    }
}

Scanner::FilterType ZoomImage::filterType() const
{
    return d->filter_type;
}

void ZoomImage::updateFilter()
{
    if (d->filter != nullptr) {
        disconnect(d->filter, &Filter::filterChanged, this, &ZoomImage::onFilterChanged);
    }

    if (d->image != nullptr && d->filter_type != Scanner::FilterType::None) {
        d->filter = d->image->filter(d->filter_type);
        connect(d->filter, &Filter::filterChanged, this, &ZoomImage::onFilterChanged);
    }
    update();
}

void ZoomImage::paint(QPainter* p)
{
    QImage image;

    if (d->filter != nullptr)
        image = d->filter->filteredImage();
    else if (d->image != nullptr)
        image = d->image->originalImage();

    if (image.isNull()) return;

    auto w = width();
    auto h = height();

    // get the source image
    auto iw = image.width();
    auto ih = image.height();

    // prepare the clipping path (a circle) so that the picture is only shown
    // within the preview area.
    auto linewidth = std::min(width(), height()) / 20;
    QPainterPath clip;
    clip.addEllipse(0, 0, width(), height());

    // draw the part of image with clipping, note that we use ration coordinates
    p->setClipPath(clip);

    // fill background with black
    p->fillRect(0, 0, w, h, Qt::black);

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

void ZoomImage::onFilterChanged()
{
    update();
}

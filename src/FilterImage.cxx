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

#include "FilterImage.hxx"

#include "Filter.hxx"
#include "ScanImage.hxx"

#include <QtGui/QPainter>

struct FilterImage::Data {
    std::shared_ptr<Filter> filter = nullptr;
    qreal painted_width = 0;
    qreal painted_height = 0;

    ScanImage::FilterType filter_type = ScanImage::FilterType::None;
    ScanImage* image = nullptr;
};

FilterImage::FilterImage(QQuickItem* parent) : QQuickPaintedItem(parent), d(new Data) {}

FilterImage::~FilterImage() = default;

qreal FilterImage::paintedWidth() const
{
    return d->painted_width;
}

qreal FilterImage::paintedHeight() const
{
    return d->painted_height;
}

ScanImage::FilterType FilterImage::filterType() const
{
    return d->filter_type;
}

void FilterImage::setFilterType(ScanImage::FilterType type)
{
    if (d->filter_type != type) {
        d->filter_type = type;
        emit filterTypeChanged();
    }
}

ScanImage* FilterImage::image() const
{
    return d->image;
}

void FilterImage::setImage(ScanImage* image)
{
    if (d->image != image) {
        d->image = image;
        emit imageChanged();
    }
}

void FilterImage::updateFilter()
{
    if (d->filter != nullptr) {
        disconnect(d->filter.get(), &Filter::filterChanged, this, &FilterImage::update);
    }

    if (d->image != nullptr || d->filter_type == ScanImage::FilterType::None) {
        d->filter = d->image->filter(d->filter_type);
        connect(d->filter.get(), &Filter::filterChanged, this, &FilterImage::update);
    } else {
        d->filter = nullptr;
    }
    update();
}

void FilterImage::update()
{
    QQuickPaintedItem::update();
}

void FilterImage::paint(QPainter* painter)
{
    QImage image;

    if (d->filter)
        image = d->filter->filteredImage();
    else if (d->image)
        image = d->image->originalImage();
    else
        return;

    if (image.width() > 0 && image.height() > 0) {
        auto wratio = static_cast<qreal>(width()) / image.width();
        auto hratio = static_cast<qreal>(height()) / image.height();
        auto ratio = std::min(wratio, hratio);
        d->painted_width = image.width() * ratio;
        d->painted_height = image.height() * ratio;
        emit paintedSizeChanged();
        painter->drawImage(QRectF{(width() - d->painted_width) / 2,
                                  (height() - d->painted_height) / 2,
                                  d->painted_width,
                                  d->painted_height},
                           image);
    }
}

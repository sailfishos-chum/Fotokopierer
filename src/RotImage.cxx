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

#include "RotImage.hxx"

#include <QtGui/QImage>

RotImage::RotImage(QQuickItem* parent) : BaseImage(parent), orientation_(0) {}

RotImage::~RotImage() = default;

int RotImage::orientation() const
{
    return orientation_;
}

void RotImage::setOrientation(int orientation)
{
    orientation %= 4;
    if (orientation != orientation_) {
        orientation_ = orientation;
        emit orientationChanged();
        updateImage();
    }
}

QImage RotImage::transform(const QImage& image)
{
    QTransform transform;
    transform.rotate(orientation_ * 90.0);
    return image.transformed(transform);
}

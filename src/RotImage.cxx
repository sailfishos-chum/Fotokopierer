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

RotImage::RotImage() : rotation_(0) {}

RotImage::~RotImage() = default;

int RotImage::rotation() const
{
    return rotation_;
}

void RotImage::setRotation(int rotation)
{
    rotation %= 4;
    if (rotation != rotation_) {
        rotation_ = rotation;
        emit rotationChanged();
        updateImage();
    }
}

QImage RotImage::transform(const QImage& image)
{
    QTransform transform;
    transform.rotate(rotation_ * 90.0);
    return image.transformed(transform);
}

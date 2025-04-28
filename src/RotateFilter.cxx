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

#include "RotateFilter.hxx"

#include <QtCore/QJsonObject>
#include <QtGui/QImage>

RotateFilter::RotateFilter(ScanImage* image) : RotateFilter(image, nullptr) {}

RotateFilter::RotateFilter(ScanImage* image, Filter* previous_filter)
    : Filter(image, previous_filter), orientation_(0)
{
    connect(this, &RotateFilter::orientationChanged, this, &Filter::filterChanged);
}

RotateFilter::~RotateFilter() = default;

int RotateFilter::orientation() const
{
    return orientation_;
}

void RotateFilter::setOrientation(int orientation)
{
    orientation %= 4;
    if (orientation != orientation_) {
        orientation_ = orientation;
        emit orientationChanged();
    }
}

QJsonObject RotateFilter::saveJson() const
{
    return {};
}

void RotateFilter::loadJson(QJsonObject& object) {}

QImage RotateFilter::apply(QImage&& image)
{
    QTransform transform;
    transform.rotate(orientation_ * 90.0);
    return image.transformed(transform);
}

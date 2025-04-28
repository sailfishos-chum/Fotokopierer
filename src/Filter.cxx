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

#include "Filter.hxx"

#include "ScanImage.hxx"

#include <QtGui/QImage>

Filter::Filter(ScanImage* image) : QObject(image) {}

Filter::Filter(ScanImage* image, const std::shared_ptr<Filter>& previous_filter)
    : QObject(image), previous_filter_(previous_filter)
{
}

Filter::~Filter() = default;

ScanImage* Filter::image()
{
    return qobject_cast<ScanImage*>(parent());
}

QImage Filter::filteredImage()
{
    if (previous_filter_ != nullptr) {
        return apply(previous_filter_->filteredImage());
    } else {
        return apply(image()->originalImage());
    }
}

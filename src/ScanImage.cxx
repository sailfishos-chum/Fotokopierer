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

#include "ScanImage.hxx"

#include "ColorizeFilter.hxx"
#include "CutFilter.hxx"
#include "Document.hxx"
#include "Filter.hxx"
#include "RotateFilter.hxx"

#include <QtCore/QVector>
#include <QtGui/QImage>

struct ScanImage::Data {
    QVector<std::shared_ptr<Filter>> filter;
    QImage original;
};

ScanImage::ScanImage(QObject* parent) : QObject(parent), d(new Data)
{
    d->filter.reserve(3);
    d->filter.push_back(std::shared_ptr<Filter>(new RotateFilter(this)));
    d->filter.push_back(std::shared_ptr<Filter>(new CutFilter(this, d->filter.back())));
    d->filter.push_back(std::shared_ptr<Filter>(new ColorizeFilter(this, d->filter.back())));
}

ScanImage::~ScanImage() = default;

std::shared_ptr<Filter> ScanImage::filter(FilterType type)
{
    if (type == FilterType::None)
        return nullptr;
    else
        return d->filter[static_cast<int>(type)];
}

RotateFilter* ScanImage::rotateFilter() const
{
    return qobject_cast<RotateFilter*>(d->filter[static_cast<int>(FilterType::Rotate)].get());
}

CutFilter* ScanImage::cutFilter() const
{
    return qobject_cast<CutFilter*>(d->filter[static_cast<int>(FilterType::Cut)].get());
}

ColorizeFilter* ScanImage::colorizeFilter() const
{
    return qobject_cast<ColorizeFilter*>(d->filter[static_cast<int>(FilterType::Colorize)].get());
}

bool ScanImage::loadFile(const QString& file_name)
{
    QImage image(file_name);
    if (image.isNull()) {
        return false;
    } else {
        d->original = image;
        emit originalImageChanged();
        return true;
    }
}

void ScanImage::saveAndClear(Document* doc)
{
    // Compute the result image.
    QImage image = d->original;
    auto f = filter(static_cast<FilterType>(d->filter.size()));
    f->apply(QImage(image));

    // Clear all filters.
    for (int i = 0; i < d->filter.size(); i++) d->filter[i] = nullptr;

    // Add a new page.
    doc->addPage(d->original, image);
}

QImage ScanImage::originalImage() const
{
    return d->original;
}

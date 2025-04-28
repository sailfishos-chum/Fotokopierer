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

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFile>
#include <QtCore/QFutureWatcher>
#include <QtCore/QVector>
#include <QtGui/QImage>
#include <QtGui/QImageReader>

#include <cassert>

struct ScanImage::Data {
    QVector<Filter*> filter;
    QImage original;
    QString originalPath;
    QImage scaled;
    QFutureWatcher<void> saveFuture;
    bool deleteOriginalOnClear = false;
};

ScanImage::ScanImage(QObject* parent) : QObject(parent), d(new Data)
{
    d->filter.reserve(3);
    d->filter.push_back(new RotateFilter(this));
    d->filter.push_back(new CutFilter(this, d->filter.back()));
    d->filter.push_back(new ColorizeFilter(this, d->filter.back()));

    connect(&d->saveFuture, &QFutureWatcher<void>::finished, this, &ScanImage::imageSaved);
}

ScanImage::~ScanImage()
{
    if (d->deleteOriginalOnClear && !d->originalPath.isEmpty()) {
        QFile::remove(d->originalPath);
    }
}

QImage ScanImage::original() const
{
    return d->original;
}

QImage ScanImage::computeFilteredImage() const
{
    QImage image = d->original;
    for (auto filter : d->filter) {
        image = filter->apply(std::move(image));
    }
    return image;
}

Filter* ScanImage::filter(FilterType type)
{
    if (type == FilterType::None)
        return nullptr;
    else
        return d->filter[static_cast<int>(type)];
}

RotateFilter* ScanImage::rotateFilter() const
{
    return qobject_cast<RotateFilter*>(d->filter[static_cast<int>(FilterType::Rotate)]);
}

CutFilter* ScanImage::cutFilter() const
{
    return qobject_cast<CutFilter*>(d->filter[static_cast<int>(FilterType::Cut)]);
}

ColorizeFilter* ScanImage::colorizeFilter() const
{
    return qobject_cast<ColorizeFilter*>(d->filter[static_cast<int>(FilterType::Colorize)]);
}

void ScanImage::setDeleteOriginalOnClear(bool enabled)
{
    if (enabled != d->deleteOriginalOnClear) {
        d->deleteOriginalOnClear = enabled;
        emit deleteOriginalOnClearChanged();
    }
}

bool ScanImage::deleteOriginalOnClear() const
{
    return d->deleteOriginalOnClear;
}

bool ScanImage::loadFile(const QString& file_name)
{
    QImageReader imageReader(file_name);
    imageReader.setAutoTransform(true);
    auto image = imageReader.read();
    if (image.isNull()) {
        return false;
    } else {
        d->originalPath = file_name;
        d->original = image;
        setDeleteOriginalOnClear(false);
        if (image.width() > image.height()) {
            d->scaled = image.scaledToWidth(qMin(image.width(), 1000));
        } else {
            d->scaled = image.scaledToHeight(qMin(image.height(), 1000));
        }
        emit originalImageChanged();
        return true;
    }
}

void ScanImage::saveAndClear(Document* doc)
{
    assert(doc != nullptr);

    connect(this, &ScanImage::addPage, doc, &Document::addPage);

    QImage original = d->original;

    clear();

    d->saveFuture.setFuture(QtConcurrent::run([this, original] {
        QImage image = original;
        for (auto filter : d->filter) {
            image = filter->apply(std::move(image));
        }

        // Add a new page.
        emit addPage(original, image);
    }));
}

void ScanImage::clear()
{
    if (!d->saveFuture.isRunning()) {
        if (d->deleteOriginalOnClear && !d->originalPath.isEmpty()) {
            QFile::remove(d->originalPath);
            d->originalPath.clear();
        }
        d->original = QImage();
        d->scaled = QImage();
        emit originalImageChanged();
    }
}

QImage ScanImage::originalImage() const
{
    return d->scaled;
}

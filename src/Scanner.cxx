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

#include "Scanner.hxx"

#include "ColorizeFilter.hxx"
#include "CutFilter.hxx"
#include "Filter.hxx"
#include "RotateFilter.hxx"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFile>
#include <QtCore/QFutureWatcher>
#include <QtCore/QVector>
#include <QtGui/QImage>
#include <QtGui/QImageReader>

#include <cassert>

struct Scanner::Data {
    QVector<Filter*> filter;
    QImage original;
    QString originalPath;
    QImage scaled;
    QFutureWatcher<void> saveFuture;
    bool deleteOriginalOnClear = false;
};

Scanner::Scanner(QObject* parent)
    : QObject(parent), d(new Data)
{
    d->filter.reserve(3);
    d->filter.push_back(new RotateFilter(this));
    d->filter.push_back(new CutFilter(this, d->filter.constLast()));
    d->filter.push_back(new ColorizeFilter(this, d->filter.constLast()));

    connect(&d->saveFuture, &QFutureWatcher<void>::finished, this, &Scanner::imageSaved);
}

Scanner::~Scanner()
{
    if (d->deleteOriginalOnClear && !d->originalPath.isEmpty()) {
        QFile::remove(d->originalPath);
    }
}

QImage Scanner::original() const
{
    return d->original;
}

QImage Scanner::computeFilteredImage() const
{
    QImage image = d->original;
    for (auto& filter : d->filter) {
        image = filter->apply(std::move(image));
    }
    return image;
}

Filter* Scanner::filter(FilterType type)
{
    if (type == FilterType::None)
        return nullptr;
    else
        return d->filter.at(static_cast<int>(type));
}

RotateFilter* Scanner::rotateFilter() const
{
    return qobject_cast<RotateFilter*>(d->filter.at(static_cast<int>(FilterType::Rotate)));
}

CutFilter* Scanner::cutFilter() const
{
    return qobject_cast<CutFilter*>(d->filter.at(static_cast<int>(FilterType::Cut)));
}

ColorizeFilter* Scanner::colorizeFilter() const
{
    return qobject_cast<ColorizeFilter*>(d->filter.at(static_cast<int>(FilterType::Colorize)));
}

void Scanner::setDeleteOriginalOnClear(bool enabled)
{
    if (enabled != d->deleteOriginalOnClear) {
        d->deleteOriginalOnClear = enabled;
        emit deleteOriginalOnClearChanged();
    }
}

bool Scanner::deleteOriginalOnClear() const
{
    return d->deleteOriginalOnClear;
}

bool Scanner::loadFile(const QString& file_name)
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

void Scanner::clear()
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

QImage Scanner::originalImage() const
{
    return d->scaled;
}

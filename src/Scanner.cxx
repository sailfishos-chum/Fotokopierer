/*
 * Copyright (c) 2018, 2019, 2021 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFile>
#include <QtCore/QFutureWatcher>
#include <QtCore/QVector>
#include <QtGui/QImage>
#include <QtGui/QImageReader>
#include <cassert>

#include "Document.hxx"
#include "Page.hxx"
#include "ScanImage.hxx"

struct Scanner::Data {
    QString originalPath;
    std::shared_ptr<ScanImage> currentImage;
    bool deleteOriginalOnClear = false;
};

Scanner::Scanner(QObject* parent)
    : QObject(parent), d(new Data)
{
}

Scanner::~Scanner()
{
    if (d->deleteOriginalOnClear && !d->originalPath.isEmpty()) {
        QFile::remove(d->originalPath);
    }
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

std::shared_ptr<ScanImage> Scanner::currentImage() const
{
    return d->currentImage;
}

void Scanner::addPage(Document* doc)
{
    if (doc == nullptr) {
        return;
    }

    auto page = doc->newPage();
    if (page == nullptr) {
        return;
    }

    page->newFromImage(doc->directory(), currentImage());
}

void Scanner::updatePage(Page* page)
{
    if (page == nullptr) {
        return;
    }

    page->updateFromImage(currentImage());
}

bool Scanner::loadPage(Page* page)
{
    if (page == nullptr) {
        return false;
    }

    if (!loadFile(page->original(), page->settings())) {
        return false;
    }

    return true;
}

bool Scanner::loadFile(const QString& file_name)
{
    return loadFile(file_name, {});
}

bool Scanner::loadFile(const QString& file_name, const QJsonObject& settings)
{
    QImageReader imageReader(file_name);
    imageReader.setAutoTransform(true);
    auto image = imageReader.read();
    if (image.isNull()) {
        return false;
    } else {
        d->originalPath = file_name;
        d->currentImage.reset(new ScanImage(image));
        setDeleteOriginalOnClear(false);

        // if (image.width() > image.height()) {
        //     d->scaled = image.scaledToWidth(qMin(image.width(), 1000));
        // } else {
        //     d->scaled = image.scaledToHeight(qMin(image.height(), 1000));
        // }

        if (!settings.isEmpty()) {
            d->currentImage->loadJson(settings);
        }

        emit currentImageChanged();
        return true;
    }
}

void Scanner::clear()
{
    if (d->currentImage != nullptr) {
        if (d->deleteOriginalOnClear && !d->originalPath.isEmpty()) {
            QFile::remove(d->originalPath);
            d->originalPath.clear();
        }
        d->currentImage = nullptr;
        emit currentImageChanged();
    }
}

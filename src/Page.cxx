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

#include "Page.hxx"

#include "Fotokopierer.hxx"
#include "ScanImage.hxx"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QException>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QFutureWatcher>
#include <QtCore/QJsonObject>
#include <QtGui/QImage>

namespace
{
/// Error when creating a new page.
class GeneratingError : public QException
{
public:
    GeneratingError(const QString& message)
        : message_(message) {}
    GeneratingError(const GeneratingError&) = default;

    void raise() const { throw *this; }
    GeneratingError* clone() const { return new GeneratingError(*this); }

    QString message() const { return message_; }

private:
    QString message_;
};
}  // namespace

struct Page::Data {
    QDateTime creation_time;
    QString original_path;
    QString result_path;
    QString thumbnail_path;

    QFutureWatcher<QString> result_thumbnail;
    QFutureWatcher<bool> generating;

    Status status = Ready;
};

Page::Page(QObject* parent)
    : QObject(parent), d(new Data)
{
    connect(
        &d->result_thumbnail, &QFutureWatcher<QString>::finished, this, &Page::thumbnailFinished);

    connect(&d->generating, &QFutureWatcher<bool>::finished, this, &Page::generationFinished);
}

Page::Page(const QDateTime& creation_time,
           const QString& original_path,
           const QString& result_path,
           const QString& thumbnail_path,
           QObject* parent)
    : Page(parent)
{
    d->creation_time = creation_time;
    d->original_path = original_path;
    d->result_path = result_path;
    d->thumbnail_path = thumbnail_path;
}

Page::Page(const QDir& dir, const ScanImage* scanImage, QObject* parent)
    : Page(parent)
{
    setStatus(Generating);

    auto ctime = QDateTime::currentDateTime();

    auto original_path =
        dir.filePath(ctime.toString(FilenameFormat) + QStringLiteral("-original.jpg"));
    auto result_path = dir.filePath(ctime.toString(FilenameFormat) + QStringLiteral("-result.png"));

    d->creation_time = ctime;
    d->original_path = original_path;
    d->result_path = result_path;

    d->generating.setFuture(QtConcurrent::run([scanImage, original_path, result_path]() {
        QImage original = scanImage->original();
        QImage result = scanImage->computeFilteredImage();

        if (!original.save(original_path)) {
            qWarning() << "Page could not be created: error saving original image";
            throw GeneratingError(
                QStringLiteral("Page could not be created: error saving original image"));
        };

        if (!result.save(result_path)) {
            qWarning() << "Page could not be created: error saving result image";
            throw GeneratingError(
                QStringLiteral("Page could not be created: error saving result image"));
        };

        return true;
    }));
}

Page::~Page() = default;

Page::Status Page::status() const
{
    return d->status;
}

void Page::setStatus(Status status)
{
    if (d->status != status) {
        d->status = status;
        emit statusChanged();
    }
}

QDateTime Page::creationTime() const
{
    return d->creation_time;
}

QString Page::thumbnail()
{
    // Check if thumbnail image exists.
    if (!d->thumbnail_path.isNull()) {
        QFileInfo finfo(d->thumbnail_path);
        if (finfo.exists()) {
            // thumbnail file exists, return the path
            return d->thumbnail_path;
        }
    }

    if (d->status == Ready) {
        setStatus(Thumbnail);
        // thumbnail does not exist, try to create it from the result image
        if (!d->thumbnail_path.isEmpty()) d->thumbnail_path.clear();

        d->result_thumbnail.setFuture(
            QtConcurrent::run(this, &Page::updateThumbnail, d->result_path));
    }

    return {};
}

void Page::thumbnailFinished()
{
    auto path = d->result_thumbnail.result();
    if (path != d->thumbnail_path) {
        d->thumbnail_path = path;
        emit thumbnailChanged();
    }
    setStatus(Ready);
}

QString Page::getOriginalImagePath() const
{
    return d->original_path;
}

QString Page::result() const
{
    return d->result_path;
}

void Page::remove()
{
    QFile(d->original_path).remove();
    QFile(d->result_path).remove();
    QFile(d->thumbnail_path).remove();
}

void Page::generationFinished()
{
    try {
        (void)d->generating.result();
        setStatus(Ready);
        // start generation of thumbnail
        (void)thumbnail();
    } catch (GeneratingError& e) {
        setStatus(Invalid);
    }
}

QString Page::updateThumbnail(const QString& filename) const
{
    // compute the resulting filename by attaching "-thumb" to the file name
    QFileInfo f(filename);
    QString scaled_filename = f.path() + QStringLiteral("/") + f.baseName() +
                              QStringLiteral("-thumb.") + f.completeSuffix();

    // load original file
    QImage img(filename);

    if (img.isNull()) return {};

    // scale down
    QImage scaled;
    if (img.width() > img.height()) {
        scaled = img.scaledToWidth(Page::ThumbnailSize);
    } else {
        scaled = img.scaledToHeight(Page::ThumbnailSize);
    }

    // write thumbnail to file
    scaled.save(scaled_filename);

    return scaled_filename;
}

bool Page::write(QJsonObject& json) const
{
    json[QStringLiteral("creationTime")] = d->creation_time.toString(FilenameFormat);
    json[QStringLiteral("originalPath")] = d->original_path;
    json[QStringLiteral("resultPath")] = d->result_path;
    if (!d->thumbnail_path.isEmpty()) {
        json[QStringLiteral("thumbnailPath")] = d->thumbnail_path;
    }

    return true;
}

bool Page::read(const QJsonObject& json)
{
    auto page_creation_time = json[QStringLiteral("creationTime")];
    if (!page_creation_time.isString()) return false;
    auto page_ctime = QDateTime::fromString(page_creation_time.toString(), FilenameFormat);
    if (page_ctime.isNull()) return false;

    auto page_original_path = json[QStringLiteral("originalPath")];
    if (!page_original_path.isString()) return false;

    auto page_result_path = json[QStringLiteral("resultPath")];
    if (!page_result_path.isString()) return false;

    auto page_thumbnail_path = json[QStringLiteral("thumbnailPath")];
    if (!page_thumbnail_path.isString() && !page_thumbnail_path.isUndefined()) return false;

    d->creation_time = page_ctime;
    d->original_path = page_original_path.toString();
    d->result_path = page_result_path.toString();
    d->thumbnail_path = page_thumbnail_path.toString();

    return true;
}

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

#include "ColorizeFilter.hxx"
#include "Fotokopierer.hxx"
#include "Scanner.hxx"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QException>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QFutureWatcher>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>
#include <QtGui/QImage>

namespace
{
/// Error when creating a new page.
class GeneratingError : public QException
{
public:
    explicit GeneratingError(const QString& message)
        : message_(message) {}

    GeneratingError(const GeneratingError&) = default;
    GeneratingError(GeneratingError&&) = default;
    GeneratingError& operator=(const GeneratingError&) = default;
    GeneratingError& operator=(GeneratingError&&) = default;

    ~GeneratingError() override = default;

    void raise() const override { throw *this; }
    GeneratingError* clone() const override { return new GeneratingError(*this); }

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
    QJsonObject settings;  ///< The filter settings for this page.

    QFutureWatcher<QString> result_thumbnail;
    QFutureWatcher<bool> generating;

    Status status = Ready;
};

Page::Page(QObject* parent)
    : QObject(parent), d(new Data)
{
    // TODO: For some reason, that I do not understand, the 'finished' signal is not emitted.
    // We send our own signals to replace them when the concurrent work task is
    // finished. This seems to work.

    //connect(&d->result_thumbnail, &QFutureWatcher<QString>::finished, this, &Page::onThumbnailFinished);
    //connect(&d->generating, &QFutureWatcher<bool>::finished, this, &Page::onGenerationFinished);
    connect(this, &Page::thumbnailFinished, this, &Page::onThumbnailFinished, Qt::QueuedConnection);
    connect(this, &Page::generationFinished, this, &Page::onGenerationFinished, Qt::QueuedConnection);
}

Page::~Page() = default;

void Page::loadFromScanner(const QDir& dir, const Scanner* scanner)
{
    setStatus(Generating);

    auto ctime = QDateTime::currentDateTime();

    auto original_path = dir.filePath(ctime.toString(FilenameFormat) + QStringLiteral("-original.jpg"));

    auto ext = QStringLiteral("png");
    switch (scanner->colorizeFilter()->colorMode()) {
        case ColorizeFilter::FullColor:
        case ColorizeFilter::Gray: ext = QStringLiteral("jpg"); break;
        default: break;
    }
    auto result_path = dir.filePath(QStringLiteral("%1-result.%2")
                                        .arg(ctime.toString(FilenameFormat))
                                        .arg(ext));

    updateFromScanner(scanner, original_path, result_path, ctime);
}

void Page::updateFromScanner(const Scanner* scanner)
{
    auto original_path = d->original_path;
    auto result_path = d->result_path;

    // the thumbnail is outdated now
    if (!d->thumbnail_path.isEmpty()) {
        QFile(d->thumbnail_path).remove();
    }

    // Remove existing data (because it is regenerated).
    setOriginal({});
    setResult({});
    setThumbnail({});

    updateFromScanner(scanner, original_path, result_path, d->creation_time);
}

void Page::updateFromScanner(const Scanner* scanner,
                             const QString& original_path,
                             const QString& result_path,
                             const QDateTime& creation_time)
{
    setStatus(Generating);

    setCreationTime(creation_time);

    d->settings = scanner->saveJson();

    d->generating.setFuture(QtConcurrent::run([this, scanner, original_path, result_path]() {
        QImage original = scanner->original();
        QImage result = scanner->computeFilteredImage();

        if (!original.save(original_path)) {
            qWarning() << "Page could not be created: error saving original image";
            throw GeneratingError(tr("Page could not be created: error saving original image"));
        };

        if (!result.save(result_path)) {
            qWarning() << "Page could not be created: error saving result image";
            throw GeneratingError(tr("Page could not be created: error saving result image"));
        };

        emit generationFinished(original_path, result_path);

        return true;
    }));
}

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

void Page::setCreationTime(const QDateTime& creation_time)
{
    if (creation_time != d->creation_time) {
        d->creation_time = creation_time;
        emit creationTimeChanged();
    }
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
        if (!d->thumbnail_path.isEmpty()) {
            d->thumbnail_path.clear();
        }

        d->result_thumbnail.setFuture(QtConcurrent::run(this, &Page::updateThumbnail, d->result_path));
    }

    return {};
}

void Page::setThumbnail(const QString& thumbnail)
{
    if (thumbnail != d->thumbnail_path) {
        d->thumbnail_path = thumbnail;
        emit thumbnailChanged();
    }
}

void Page::onThumbnailFinished(const QString& thumbnail)
{
    setThumbnail(thumbnail);
    setStatus(Ready);
}

QString Page::original() const
{
    return d->original_path;
}

// TODO: add setters for other path properties
void Page::setOriginal(const QString& original)
{
    if (d->original_path != original) {
        d->original_path = original;
        emit originalChanged();
    }
}

QUrl Page::resultUrl() const
{
    return QUrl::fromLocalFile(result());
}

QString Page::result() const
{
    return d->result_path;
}

void Page::setResult(const QString& result)
{
    if (result != d->result_path) {
        d->result_path = result;
        emit resultChanged();
    }
}

void Page::remove()
{
    if (!d->original_path.isEmpty()) {
        QFile(d->original_path).remove();
    }
    if (!d->result_path.isEmpty()) {
        QFile(d->result_path).remove();
    }
    if (!d->thumbnail_path.isEmpty()) {
        QFile(d->thumbnail_path).remove();
    }
}

void Page::onGenerationFinished(const QString& original_path, const QString& result_path)
{
    try {
        (void)d->generating.result();
        setOriginal(original_path);
        setResult(result_path);
        setStatus(Ready);
        // start generation of thumbnail
        (void)thumbnail();
    } catch (GeneratingError& e) {
        emit error(e.message());
        setStatus(Invalid);
    }
}

QString Page::updateThumbnail(const QString& filename)
{
    // compute the resulting filename by attaching "-thumb" to the file name
    QFileInfo f(filename);
    QString scaled_filename = f.path() + QStringLiteral("/") + f.baseName() +
                              QStringLiteral("-thumb.") + f.completeSuffix();

    // load original file
    QImage img(filename);

    if (img.isNull()) {
        return {};
    }

    // scale down
    QImage scaled;
    if (img.width() > img.height()) {
        scaled = img.scaledToWidth(Page::ThumbnailSize);
    } else {
        scaled = img.scaledToHeight(Page::ThumbnailSize);
    }

    // write thumbnail to file
    scaled.save(scaled_filename);

    emit thumbnailFinished(scaled_filename);

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
    json[QStringLiteral("filters")] = d->settings;

    return true;
}

bool Page::read(const QJsonObject& json)
{
    auto page_creation_time = json[QStringLiteral("creationTime")];
    if (!page_creation_time.isString()) {
        return false;
    }
    auto page_ctime = QDateTime::fromString(page_creation_time.toString(), FilenameFormat);
    if (page_ctime.isNull()) {
        return false;
    }

    auto page_original_path = json[QStringLiteral("originalPath")];
    if (!page_original_path.isString()) {
        return false;
    }

    auto page_result_path = json[QStringLiteral("resultPath")];
    if (!page_result_path.isString()) {
        return false;
    }

    auto page_thumbnail_path = json[QStringLiteral("thumbnailPath")];
    if (!page_thumbnail_path.isString() && !page_thumbnail_path.isUndefined()) {
        return false;
    }

    // Verify that the result file exist.
    if (!QFileInfo(page_result_path.toString()).exists()) {
        return false;
    }

    setCreationTime(page_ctime);
    setOriginal(page_original_path.toString());
    setResult(page_result_path.toString());
    setThumbnail(page_thumbnail_path.toString());
    d->settings = json[QStringLiteral("filters")].toObject();

    return true;
}

QJsonObject Page::settings() const
{
    return d->settings;
}

/*
 * Copyright (c) 2018-2021 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include "Convert.hxx"
#include "Document.hxx"
#include "Fotokopierer.hxx"
#include "ScanImage.hxx"

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

static bool fixPath(const QString& filePath, const QDir& docpath, QString& result);

Page::Page()
    : d(new Data)
{
    // TODO: For some reason, that I do not understand, the 'finished' signal is not emitted.
    // We send our own signals to replace them when the concurrent work task is
    // finished. This seems to work.

    // connect(&d->result_thumbnail, &QFutureWatcher<QString>::finished, this, &Page::onThumbnailFinished);
    // connect(&d->generating, &QFutureWatcher<bool>::finished, this, &Page::onGenerationFinished);
    connect(this, &Page::thumbnailFinished, this, &Page::onThumbnailFinished, Qt::QueuedConnection);
    connect(this, &Page::generationFinished, this, &Page::onGenerationFinished, Qt::QueuedConnection);
}

Page::~Page() = default;

void Page::newFromImage(const QDir& dir, const std::shared_ptr<ScanImage>& scanImage)
{
    setStatus(Generating);

    auto ctime = QDateTime::currentDateTime();

    auto original_path = dir.filePath(ctime.toString(FilenameFormat) + QStringLiteral("-original.jpg"));

    auto ext = QStringLiteral("png");
    switch (scanImage->colorMode()) {
        case ColorizeView::FullColor:
        case ColorizeView::Gray: ext = QStringLiteral("jpg"); break;
        default: break;
    }
    auto result_path = dir.filePath(QStringLiteral("%1-result.%2")
                                        .arg(ctime.toString(FilenameFormat), ext));

    updateImage(scanImage, original_path, result_path, ctime);
}

void Page::updateFromImage(const std::shared_ptr<ScanImage>& scanImage)
{
    auto original_path = d->original_path;
    auto result_path = d->result_path;

    // the thumbnail is outdated now
    if (!d->thumbnail_path.isEmpty()) {
        QFile(d->thumbnail_path).remove();
    }
    // the result file is old and might be renamed (different image type)
    if (!result_path.isEmpty()) {
        QFile(result_path).remove();
    }

    // Remove existing data (because it is regenerated).
    setOriginal({});
    setResult({});
    setThumbnail({});

    // Compute the new result path.
    auto ext = QStringLiteral("png");
    switch (scanImage->colorMode()) {
        case ColorizeView::FullColor:
        case ColorizeView::Gray: ext = QStringLiteral("jpg"); break;
        default: break;
    }

    QFileInfo fi(result_path);
    result_path = QStringLiteral("%1/%2.%3").arg(fi.path(), fi.completeBaseName(), ext);

    updateImage(scanImage, original_path, result_path, d->creation_time);
}

void Page::updateImage(const std::shared_ptr<ScanImage>& scanImage,
                       const QString& original_path,
                       const QString& result_path,
                       const QDateTime& creation_time)
{
    setStatus(Generating);

    setCreationTime(creation_time);

    d->settings = scanImage->saveJson();

    d->generating.setFuture(QtConcurrent::run([this, scanImage, original_path, result_path]() {
        auto original = scanImage->original();
        auto result = scanImage->colorizedImage(true);

        if (!cvMatToQImage(original).save(original_path)) {
            qWarning() << "Page could not be created: error saving original image";
            throw GeneratingError(tr("Page could not be created: error saving original image"));
        };

        if (!cvMatToQImage(result).save(result_path)) {
            qWarning() << "Page could not be created: error saving result image";
            throw GeneratingError(tr("Page could not be created: error saving result image"));
        };

        emit generationFinished(original_path, result_path);

        return true;
    }));
}

QString findUniqueFilename(const QString& original_path)
{
    static const int MAX_COPIES = 1000;  // Maximal number of copies of the same page.

    auto path = original_path;
    for (int i = 1; i < MAX_COPIES; i++) {
        if (!QFileInfo::exists(path)) {
            return path;
        }

        QFileInfo f(original_path);
        path = QStringLiteral("%1/%2-%3.%4").arg(f.path()).arg(f.baseName()).arg(i).arg(f.completeSuffix());
    }

    return {};
}

void Page::initCopy(const QDir& dir, Document* sourceDoc, Page* source, bool move)
{
    setStatus(Generating);

    setCreationTime(source->d->creation_time);

    d->settings = source->d->settings;
    d->creation_time = source->d->creation_time;

    auto src_original_path = source->d->original_path;
    auto src_result_path = source->d->result_path;

    auto original_pth = dir.filePath(QFileInfo(src_original_path).fileName());
    auto result_pth = dir.filePath(QFileInfo(src_result_path).fileName());

    d->generating.setFuture(QtConcurrent::run(
        [this, src_original_path, src_result_path, original_pth, result_pth, sourceDoc, source, move]() {
            auto original_path = findUniqueFilename(original_pth);
            auto result_path = findUniqueFilename(result_pth);

            if (original_path.isEmpty() || result_path.isEmpty()) {
                return false;
            }

            if (!QFile::copy(src_original_path, original_path)) {
                return false;
            }

            if (!QFile::copy(src_result_path, result_path)) {
                QFile::remove(original_path);
                return false;
            }

            if (move && sourceDoc != nullptr) {
                emit deleteSourcePage(sourceDoc, source);
            }

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

bool Page::write(QJsonObject& json, const QDir& docpath) const
{
    json[QStringLiteral("creationTime")] = d->creation_time.toString(FilenameFormat);
    json[QStringLiteral("originalPath")] = docpath.relativeFilePath(d->original_path);
    json[QStringLiteral("resultPath")] = docpath.relativeFilePath(d->result_path);
    if (!d->thumbnail_path.isEmpty()) {
        json[QStringLiteral("thumbnailPath")] = docpath.relativeFilePath(d->thumbnail_path);
    }
    json[QStringLiteral("filters")] = d->settings;

    return true;
}

bool Page::read(const QJsonObject& json, const QDir& docpath)
{
    auto page_creation_time = json[QStringLiteral("creationTime")];
    if (!page_creation_time.isString()) {
        qDebug() << "Error: empty page creation time";
        return false;
    }
    auto page_ctime = QDateTime::fromString(page_creation_time.toString(), FilenameFormat);
    if (page_ctime.isNull()) {
        qDebug() << "Error: invalid page creation time";
        return false;
    }

    auto page_original_path = json[QStringLiteral("originalPath")];
    if (!page_original_path.isString()) {
        qDebug() << "Error: original image path is empty";
        return false;
    }

    auto page_result_path = json[QStringLiteral("resultPath")];
    if (!page_result_path.isString()) {
        qDebug() << "Error: result image path is empty";
        return false;
    }

    auto page_thumbnail_path = json[QStringLiteral("thumbnailPath")];
    if (!page_thumbnail_path.isString() && !page_thumbnail_path.isUndefined()) {
        qDebug() << "Error: thumbnail image path is invalid";
        return false;
    }

    QString pg_original_path;
    if (!fixPath(page_original_path.toString(), docpath, pg_original_path)) {
        qDebug() << "Error: original image does not exist";
        return false;
    }

    QString pg_result_path;
    if (!fixPath(page_result_path.toString(), docpath, pg_result_path)) {
        qDebug() << "Error: result image does not exist";
        return false;
    }

    QString pg_thumbnail_path;
    if (!page_thumbnail_path.isUndefined() &&
        !fixPath(page_thumbnail_path.toString(), docpath, pg_thumbnail_path)) {
        qDebug() << "Error: thumbnail does not exist, throw it away";
    }

    qDebug() << "Use orig: " << pg_original_path << " result: " << pg_result_path << " thumb: " << pg_thumbnail_path;

    setCreationTime(page_ctime);
    setOriginal(pg_original_path);
    setResult(pg_result_path);
    setThumbnail(pg_thumbnail_path);
    d->settings = json[QStringLiteral("filters")].toObject();

    return true;
}

QJsonObject Page::settings() const
{
    return d->settings;
}

/// Extract the correct absolute filename from the path in the stored json document.
///
/// Because of a bad design decision, old documents may store absolute
/// paths to all files. This is bad because documents cannot be moved
/// or copied easily. This function possibly converts absolute paths to paths
/// relative to the document directory. This works in most cases because by default
/// all images are stored directly in the document's directory.
bool fixPath(const QString& filePath, const QDir& docpath, QString& result)
{
    auto finfo = QFileInfo(filePath);

    if (finfo.isRelative()) {
        auto path = docpath.filePath(finfo.filePath());
        if (QFileInfo(path).exists()) {
            result = path;
            return true;
        }

        qDebug() << "Error: file at " << path << " does not exist";
    } else {
        if (finfo.exists()) {
            result = filePath;
            return true;
        }

        // This is actually a workaround because older versions of Fotokopierer stored
        // absolute file paths. However, this does not work well because the paths may be
        // different on different systems preventing moving a document from one system
        // to another.
        auto relative_path = docpath.filePath(QFileInfo(filePath).fileName());
        qDebug() << "Absolute file " << filePath << " does not exist, try relative path " << relative_path;
        if (QFileInfo(relative_path).exists()) {
            result = docpath.absoluteFilePath(relative_path);
            return true;
        }

        qDebug() << "Error: image at relative path " << relative_path << " does not exist";
    }

    return false;
}

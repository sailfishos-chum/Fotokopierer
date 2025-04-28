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

#include "Document.hxx"
#include "PlainImage.hxx"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonObject>
#include <QtCore/QThread>
#include <QtGui/QImage>

namespace
{
/// Task to load and scale an image asynchronously.
///
/// This thread loads the original file, scales it down, stores the thumbnail in
/// a file and the sends the resulting image as its signal.
class ThumbnailTask : public QObject
{
    Q_OBJECT

public slots:
    void run(const QString& filename)
    {
        // compute the resulting filename by attaching "-thumb" to the file name
        QFileInfo f(filename);
        QString scaled_filename = f.path() + QStringLiteral("/") + f.baseName() +
                                  QStringLiteral("-thumb.") + f.completeSuffix();

        // load original file
        QImage img(filename);

        if (img.isNull()) {
            emit resultReady({});
            return;
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

        // send result
        emit resultReady(scaled_filename);
    }

signals:
    void resultReady(const QString& path);
};

static QThread* thumbnailThread()
{
    static QThread thread;
    static bool started = false;
    if (!started) {
        started = true;
        thread.start();
    }
    return &thread;
}

}  // namespace

struct Page::Data {
    ThumbnailTask* task = nullptr;
    QDateTime creation_time;
    QString original_path;
    QString result_path;
    QString thumbnail_path;

    bool loading = false;
};

Page::Page(QObject* parent) : QObject(parent), d(new Data) {}

Page::Page(const QDateTime& creation_time,
           const QString& original_path,
           const QString& result_path,
           const QString& thumbnail_path,
           QObject* parent)
    : QObject(parent), d(new Data)
{
    d->creation_time = creation_time;
    d->original_path = original_path;
    d->result_path = result_path;
    d->thumbnail_path = thumbnail_path;
}

Page::~Page()
{
    if (d->task) d->task->deleteLater();
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

    if (!d->loading) {
        d->loading = true;
        // thumbnail does not exist, try to create it from the result image
        if (!d->thumbnail_path.isEmpty()) {
            d->thumbnail_path = QString();
        }

        if (!d->task) {
            auto task = new ThumbnailTask();
            task->moveToThread(thumbnailThread());
            connect(task, &ThumbnailTask::resultReady, this, &Page::setThumbnail);
            connect(this, &Page::refreshThumbnail, task, &ThumbnailTask::run);
            d->task = task;
        }

        emit refreshThumbnail(d->result_path);
    }

    return {};
}

void Page::setThumbnail(const QString& path)
{
    if (path != d->thumbnail_path) {
        d->thumbnail_path = path;
        emit thumbnailChanged();
    }

    d->loading = false;
    d->task->deleteLater();
    d->task = nullptr;
}

QString Page::getOriginalImagePath() const
{
    return d->original_path;
}

QString Page::getResultImagePath() const
{
    return d->result_path;
}

void Page::remove()
{
    QFile(d->original_path).remove();
    QFile(d->result_path).remove();
    QFile(d->thumbnail_path).remove();
}

bool Page::write(QJsonObject& json) const
{
    json[QStringLiteral("creationTime")] = d->creation_time.toString(Document::FilenameFormat);
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
    auto page_ctime =
        QDateTime::fromString(page_creation_time.toString(), Document::FilenameFormat);
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

#include "Page.moc"

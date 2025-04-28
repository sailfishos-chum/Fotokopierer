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

#ifndef __FOTOKOPIERER_PAGE_HXX__
#define __FOTOKOPIERER_PAGE_HXX__

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtGui/QImage>

class ScanImage;

/// A single scanned page.
class Page : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QDateTime creationTime READ creationTime CONSTANT)
    Q_PROPERTY(QString thumbnail READ thumbnail NOTIFY thumbnailChanged)
    Q_PROPERTY(QString result READ result NOTIFY resultChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

public:
    static const int ThumbnailSize = 500;

    enum Status {
        Ready,       ///< the page is ready
        Generating,  ///< the page is being generated
        Loading,     ///< the page is being loaded
        Thumbnail,   ///< the thumbnail is being created
        Invalid,     ///< the page is invalid
    };

    Q_ENUM(Status)

public:
    explicit Page(QObject* parent = nullptr);

    Page(const QDateTime& creation_time,
         const QString& original_path,
         const QString& result_path,
         const QString& thumbnail_path,
         QObject* parent);

    /// Create a new page from a scanned image.
    ///
    /// The document is placed in the given directory.
    Page(const QDir& dir, const ScanImage* scanImage, QObject* parent);

    Page(const Page&) = delete;
    Page(Page&&) = delete;
    Page& operator=(const Page&) = delete;
    Page& operator=(Page&&) = delete;

    ~Page() override;

    QDateTime creationTime() const;

    QString thumbnail();

    QString getOriginalImagePath() const;

    QString result() const;

    Status status() const;

    bool write(QJsonObject& json) const;

    bool read(const QJsonObject& json);

public slots:
    /// Delete all files associated with this page.
    void remove();

private:
    QString updateThumbnail(const QString& filename) const;

private slots:
    void generationFinished();

    void thumbnailFinished();

    void setStatus(Page::Status status);

signals:
    void thumbnailChanged();

    void resultChanged();

    void statusChanged();

private:
    struct Data;
    QScopedPointer<Data> d;
};

#endif

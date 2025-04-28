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

#ifndef __FOTOKOPIERER_PAGE_HXX__
#define __FOTOKOPIERER_PAGE_HXX__

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtGui/QImage>

#include <memory>

class ScanImage;

/// A single scanned page.
class Page : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QDateTime creationTime READ creationTime NOTIFY creationTimeChanged)
    Q_PROPERTY(QString original READ original NOTIFY originalChanged)
    Q_PROPERTY(QString thumbnail READ thumbnail NOTIFY thumbnailChanged)
    Q_PROPERTY(QString result READ result NOTIFY resultChanged)
    Q_PROPERTY(QUrl resultUrl READ resultUrl NOTIFY resultChanged)
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
    explicit Page();

    Page(const Page&) = delete;
    Page(Page&&) = delete;
    Page& operator=(const Page&) = delete;
    Page& operator=(Page&&) = delete;

    ~Page() override;

    QDateTime creationTime() const;

    QString thumbnail();

    QString original() const;

    QString result() const;

    QUrl resultUrl() const;

    Status status() const;

    /// Initialize this page from the results of a Scanner.
    ///
    /// The page files are stored in the document directory `dir`.
    void newFromImage(const QDir& dir, const std::shared_ptr<ScanImage>& scanImage);

    /// Initialize this page from the results of a Scanner.
    ///
    /// The page files reuse (and overwrite) the current files.
    void updateFromImage(const std::shared_ptr<ScanImage>& scanImage);

    bool write(QJsonObject& json, const QDir& docpath) const;

    bool read(const QJsonObject& json, const QDir& docpath);

    /// Return the filter settings of this page.
    QJsonObject settings() const;

public slots:
    /// Delete all files associated with this page.
    void remove();

private:
    void updateImage(const std::shared_ptr<ScanImage>& scanImage,
                     const QString& original_path,
                     const QString& result_path,
                     const QDateTime& creation_time);

    QString updateThumbnail(const QString& filename);

private slots:
    void onGenerationFinished(const QString& original_path, const QString& result_path);

    void onThumbnailFinished(const QString& thumbnail_path);

    void setStatus(Page::Status status);

    void setOriginal(const QString& original);

    void setResult(const QString& result);

    void setThumbnail(const QString& thumbnail);

    void setCreationTime(const QDateTime& creation_time);

signals:
    void originalChanged();

    void resultChanged();

    void thumbnailChanged();

    void creationTimeChanged();

    void statusChanged();

    void generationFinished(const QString& original_path, const QString& result_path);

    void thumbnailFinished(const QString& thumbnail_path);

    /// An error occurred.
    void error(const QString& errorMessage);

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

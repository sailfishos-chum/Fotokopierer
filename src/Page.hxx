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
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtGui/QImage>

/// A single scanned page.
class Page : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QDateTime creationTime READ creationTime)
    Q_PROPERTY(QString thumbnail READ thumbnail NOTIFY thumbnailChanged)

public:
    static const int ThumbnailSize = 300;

public:
    Page(QObject* parent = nullptr);

    Page(const QDateTime& creation_time,
         const QString& original_path,
         const QString& result_path,
         const QString& thumbnail_path,
         QObject* parent);

    ~Page();

    QDateTime creationTime() const;

    QString thumbnail();

    QString getOriginalImagePath() const;

    QString getResultImagePath() const;

    bool write(QJsonObject& json) const;

    bool read(const QJsonObject& json);

public slots:
    /// Delete all files associated with this page.
    void remove();

private slots:
    void setThumbnail(const QString& path);

signals:
    void thumbnailChanged();

private:
    struct Data;
    QScopedPointer<Data> d;
};

#endif

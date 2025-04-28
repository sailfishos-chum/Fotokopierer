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

#ifndef __FOTOKOPIERER_DOCUMENT_HXX__
#define __FOTOKOPIERER_DOCUMENT_HXX__

#include <QtCore/QAbstractListModel>
#include <QtCore/QScopedPointer>

#include <memory>

class BaseImage;
class Page;

/// A scanned document
///
/// This is an ordered collection of scanned pages.
class Document : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

public:
    enum PageRoles { ThumbnailRole = Qt::UserRole + 1 };

    static const QString FilenameFormat;

public:
    Document(QObject *parent = nullptr);

    ~Document();

    /// Return the document title.
    QString title() const;

    /// Add a newly scanned page to the document.
    ///
    /// The new page will be created with the given original and result image
    /// and the current time. It will be the last page of the current document.
    Q_INVOKABLE void addPage(BaseImage *original, BaseImage *result);

    /// Delete a page from the document.
    Q_INVOKABLE void deletePage(int pageIndex);

    Q_INVOKABLE bool save() const;

    Q_INVOKABLE bool load(const QString &filename, QObject *parent = nullptr);

public slots:
    /// Set the document title.
    void setTitle(const QString &title);

    /// Move a page `from` to position `to`.
    void move(int from, int to);

signals:
    void titleChanged();

    void error(const QString &msg);

private:
    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

private slots:
    void updateThumbnail();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

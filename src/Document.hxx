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

/// A scanned document
///
/// This is an ordered collection of scanned pages.
class Document : public QAbstractListModel
{
    Q_OBJECT

public:
    enum PageRoles { PageRole = Qt::UserRole + 1 };

    static const QString FilenameFormat;

public:
    Document(QObject *parent = nullptr);

    ~Document();

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    /// Add a newly scanned page to the image.
    ///
    /// The new page will be created with the given original and result image
    /// and the current time. It will be the last page of the current document.
    Q_INVOKABLE void addPage(BaseImage *original, BaseImage *result);

    Q_INVOKABLE bool save() const;

    Q_INVOKABLE bool load(const QString &filename, QObject *parent = nullptr);

public slots:
    /// Move a page `from` to position `to`.
    void move(int from, int to);

signals:
    void error(const QString &msg);

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

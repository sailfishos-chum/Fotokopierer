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

#ifndef __FOTOKOPIERER_DOCUMENTLIST_HXX__
#define __FOTOKOPIERER_DOCUMENTLIST_HXX__

#include <QtCore/QAbstractListModel>

#include <memory>

class Document;

/// Collection of all documents.
class DocumentList : public QAbstractListModel
{
    Q_OBJECT
public:
    enum DocumentRoles {
        TitleRole = Qt::UserRole + 1,
        CreationTimeRole,
        NumPagesRole,
        DocumentRole,
        ThumbnailsRole,
    };

public:
    DocumentList(QObject *parent = nullptr);

    ~DocumentList();

    /// Create and return a new document.
    ///
    /// On error return NULL.
    Q_INVOKABLE Document *newDocument();

    /// Delete a document from the document list.
    Q_INVOKABLE void deleteDocument(int docIndex);

private:
    void addDocument(const QSharedPointer<Document> &document);

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

private slots:
    void documentChanged();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

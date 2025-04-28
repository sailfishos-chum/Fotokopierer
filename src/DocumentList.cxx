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

#include "DocumentList.hxx"

#include "Document.hxx"
#include "Page.hxx"

#include <QtCore/QDateTime>
#include <QtCore/QSharedPointer>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>

struct DocumentList::Data {
    QList<QSharedPointer<Document>> docs;
};

DocumentList::DocumentList(QObject *parent) : QAbstractListModel(parent), d(new Data)
{
    auto doc = QSharedPointer<Document>(new Document());
    doc->load(QStandardPaths::locate(QStandardPaths::HomeLocation,
                                     QStringLiteral("fotokopierer/doc1/doc.json")));
    addDocument(doc);
}

DocumentList::~DocumentList() = default;

void DocumentList::addDocument(const QSharedPointer<Document> &doc)
{
    connect(doc.data(), &Document::pagesChanged, this, &DocumentList::documentChanged);
    connect(doc.data(), &Document::titleChanged, this, &DocumentList::documentChanged);
    connect(doc.data(), &Document::creationTimeChanged, this, &DocumentList::documentChanged);

    beginInsertRows({}, d->docs.size(), d->docs.size());
    d->docs.push_back(doc);
    endInsertRows();
}

Document *DocumentList::newDocument()
{
    auto doc = QSharedPointer<Document>(new Document(Document::create()));
    addDocument(doc);
    return doc.data();
}

void DocumentList::documentChanged()
{
    auto sender = QObject::sender();
    for (int i = 0; i < d->docs.size(); i++) {
        auto &doc = d->docs[i];
        if (doc.data() == sender) {
            auto idx = index(i);
            emit dataChanged(idx, idx, {ThumbnailsRole, TitleRole, CreationTimeRole, NumPagesRole});
        }
    }
}

int DocumentList::rowCount(const QModelIndex &parent) const
{
    return d->docs.size();
}

QVariant DocumentList::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case TitleRole: return d->docs[index.row()]->title();
        case CreationTimeRole: return d->docs[index.row()]->creationTime();
        case NumPagesRole: return d->docs.size();
        case DocumentRole: return QVariant::fromValue(d->docs[index.row()].data());
        case ThumbnailsRole: {
            QStringList thumbs;
            auto &doc = d->docs[index.row()];
            for (int i = 0, n = std::min(doc->numPages(), 3); i < n; i++) {
                thumbs.push_back(QUrl::fromLocalFile(doc->page(i).thumbnail()).toString());
            }
            return thumbs;
        }
    }
    return {};
}

QHash<int, QByteArray> DocumentList::roleNames() const
{
    static QHash<int, QByteArray> role_names = {{TitleRole, "role_title"},
                                                {CreationTimeRole, "role_creationTime"},
                                                {NumPagesRole, "role_numPages"},
                                                {DocumentRole, "role_document"},
                                                {ThumbnailsRole, "role_thumbnails"}};
    return role_names;
}

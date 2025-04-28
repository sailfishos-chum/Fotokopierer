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
#include "Fotokopierer.hxx"
#include "Page.hxx"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QSharedPointer>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtQml/QQmlEngine>

struct DocumentList::Data {
    QVector<QSharedPointer<Document>> docs;
};

DocumentList::DocumentList(QObject* parent)
    : QAbstractListModel(parent), d(new Data)
{
}

DocumentList::~DocumentList() = default;

void DocumentList::load()
{
    d->docs.clear();
    auto dir = getDocumentDirectory();
    for (auto& path : QDir(dir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        QDir docdir = dir;
        docdir.cd(path);
        if (docdir.exists(QStringLiteral("doc.json"))) {
            auto doc = QSharedPointer<Document>(new Document());
            addDocument(doc);
            doc->loadAsync(docdir.filePath(QStringLiteral("doc.json")));
        }
    }
}

Document* DocumentList::latestDocument() const
{
    if (!d->docs.isEmpty()) {
        return d->docs.last().data();
    } else {
        return nullptr;
    }
}

void DocumentList::addDocument(const QSharedPointer<Document>& doc)
{
    connect(doc.data(), &Document::pagesChanged, this, &DocumentList::onDocumentChanged);
    connect(doc.data(), &Document::titleChanged, this, &DocumentList::onDocumentChanged);
    connect(doc.data(), &Document::creationTimeChanged, this, &DocumentList::onDocumentChanged);
    connect(doc.data(), &Document::statusChanged, this, &DocumentList::onDocumentStatusChanged);
    connect(doc.data(), &Document::error, this, &DocumentList::error);

    beginInsertRows({}, d->docs.size(), d->docs.size());
    d->docs.push_back(doc);
    endInsertRows();

    emit latestDocumentChanged();
}

Document* DocumentList::newDocument()
{
    auto doc = QSharedPointer<Document>(new Document(Document::create()));
    addDocument(doc);
    auto d = doc.data();
    QQmlEngine::setObjectOwnership(d, QQmlEngine::CppOwnership);
    return d;
}

void DocumentList::deleteDocument(int docIndex)
{
    beginRemoveRows({}, docIndex, docIndex);
    auto doc = d->docs.takeAt(docIndex);
    doc->remove();
    endRemoveRows();

    emit latestDocumentChanged();
}

void DocumentList::onDocumentChanged()
{
    auto sender = QObject::sender();
    for (int i = 0; i < d->docs.size(); i++) {
        auto& doc = d->docs.at(i);
        if (doc.data() == sender) {
            auto idx = index(i);
            emit dataChanged(idx, idx, {ThumbnailsRole, TitleRole, CreationTimeRole, NumPagesRole});
        }
    }
}

void DocumentList::onDocumentStatusChanged()
{
    auto* doc = qobject_cast<Document*>(sender());

    if (doc->status() == Document::Invalid) {
        for (int i = 0; i < d->docs.size(); i++) {
            if (d->docs.at(i) == doc) {
                deleteDocument(i);
                break;
            }
        }
    }
}

int DocumentList::rowCount(const QModelIndex& parent) const
{
    (void)parent;
    return d->docs.size();
}

QVariant DocumentList::data(const QModelIndex& index, int role) const
{
    switch (role) {
        case TitleRole: return d->docs[index.row()]->title();
        case CreationTimeRole: return d->docs[index.row()]->creationTime();
        case NumPagesRole: return d->docs[index.row()]->numPages();
        case DocumentRole: return QVariant::fromValue(d->docs.at(index.row()).data());
        case ThumbnailsRole: {
            QStringList thumbs;
            thumbs.reserve(3);
            auto& doc = d->docs.at(index.row());
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

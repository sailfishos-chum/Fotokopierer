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

#include "Document.hxx"

#include "BaseImage.hxx"
#include "Page.hxx"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QJsonValueRef>
#include <QtCore/QList>
#include <QtCore/QSharedPointer>
#include <QtCore/QUrl>

#include <memory>

#include <QDebug>

const QString Document::FilenameFormat = QStringLiteral("yyyyMMddTHHmmss");

struct Document::Data {
    QString title;                      ///< document title
    QString filename;                   ///< filename of the document data
    QDateTime creation_time;            ///< time when the document has been created
    QList<QSharedPointer<Page>> pages;  ///< page of the document
};

Document::Document(QObject *parent) : QAbstractListModel(parent), d(new Data) {}

Document::~Document() = default;

QString Document::title() const
{
    return d->title;
}

void Document::setTitle(const QString &title)
{
    if (title != d->title) {
        d->title = title;
        emit titleChanged();
    }
}

int Document::numPages() const
{
    return d->pages.size();
}

Page &Document::page(int i)
{
    return *d->pages[i];
}

const Page &Document::page(int i) const
{
    return *d->pages[i];
}

QDateTime Document::creationTime() const
{
    return d->creation_time;
}

int Document::rowCount(const QModelIndex &parent) const
{
    return d->pages.size();
}

QVariant Document::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case ThumbnailRole: {
            if (index.column() == 0 && index.row() < d->pages.size()) {
                return QUrl::fromLocalFile(d->pages[index.row()]->thumbnail());
            }
            break;
        }
        case CreationTimeRole: {
            if (index.column() == 0 && index.row() < d->pages.size()) {
                return d->pages[index.row()]->creationTime();
            }
            break;
        }
    }

    return {};
}

QHash<int, QByteArray> Document::roleNames() const
{
    static const QHash<int, QByteArray> roles = {{ThumbnailRole, "role_thumbnail"},
                                                 {CreationTimeRole, "role_creationTime"}};
    return roles;
}

void Document::move(int from, int to)
{
    if (beginMoveRows({}, from, from, {}, to > from ? to + 1 : to)) {
        auto p = d->pages[from];
        d->pages.removeAt(from);
        d->pages.insert(to, p);
        endMoveRows();
        save();
        emit pagesChanged();
    }
}

void Document::addPage(BaseImage *original, BaseImage *result)
{
    QImage original_img = original->image();
    if (original_img.isNull()) {
        emit error(QStringLiteral("Page could not be created: no original image"));
        return;
    }

    QImage result_img = result->image();
    if (result_img.isNull()) {
        emit error(QStringLiteral("Page could not be created: no result image"));
        return;
    }

    auto ctime = QDateTime::currentDateTime();
    auto dir = QFileInfo(d->filename).dir();

    auto original_path =
        dir.filePath(ctime.toString(FilenameFormat) + QStringLiteral("-original.jpg"));
    auto result_path = dir.filePath(ctime.toString(FilenameFormat) + QStringLiteral("-result.png"));

    if (!original_img.save(original_path)) {
        emit error(QStringLiteral("Page could not be created: error saving original image"));
        return;
    };

    if (!result_img.save(result_path)) {
        emit error(QStringLiteral("Page could not be created: error saving result image"));
        return;
    };

    QSharedPointer<Page> p(new Page(ctime, original_path, result_path, {}, this));

    connect(p.data(), &Page::thumbnailChanged, this, &Document::updateThumbnail);

    beginInsertRows({}, d->pages.size(), d->pages.size());
    d->pages.push_back(p);
    endInsertRows();

    save();
    emit pagesChanged();
}

void Document::deletePage(int pageIndex)
{
    qDebug() << "Delete Page";

    beginRemoveRows({}, pageIndex, pageIndex);
    auto page = d->pages.takeAt(pageIndex);
    page->remove();
    endRemoveRows();
    save();
}

bool Document::save() const
{
    QFileInfo finfo(d->filename);

    if (!finfo.dir().exists()) {
        finfo.dir().mkpath(QStringLiteral("."));
    }

    QFile file(d->filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << tr("Can't write document file %1").arg(d->filename);
        return false;
    }
    QJsonObject doc;

    doc[QStringLiteral("title")] = d->title;
    doc[QStringLiteral("filename")] = d->filename;
    doc[QStringLiteral("creationTime")] = d->creation_time.toString(FilenameFormat);

    QJsonArray pages;
    for (auto page : d->pages) {
        QJsonObject p;
        if (!page->write(p)) return false;
        pages << p;
    }

    doc[QStringLiteral("pages")] = pages;

    if (file.write(QJsonDocument(doc).toJson()) < 0) return false;

    return true;
}

bool Document::load(const QString &filename, QObject *parent)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << tr("Can't open document file %1").arg(filename);
        return false;
    }

    auto docdata = file.readAll();
    auto doc = QJsonDocument::fromJson(docdata);
    auto json = doc.object();

    auto title = json[QStringLiteral("title")];
    if (!title.isString() && !title.isNull()) return false;

    auto creation_time = json[QStringLiteral("creationTime")];
    if (!creation_time.isString()) return false;
    auto ctime = QDateTime::fromString(creation_time.toString(), FilenameFormat);

    auto pages = json[QStringLiteral("pages")];
    if (!pages.isArray()) return false;

    QList<QSharedPointer<Page>> docpages;
    for (auto page : pages.toArray()) {
        if (!page.isObject()) return false;
        QSharedPointer<Page> p(new Page);
        if (!p->read(page.toObject())) return false;
        docpages.push_back(p);
        connect(p.data(), &Page::thumbnailChanged, this, &Document::updateThumbnail);
    }

    d->title = title.isString() ? title.toString() : ctime.toString();
    d->filename = filename;
    d->creation_time = ctime;
    d->pages = docpages;

    emit titleChanged();

    return true;
}

void Document::updateThumbnail()
{
    Page *page = dynamic_cast<Page *>(sender());

    for (int i = 0; i < d->pages.size(); i++) {
        if (page == d->pages[i]) {
            auto idx = index(i);
            emit dataChanged(idx, idx, {ThumbnailRole});
            save();
            emit pagesChanged();
            return;
        }
    }
}

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

#include "Page.hxx"

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

#include <memory>

#include <QDebug>

struct Document::Data {
    QString title;                      ///< document title
    QString filename;                   ///< filename of the document data
    QDateTime creation_time;            ///< time when the document has been created
    QList<QSharedPointer<Page>> pages;  ///< page of the document
};

Document::Document(QObject *parent) : QAbstractListModel(parent), d(new Data)
{
    QString path = QDir::homePath() + QStringLiteral("/fotokopierer/doc1");
    d->title = QStringLiteral("TestDocument");
    d->filename = path + QStringLiteral("/doc.json");
    d->creation_time = QDateTime::currentDateTime();

    d->pages.push_back(QSharedPointer<Page>(new Page(QDateTime::currentDateTime(),
                                                     path + QStringLiteral("/page1-original.jpg"),
                                                     path + QStringLiteral("/page1-result.jpg"),
                                                     {},
                                                     this)));
    d->pages.push_back(QSharedPointer<Page>(new Page(QDateTime::currentDateTime(),
                                                     path + QStringLiteral("/page2-original.jpg"),
                                                     path + QStringLiteral("/page2-result.jpg"),
                                                     {},
                                                     this)));
    d->pages.push_back(QSharedPointer<Page>(new Page(QDateTime::currentDateTime(),
                                                     path + QStringLiteral("/page3-original.jpg"),
                                                     path + QStringLiteral("/page3-result.jpg"),
                                                     {},
                                                     this)));
}

Document::~Document() = default;

int Document::rowCount(const QModelIndex &parent) const
{
    return d->pages.size();
}

QVariant Document::data(const QModelIndex &index, int role) const
{
    if (role == PageRole) {
        if (index.column() == 0 && index.row() < d->pages.size()) {
            return QVariant::fromValue(d->pages[index.row()].data());
        }
    }

    return {};
}

QHash<int, QByteArray> Document::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PageRole] = "role_page";
    return roles;
}

bool Document::save() const
{
    QFileInfo finfo(d->filename);
    qDebug() << "DIR " << finfo.dir();
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
    doc[QStringLiteral("creationTime")] =
        d->creation_time.toString(QStringLiteral("yyyyMMddTHHmmss"));

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
    if (!title.isString()) return false;

    auto creation_time = json[QStringLiteral("creationTime")];
    if (!creation_time.isString()) return false;
    auto ctime = QDateTime::fromString(creation_time.toString(), QStringLiteral("yyyyMMddTHHmmss"));

    auto pages = json[QStringLiteral("pages")];
    if (!pages.isArray()) return false;

    QList<QSharedPointer<Page>> docpages;
    for (auto page : pages.toArray()) {
        if (!page.isObject()) return false;
        QSharedPointer<Page> p(new Page);
        if (!p->read(page.toObject())) return false;
        docpages.push_back(p);
    }

    d->title = title.toString();
    d->filename = filename;
    d->creation_time = ctime;
    d->pages = docpages;

    return true;
}

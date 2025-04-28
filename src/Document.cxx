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

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QException>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QFutureWatcher>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QJsonValueRef>
#include <QtCore/QSharedPointer>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtCore/QVector>

#include <QtDebug>

#include <memory>

const QString Document::FilenameFormat = QStringLiteral("yyyy_MM_dd-HH_mm_ss");

namespace
{
/// Error when reading a document from files.
class ReadError : public QException
{
public:
    ReadError(const QString &message) : message_(message) {}
    ReadError(const ReadError &) = default;

    void raise() const { throw *this; }
    ReadError *clone() const { return new ReadError(*this); }

    QString message() const { return message_; }

private:
    QString message_;
};
}  // namespace

struct Document::DocData {
    QString title;                        ///< document title
    QString filename;                     ///< filename of the document data
    QDateTime creation_time;              ///< time when the document has been created
    QVector<QSharedPointer<Page>> pages;  ///< page of the document

    static DocData fromFile(const QString &filename);
};

struct Document::Data {
    DocData doc;                         ///< the document data
    QFutureWatcher<DocData> pendingDoc;  ///< the document data to be read
    Status status = Ready;               ///< the current status
};

Document::Document(QObject *parent) : QAbstractListModel(parent), d(new Data)
{
    connect(&d->pendingDoc, &QFutureWatcher<DocData>::finished, [this]() {
        try {
            setDocData(d->pendingDoc.result());
            setStatus(Ready);
        } catch (ReadError &e) {
            setStatus(Invalid);
            emit error(e.message());
        }
    });
}

Document::Document(Document &&doc) noexcept : QAbstractListModel(doc.parent()), d(std::move(doc.d))
{
}

Document::~Document() = default;

void Document::setStatus(Status status)
{
    if (status != d->status) {
        d->status = status;
        emit statusChanged();
    }
}

Document::Status Document::status() const
{
    return d->status;
}

Document Document::create(QObject *parent)
{
    Document doc(parent);

    doc.d->doc.creation_time = QDateTime::currentDateTime();
    doc.d->doc.title = doc.d->doc.creation_time.toString();
    auto dir = QStandardPaths::locate(QStandardPaths::HomeLocation,
                                      QStringLiteral("fotokopierer"),
                                      QStandardPaths::LocateDirectory);

    if (!dir.isEmpty()) {
        doc.d->doc.filename = QStringLiteral("%1/%2/doc.json")
                                  .arg(dir, doc.d->doc.creation_time.toString(FilenameFormat));
    }

    return doc;
}

QString Document::title() const
{
    return d->doc.title;
}

void Document::setTitle(const QString &title)
{
    if (title != d->doc.title) {
        d->doc.title = title;
        emit titleChanged();
    }
}

int Document::numPages() const
{
    return d->doc.pages.size();
}

Page &Document::page(int i)
{
    return *d->doc.pages[i];
}

const Page &Document::page(int i) const
{
    return *d->doc.pages[i];
}

QDateTime Document::creationTime() const
{
    return d->doc.creation_time;
}

int Document::rowCount(const QModelIndex &parent) const
{
    return d->doc.pages.size();
}

void Document::setDocData(DocData &&docdata)
{
    d->doc = std::move(docdata);
    for (auto &p : d->doc.pages) {
        connect(p.data(), &Page::thumbnailChanged, this, &Document::updateThumbnail);
    }
    emit titleChanged();
}

QVariant Document::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case ThumbnailRole: {
            if (index.column() == 0 && index.row() < d->doc.pages.size()) {
                return QUrl::fromLocalFile(d->doc.pages[index.row()]->thumbnail());
            }
            break;
        }
        case CreationTimeRole: {
            if (index.column() == 0 && index.row() < d->doc.pages.size()) {
                return d->doc.pages[index.row()]->creationTime();
            }
            break;
        }
        case ResultRole: {
            if (index.column() == 0 && index.row() < d->doc.pages.size()) {
                return QUrl::fromLocalFile(d->doc.pages[index.row()]->result());
            }
            break;
        }
    }

    return {};
}

QHash<int, QByteArray> Document::roleNames() const
{
    static const QHash<int, QByteArray> roles = {{ThumbnailRole, "role_thumbnail"},
                                                 {ResultRole, "role_result"},
                                                 {CreationTimeRole, "role_creationTime"}};
    return roles;
}

void Document::move(int from, int to)
{
    if (beginMoveRows({}, from, from, {}, to > from ? to + 1 : to)) {
        auto p = d->doc.pages[from];
        d->doc.pages.removeAt(from);
        d->doc.pages.insert(to, p);
        endMoveRows();
        save();
        emit pagesChanged();
    }
}

void Document::addPage(QImage original, QImage result)
{
    if (original.isNull()) {
        qWarning() << "Page could not be created: no original image";
        emit error(QStringLiteral("Page could not be created: no original image"));
        return;
    }

    if (result.isNull()) {
        qWarning() << "Page could not be created: no result image";
        emit error(QStringLiteral("Page could not be created: no result image"));
        return;
    }

    auto ctime = QDateTime::currentDateTime();
    auto dir = QFileInfo(d->doc.filename).dir();
    if (!dir.exists()) dir.mkpath(QStringLiteral("."));

    auto original_path =
        dir.filePath(ctime.toString(FilenameFormat) + QStringLiteral("-original.jpg"));
    auto result_path = dir.filePath(ctime.toString(FilenameFormat) + QStringLiteral("-result.png"));

    if (!original.save(original_path)) {
        qWarning() << "Page could not be created: error saving original image";
        emit error(QStringLiteral("Page could not be created: error saving original image"));
        return;
    };

    if (!result.save(result_path)) {
        qWarning() << "Page could not be created: error saving result image";
        emit error(QStringLiteral("Page could not be created: error saving result image"));
        return;
    };

    QSharedPointer<Page> p(new Page(ctime, original_path, result_path, {}, this));

    connect(p.data(), &Page::thumbnailChanged, this, &Document::updateThumbnail);
    connect(p.data(), &Page::statusChanged, this, &Document::updatePage);

    beginInsertRows({}, d->doc.pages.size(), d->doc.pages.size());
    d->doc.pages.push_back(p);
    endInsertRows();

    save();
    emit pagesChanged();
}

void Document::addScannedPage(ScanImage *image)
{
    if (d->status != Ready) {
        emit error(tr("Cannot add page, document is not ready"));
        return;
    }

    setStatus(Adding);

    auto dir = QFileInfo(d->doc.filename).dir();
    if (!dir.exists()) dir.mkpath(QStringLiteral("."));

    QSharedPointer<Page> page(new Page(dir, image, this));
    connect(page.data(), &Page::thumbnailChanged, this, &Document::updateThumbnail);
    connect(page.data(), &Page::statusChanged, this, &Document::updatePage);

    beginInsertRows({}, d->doc.pages.size(), d->doc.pages.size());
    d->doc.pages.push_back(page);
    endInsertRows();
}

void Document::updatePage()
{
    Page *page = qobject_cast<Page *>(sender());
    // TODO: this only works reliably if at most one page is modified at the same time
    if (page->status() == Page::Invalid) {
        setStatus(Ready);
        for (int i = 0; i < d->doc.pages.size(); i++) {
            if (page == d->doc.pages[i]) {
                deletePage(i);
                break;
            }
        }
    } else if (page->status() != Page::Ready) {
        setStatus(Adding);
    } else {
        setStatus(Ready);
    }
}

void Document::deletePage(int pageIndex)
{
    beginRemoveRows({}, pageIndex, pageIndex);
    auto page = d->doc.pages.takeAt(pageIndex);
    page->remove();
    endRemoveRows();
    save();
}

void Document::remove()
{
    QFileInfo finfo(d->doc.filename);
    if (finfo.exists()) {
        finfo.dir().removeRecursively();
        d.reset(new Data);
    }
}

bool Document::save() const
{
    QFileInfo finfo(d->doc.filename);

    if (!finfo.dir().exists()) {
        if (!finfo.dir().mkpath(QStringLiteral("."))) {
            qWarning() << tr("Cannot create path %1").arg(finfo.dir().path());
            return false;
        }
    }

    QFile file(d->doc.filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << tr("Can't write document file %1:%2").arg(d->doc.filename, file.error());
        return false;
    }
    QJsonObject doc;

    doc[QStringLiteral("title")] = d->doc.title;
    doc[QStringLiteral("filename")] = d->doc.filename;
    doc[QStringLiteral("creationTime")] = d->doc.creation_time.toString(FilenameFormat);

    QJsonArray pages;
    for (auto page : d->doc.pages) {
        QJsonObject p;
        if (!page->write(p)) return false;
        pages << p;
    }

    doc[QStringLiteral("pages")] = pages;

    if (file.write(QJsonDocument(doc).toJson()) < 0) {
        qWarning() << tr("Error writing document file:%1").arg(file.error());
        return false;
    }

    return true;
}

bool Document::load(const QString &filename)
{
    if (d->status != Ready) {
        emit error(tr("Cannot load document from '%1', another process is running").arg(filename));
    }

    try {
        setDocData(DocData::fromFile(filename));
        setStatus(Ready);
    } catch (ReadError &e) {
        setStatus(Invalid);
        emit error(e.message());
        qWarning() << "Error reading file: " << e.message();
        return false;
    }

    return true;
}

void Document::loadAsync(const QString &filename)
{
    if (d->status != Ready) {
        emit error(tr("Cannot load document from '%1', another process is running").arg(filename));
    }

    setStatus(Loading);
    d->pendingDoc.setFuture(
        QtConcurrent::run([this, filename]() { return DocData::fromFile(filename); }));
}

Document::DocData Document::DocData::fromFile(const QString &filename)
{
    auto tr = [](const char *source) { return QCoreApplication::translate("Document", source); };

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        throw ReadError(tr("Can't open document file %1").arg(filename));
    }

    auto docdata = file.readAll();
    auto doc = QJsonDocument::fromJson(docdata);
    auto json = doc.object();

    auto title = json[QStringLiteral("title")];
    if (!title.isString() && !title.isNull()) {
        throw ReadError(tr("Could not read document title from document file %1").arg(filename));
    }

    auto creation_time = json[QStringLiteral("creationTime")];
    if (!creation_time.isString()) {
        throw ReadError(tr("Could not read creation time from document file %1").arg(filename));
    }
    auto ctime = QDateTime::fromString(creation_time.toString(), Document::FilenameFormat);

    auto pages = json[QStringLiteral("pages")];
    if (!pages.isArray()) {
        throw ReadError(tr("Could not read pages from document file %1").arg(filename));
    }

    QVector<QSharedPointer<Page>> docpages;
    for (auto page : pages.toArray()) {
        if (!page.isObject()) {
            throw ReadError(tr("Could not read page from document file %1").arg(filename));
        }
        QSharedPointer<Page> p(new Page);
        if (!p->read(page.toObject())) {
            throw ReadError(tr("Error reading page from document file %1").arg(filename));
        }
        docpages.push_back(p);
    }

    DocData document;
    document.title = title.isString() ? title.toString() : ctime.toString();
    document.filename = filename;
    document.creation_time = ctime;
    document.pages = docpages;
    return document;
}

void Document::updateThumbnail()
{
    Page *page = qobject_cast<Page *>(sender());

    for (int i = 0; i < d->doc.pages.size(); i++) {
        if (page == d->doc.pages[i]) {
            auto idx = index(i);
            emit dataChanged(idx, idx, {ThumbnailRole});
            save();
            emit pagesChanged();
            return;
        }
    }
}

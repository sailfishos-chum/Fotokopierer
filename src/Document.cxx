/*
 * Copyright (c) 2018, 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include "Clipboard.hxx"
#include "Fotokopierer.hxx"
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
#include <QtQml/QQmlEngine>

#include <podofo/podofo.h>

#include <memory>

namespace
{
/// Error when reading a document from files.
class ReadError : public QException
{
public:
    explicit ReadError(const QString& message)
        : message_(message) {}

    ReadError(const ReadError&) = default;
    ReadError(ReadError&&) = default;
    ReadError& operator=(const ReadError&) = default;
    ReadError& operator=(ReadError&&) = default;
    ~ReadError() override = default;

    void raise() const override { throw *this; }
    ReadError* clone() const override { return new ReadError(*this); }

    QString message() const { return message_; }

private:
    QString message_;
};

struct PageData {
    QSharedPointer<Page> page;
    bool selected = false;

    PageData() = default;
    PageData(QSharedPointer<Page>&& page)
        : page(std::move(page)) {}
    PageData(const PageData&) = default;
    PageData(PageData&&) = default;
    PageData& operator=(const PageData&) = default;
    PageData& operator=(PageData&&) = default;
    ~PageData() = default;

    Page& operator*() { return *page.data(); }
    Page& operator*() const { return *page.data(); }

    Page* operator->() { return page.data(); }
    Page* operator->() const { return page.data(); }
};

}  // namespace

struct Document::DocData {
    QString title;            ///< document title
    QString filename;         ///< filename of the document data
    QDateTime creation_time;  ///< time when the document has been created
    QVector<PageData> pages;  ///< page of the document

    static DocData fromFile(Document* document, const QString& filename);
};

struct Document::Data {
    DocData doc;                         ///< the document data
    QFutureWatcher<DocData> pendingDoc;  ///< the document data to be read
    QFutureWatcher<QUrl> pendingPdf;     ///< the document es being exported to pdf
    Status status = Ready;               ///< the current status
};

Document::Document(QObject* parent)
    : QAbstractListModel(parent), d(new Data)
{
    connect(&d->pendingDoc, &QFutureWatcher<DocData>::finished, this, &Document::onPendingDocFinished);
    connect(&d->pendingPdf, &QFutureWatcher<QUrl>::finished, this, &Document::onPdfExportFinished);
    connect(this, &Document::creationTimeChanged, this, &Document::defaultTitleChanged);
}

Document::Document(Document&& doc) noexcept
    : QAbstractListModel(doc.parent()), d(std::move(doc.d))
{
    connect(&d->pendingDoc, &QFutureWatcher<DocData>::finished, this, &Document::onPendingDocFinished);
    connect(&d->pendingPdf, &QFutureWatcher<QString>::finished, this, &Document::onPdfExportFinished);
    connect(this, &Document::creationTimeChanged, this, &Document::defaultTitleChanged);
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

Document Document::create(QObject* parent)
{
    Document doc(parent);

    doc.d->doc.creation_time = QDateTime::currentDateTime();
    doc.d->doc.title = doc.defaultTitle();
    auto dir = getDocumentDirectory();

    if (dir.exists()) {
        // TODO: ensure that document does not exist, yet
        doc.d->doc.filename = dir.absoluteFilePath(doc.d->doc.creation_time.toString(FilenameFormat) +
                                                   QStringLiteral("/doc.json"));
    }

    return doc;
}

QString Document::defaultTitle() const
{
    return d->doc.creation_time.toString();
}

QString Document::title() const
{
    return d->doc.title;
}

void Document::setTitle(const QString& title)
{
    if (title != d->doc.title) {
        d->doc.title = title;
        save();
        emit titleChanged();
    }
}

int Document::numPages() const
{
    return d->doc.pages.size();
}

Page& Document::page(int i)
{
    return *d->doc.pages[i];
}

const Page& Document::page(int i) const
{
    return *d->doc.pages[i];
}

QDateTime Document::creationTime() const
{
    return d->doc.creation_time;
}

QStringList Document::thumbnails() const
{
    QStringList thumbnails;
    thumbnails.reserve(3);

    for (int i = 0; i < qMin(3, d->doc.pages.size()); i++) {
        thumbnails.push_back(QUrl::fromLocalFile(d->doc.pages.at(i)->thumbnail()).toString());
    }

    return thumbnails;
}

int Document::rowCount(const QModelIndex& parent) const
{
    return d->doc.pages.size();
}

QDir Document::directory() const
{
    return QFileInfo(d->doc.filename).dir();
}

void Document::onPendingDocFinished()
{
    try {
        setDocData(d->pendingDoc.result());
        setStatus(Ready);
    } catch (ReadError& e) {
        emit error(e.message());
        setStatus(Invalid);
    }
}

void Document::setDocData(DocData&& docdata)
{
    d->doc = std::move(docdata);
    for (auto& p : d->doc.pages) {
        connect(p.page.data(), &Page::thumbnailChanged, this, &Document::onThumbnailUpdated);
    }
    emit titleChanged();
}

QVariant Document::data(const QModelIndex& index, int role) const
{
    switch (role) {
        case ThumbnailRole: {
            if (index.column() == 0 && index.row() < d->doc.pages.size()) {
                return QUrl::fromLocalFile(d->doc.pages.at(index.row())->thumbnail());
            }
            break;
        }
        case CreationTimeRole: {
            if (index.column() == 0 && index.row() < d->doc.pages.size()) {
                return d->doc.pages.at(index.row())->creationTime();
            }
            break;
        }
        case ResultRole: {
            if (index.column() == 0 && index.row() < d->doc.pages.size()) {
                return QUrl::fromLocalFile(d->doc.pages.at(index.row())->result());
            }
            break;
        }
        case PageRole: {
            if (index.column() == 0 && index.row() < d->doc.pages.size()) {
                return QVariant::fromValue(d->doc.pages.at(index.row()).page.data());
            }
            break;
        }
        case SelectionRole: {
            if (index.column() == 0 && index.row() < d->doc.pages.size()) {
                return QVariant::fromValue(d->doc.pages.at(index.row()).selected);
            }
            break;
        }
    }

    return {};
}

bool Document::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (role) {
        case SelectionRole: {
            if (index.column() == 0 && index.row() < d->doc.pages.size()) {
                auto selected = value.toBool();
                auto& page = d->doc.pages[index.row()];
                if (selected != page.selected) {
                    page.selected = selected;
                    emit dataChanged(index, index, {SelectionRole});
                }
            }
            break;
        }
    }
    return false;
}

QHash<int, QByteArray> Document::roleNames() const
{
    static const QHash<int, QByteArray> roles = {{ThumbnailRole, "role_thumbnail"},
                                                 {ResultRole, "role_result"},
                                                 {CreationTimeRole, "role_creationTime"},
                                                 {PageRole, "role_page"},
                                                 {SelectionRole, "role_selected"}};
    return roles;
}

void Document::move(int from, int to)
{
    if (beginMoveRows({}, from, from, {}, to > from ? to + 1 : to)) {
        auto p = d->doc.pages.at(from);
        d->doc.pages.removeAt(from);
        d->doc.pages.insert(to, p);
        endMoveRows();
        save();
        emit pagesChanged();
    }
}

void Document::clearSelection()
{
    for (int i = 0; i < d->doc.pages.count(); i++) {
        auto& p = d->doc.pages[i];
        if (p.selected) {
            p.selected = false;
            auto idx = index(i, 0);
            emit dataChanged(idx, idx, {SelectionRole});
        }
    }
}

void Document::copySelectedPages()
{
    QVector<Page*> pages;
    for (auto& p : d->doc.pages) {
        if (p.selected) {
            pages.push_back(p.page.data());
        }
    }
    Clipboard::instance()->copy(this, pages);
}

void Document::cutSelectedPages()
{
    QVector<Page*> pages;
    for (auto& p : d->doc.pages) {
        if (p.selected) {
            pages.push_back(p.page.data());
        }
    }
    Clipboard::instance()->cut(this, pages);
}

void Document::pastePages()
{
}

Page* Document::newPage()
{
    if (d->status != Ready) {
        emit error(tr("Cannot add page, document is not ready"));
        return nullptr;
    }

    auto dir = directory();
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }

    QSharedPointer<Page> page(new Page(this));
    connect(page.data(), &Page::thumbnailChanged, this, &Document::onThumbnailUpdated);
    connect(page.data(), &Page::statusChanged, this, &Document::onPageUpdated);
    connect(page.data(), &Page::error, this, &Document::error);

    beginInsertRows({}, d->doc.pages.size(), d->doc.pages.size());
    d->doc.pages.push_back({std::move(page)});
    endInsertRows();

    emit pagesChanged();

    QQmlEngine::setObjectOwnership(page.data(), QQmlEngine::CppOwnership);

    return page.data();
}

Page* Document::newCopiedPage(const Page* source)
{
    if (source == nullptr) {
        return nullptr;
    }

    auto page = newPage();
    if (page != nullptr) {
        page->initCopy(directory(), source);
    }

    return page;
}

void Document::onPageUpdated()
{
    Page* page = qobject_cast<Page*>(sender());
    // TODO: this only works reliably if at most one page is modified at the same time
    if (page->status() == Page::Invalid) {
        setStatus(Ready);
        for (int i = 0; i < d->doc.pages.size(); i++) {
            if (page == d->doc.pages[i].page) {
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
    emit pagesChanged();
    save();
}

void Document::deletePage(Page* page)
{
    for (int pageIndex = 0; pageIndex < d->doc.pages.count(); pageIndex++) {
        if (d->doc.pages[pageIndex].page == page) {
            deletePage(pageIndex);
            break;
        }
    }
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
        qWarning() << tr("Can't write document file %1:%2").arg(d->doc.filename).arg(file.error());
        return false;
    }
    QJsonObject doc;

    doc[QStringLiteral("title")] = d->doc.title;
    doc[QStringLiteral("filename")] = d->doc.filename;
    doc[QStringLiteral("creationTime")] = d->doc.creation_time.toString(FilenameFormat);

    QJsonArray pages;
    for (auto& page : d->doc.pages) {
        QJsonObject p;
        if (!page->write(p)) {
            return false;
        }
        pages << p;
    }

    doc[QStringLiteral("pages")] = pages;

    if (file.write(QJsonDocument(doc).toJson()) < 0) {
        qWarning() << tr("Error writing document file:%1").arg(file.error());
        return false;
    }

    ensureNoMedia(QFileInfo(d->doc.filename).absolutePath());

    return true;
}

bool Document::load(const QString& filename)
{
    if (d->status != Ready) {
        emit error(tr("Cannot load document from '%1', another process is running").arg(filename));
    }

    try {
        setDocData(DocData::fromFile(this, filename));
        setStatus(Ready);
    } catch (ReadError& e) {
        setStatus(Invalid);
        emit error(e.message());
        qWarning() << "Error reading file: " << e.message();
        return false;
    }

    return true;
}

void Document::loadAsync(const QString& filename)
{
    if (d->status != Ready) {
        emit error(tr("Cannot load document from '%1', another process is running").arg(filename));
    }

    setStatus(Loading);
    d->pendingDoc.setFuture(
        QtConcurrent::run([this, filename]() {
            return DocData::fromFile(this, filename);
        }));
}

Document::DocData Document::DocData::fromFile(Document* document, const QString& filename)
{
    auto tr = [](const char* source) { return QCoreApplication::translate("Document", source); };

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
    auto ctime = QDateTime::fromString(creation_time.toString(), FilenameFormat);

    auto pages = json[QStringLiteral("pages")];
    if (!pages.isArray()) {
        throw ReadError(tr("Could not read pages from document file %1").arg(filename));
    }

    QVector<PageData> docpages;
    for (auto&& page : pages.toArray()) {
        if (!page.isObject()) {
            throw ReadError(tr("Could not read page from document file %1").arg(filename));
        }
        QSharedPointer<Page> p(new Page);

        connect(p.data(), &Page::thumbnailChanged, document, &Document::onThumbnailUpdated);
        connect(p.data(), &Page::statusChanged, document, &Document::onPageUpdated);
        connect(p.data(), &Page::error, document, &Document::error);

        if (!p->read(page.toObject())) {
            throw ReadError(tr("Error reading page from document file %1").arg(filename));
        }
        p->moveToThread(QCoreApplication::instance()->thread());
        docpages.push_back({std::move(p)});
    }

    ensureNoMedia(QFileInfo(file).absolutePath());

    DocData d;
    d.title = title.isString() ? title.toString() : ctime.toString();
    d.filename = filename;
    d.creation_time = ctime;
    d.pages = docpages;
    return d;
}

void Document::onThumbnailUpdated()
{
    Page* page = qobject_cast<Page*>(sender());

    for (int i = 0; i < d->doc.pages.size(); i++) {
        if (page == d->doc.pages[i].page) {
            auto idx = index(i);
            emit dataChanged(idx, idx, {ThumbnailRole});
            save();
            emit pagesChanged();
            return;
        }
    }
}

void Document::ensureNoMedia(const QString& path)
{
    auto dir = QDir(path);
    if (!dir.exists(QStringLiteral(".nomedia"))) {
        QFile nomedia(dir.absoluteFilePath(QStringLiteral(".nomedia")));
        nomedia.open(QIODevice::WriteOnly);
    }
}

static PoDoFo::PdfString toPdfString(const QString& str)
{
    return {str.toUtf8().constData()};
}

void Document::exportToPdf(const QString& filename, bool overwrite)
{
    using namespace PoDoFo;

    if (d->status != Ready) {
        qWarning() << "Document not ready";
        emit error(tr("Cannot export to pdf, document is not ready"));
        return;
    }

    if (QFileInfo::exists(filename) && !overwrite) {
        emit errorPdfExists(QFileInfo(filename).fileName());
        return;
    }

    setStatus(Exporting);

    QString title = d->doc.title;
    QStringList pageimages;
    for (auto& page : d->doc.pages) {
        pageimages.push_back(page->result());
    }

    d->pendingPdf.setFuture(QtConcurrent::run([title, pageimages, filename]() {
        PdfStreamedDocument pdf(filename.toUtf8().constData());
        PdfPainter painter;

        for (auto& page : pageimages) {
            PdfImage pageimage(&pdf);
            pageimage.LoadFromFile(page.toUtf8().data());

            auto pdfpage = pdf.CreatePage({0.0, 0.0, pageimage.GetWidth(), pageimage.GetHeight()});
            if (pdfpage == nullptr) {
                PODOFO_RAISE_ERROR(ePdfError_InvalidHandle);
            }

            painter.SetPage(pdfpage);

            painter.DrawImage(0.0, 0.0, &pageimage);

            painter.FinishPage();
        }

        pdf.GetInfo()->SetCreator(toPdfString(ApplicationName));
        pdf.GetInfo()->SetTitle(toPdfString(title));
        pdf.Close();

        return QUrl::fromLocalFile(filename);
    }));
}

void Document::exportToPdf(bool overwrite)
{
    exportToPdf(getDocumentDirectory().filePath(d->doc.title) + QStringLiteral(".pdf"), overwrite);
}

void Document::onPdfExportFinished()
{
    try {
        auto path = d->pendingPdf.result();
        emit exportToPdfFinished(path);
    } catch (QUnhandledException& e) {
        emit error(tr("Error creating pdf-file"));
    }

    setStatus(Ready);
}

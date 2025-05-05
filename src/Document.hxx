/*
 * Copyright (c) 2018, 2019, 2021 Frank Fischer <frank-fischer@shadow-soft.de>
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
#include <QtCore/QDateTime>
#include <QtCore/QFutureWatcher>
#include <QtGui/QImage>
#include <memory>

class Page;

class QDir;

/// A scanned document
///
/// This is an ordered collection of scanned pages.
class Document : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString defaultTitle READ defaultTitle NOTIFY defaultTitleChanged)
    Q_PROPERTY(QDateTime creationTime READ creationTime NOTIFY creationTimeChanged)
    Q_PROPERTY(QStringList thumbnails READ thumbnails NOTIFY pagesChanged)
    Q_PROPERTY(int numPages READ numPages NOTIFY pagesChanged)
    Q_PROPERTY(int numSelectedPages READ numSelectedPages NOTIFY selectedPagesChanged)
    Q_PROPERTY(bool hasSelectedPages READ hasSelectedPages NOTIFY selectedPagesChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

public:
    enum PageRoles { ThumbnailRole = Qt::UserRole + 1,
                     ResultRole,
                     CreationTimeRole,
                     PageRole,
                     SelectionRole };

    enum Status {
        Ready,      ///< Document is ready
        Invalid,    ///< Document has become invalid (e.g. error during loading)
        Loading,    ///< Document is being loaded
        Adding,     ///< A page is being added.
        Exporting,  ///< The document is being exported to pdf
    };
    Q_ENUM(Status)

    struct DocData;

public:
    explicit Document(QObject *parent = nullptr);

    Document(Document &&doc) noexcept;

    Document(const Document &) = delete;

    ~Document() override;

    Document &operator=(Document &&) = delete;

    Document &operator=(const Document &) = delete;

    /// Create a new document with the current time.
    static Document create(QObject *parent = nullptr);

    /// Return the document title.
    QString title() const;

    /// Return the document creation time.
    QDateTime creationTime() const;

    /// Return the default title of the document.
    QString defaultTitle() const;

    /// Return the number of pages.
    int numPages() const;

    /// Return the i-th page.
    Page &page(int i);

    /// Return the i-th page.
    const Page &page(int i) const;

    /// Return the thumbnails of this Document.
    ///
    /// This is the same as returned by the `ThumbnailRole` model role but
    /// accessibly as a property.
    QStringList thumbnails() const;

    /// Create and return a new empty page.
    ///
    /// Return nullptr if the document is not Ready.
    Page *newPage();

    /// Delete a page from the document.
    Q_INVOKABLE void deletePage(int pageIndex);

    /// Delete a page from the document.
    void deletePage(Page *page);

    /// Create and return a new page which is a copy of the given page.
    Page *newCopiedPage(Document *sourceDoc, Page *source, bool move = false);

    /// Delete this document.
    ///
    /// Calling this function removes all files associated with this document.
    void remove();

    /// Save document.
    Q_INVOKABLE bool save() const;

    /// Load document from the given file.
    Q_INVOKABLE bool load(const QString &filename);

    /// Load document from the given file asynchronously.
    Q_INVOKABLE void loadAsync(const QString &filename);

    /// Return the current status.
    Status status() const;

    /// Return the document's directory.
    QDir directory() const;

public slots:
    /// Set the document title.
    void setTitle(const QString &title);

    /// Move a page `from` to position `to`.
    void move(int from, int to);

    /// Return true if there is at least one selected page
    bool hasSelectedPages() const;

    /// Return the number of selected pages.
    int numSelectedPages() const;

    /// Cancel the selection of all pages.
    void clearSelection();

    /// Copy the currently selected pages to the clipboard.
    void copySelectedPages();

    /// Cut the currently selected pages to the clipboard.
    void cutSelectedPages();

    /// Delete the currently selected pages.
    void deleteSelectedPages();

    /// Paste pages from the clipboard.
    void pastePages();

    /// Export document as PDF to a file with the given name.
    ///
    /// If the file exists and `overwrite` is `true` the file will be replaced.
    /// If `overwrite` is false the signal `errorPdfExists` is raised.
    void exportToPdf(const QString &filename, bool overwrite = false);

    /// Export document as PDF to a file with the default file name.
    ///
    /// If the file exists and `overwrite` is `true` the file will be replaced.
    /// If `overwrite` is false the signal `errorPdfExists` is raised.
    void exportToPdf(bool overwrite = false);

private:
    /// Set the document data.
    void setDocData(DocData &&docdata);

    /// Do the pdf export according to the used backend.
    static void doExportToPdf(const QString& filename, const QString& title, const QStringList& pageimages);

    /// Fetch the pending pdf result, check for backend specific exceptions.
    QUrl doGetPendingPdf(const QFutureWatcher<QUrl>& pendingPdf);

private slots:
    /// Change the current status.
    void setStatus(Document::Status status);

    /// The asynchronously loaded document data is ready.
    void onPendingDocFinished();

    /// The status of a page has changed.
    void onPageUpdated();

    /// Called when the pdf export has been completed.
    void onPdfExportFinished();

signals:
    void titleChanged();

    void defaultTitleChanged();

    void creationTimeChanged();

    /// Signal emitted when at least on of the document's pages changed.
    ///
    /// This could be a new thumbnail, creation time or the order of the pages.
    void pagesChanged();

    void selectedPagesChanged();

    /// Status changed.
    void statusChanged();

    /// The document has been exported to a pdf.
    void exportToPdfFinished(const QUrl &path);

    /// Error raised when the exported file already exists.
    void errorPdfExists(const QString &filename);

    /// An error has been raised.
    void error(const QString &errorMessage);

private:
    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QHash<int, QByteArray> roleNames() const override;

    /// Ensures the ".nomedia" file exists in the given directory.
    static void ensureNoMedia(const QString &path);

private slots:
    void onDeleteSourcePage(Document *sourceDoc, Page *source);
    void onThumbnailUpdated();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

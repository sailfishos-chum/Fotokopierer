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
#include <QtCore/QDateTime>
#include <QtGui/QImage>

#include <memory>

class Page;

/// A scanned document
///
/// This is an ordered collection of scanned pages.
class Document : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QDateTime creationTime READ creationTime NOTIFY creationTimeChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

public:
    enum PageRoles { ThumbnailRole = Qt::UserRole + 1, ResultRole, CreationTimeRole };

    enum Status {
        Ready,    ///< Document is ready,
        Loading,  ///< Document is being loaded
        Adding    ///< A page is being added.
    };
    Q_ENUM(Status)

    static const QString FilenameFormat;

    struct DocData;

public:
    explicit Document(QObject *parent = nullptr);

    Document(Document &&doc) noexcept;

    ~Document() override;

    /// Create a new document with the current time.
    static Document create(QObject *parent = nullptr);

    /// Return the document title.
    QString title() const;

    /// Return the document creation time.
    QDateTime creationTime() const;

    /// Return the number of pages.
    int numPages() const;

    /// Return the i-th page.
    Page &page(int i);

    /// Return the i-th page.
    const Page &page(int i) const;

    /// Delete a page from the document.
    Q_INVOKABLE void deletePage(int pageIndex);

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

public slots:
    /// Set the document title.
    void setTitle(const QString &title);

    /// Move a page `from` to position `to`.
    void move(int from, int to);

    /// Add a newly scanned page to the document.
    ///
    /// The new page will be created with the given original and result image
    /// and the current time. It will be the last page of the current document.
    void addPage(QImage original, QImage result);

private:
    /// Set the document data.
    void setDocData(DocData &&docdata);

private slots:
    /// Change the current status.
    void setStatus(Status status);

signals:
    void titleChanged();

    void creationTimeChanged();

    /// Signal emitted when at least on of the document's pages changed.
    ///
    /// This could be a new thumbnail, creation time or the order of the pages.
    void pagesChanged();

    /// Status changed.
    void statusChanged();

    /// An error has been raised.
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

/*
 * Copyright (c) 2019-2021 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __FOTOKOPIERER_CLIPBOARD_HXX__
#define __FOTOKOPIERER_CLIPBOARD_HXX__

#include <QtCore/QObject>
#include <memory>

class Document;
class Page;

/// The global clipboard of pages.
class Clipboard : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)
    Q_PROPERTY(int numPages READ numPages NOTIFY emptyChanged)

public:
    Clipboard();

    ~Clipboard();

    /// Return true iff the clipboard is empty.
    bool isEmpty() const;

    /// Return the number of pages in the clipboard.
    int numPages() const;

    /// Copy the given pages from the given document to the clipboard.
    void copy(Document* doc, const QVector<Page*>& pages);

    /// Cut the given pages from the given document to the clipboard.
    void cut(Document* doc, const QVector<Page*>& pages);

    /// Paste the clipboard to the specified document.
    void paste(Document* target);

    static Clipboard* instance();

public slots:
    /// Clear the clipboard.
    void clear();

signals:
    void emptyChanged();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

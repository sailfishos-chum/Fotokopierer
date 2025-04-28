/*
 * Copyright (c) 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include "Clipboard.hxx"

#include "Document.hxx"
#include "Page.hxx"

Q_GLOBAL_STATIC(Clipboard, clipboard);

struct Clipboard::Data {
    Document* doc = nullptr;
    QVector<Page*> pages;
    bool cut = false;
};

Clipboard::Clipboard()
    : d(new Data)
{
}

Clipboard::~Clipboard() = default;

Clipboard* Clipboard::instance()
{
    return clipboard;
}

bool Clipboard::isEmpty() const
{
    return d->doc != nullptr;
}

int Clipboard::numPages() const
{
    return d->pages.count();
}

void Clipboard::copy(Document* doc, const QVector<Page*>& pages)
{
    clear();

    if (doc != nullptr) {
        d->doc = doc;
        d->pages = pages;

        connect(doc, &QObject::destroyed, this, &Clipboard::clear);
        connect(doc, &Document::pagesChanged, this, &Clipboard::clear);
        emit emptyChanged();
    }
}

void Clipboard::cut(Document* doc, const QVector<Page*>& pages)
{
    copy(doc, pages);
    d->cut = true;
}

void Clipboard::paste(Document* target)
{
    if (target == nullptr) return;

    for (auto& p : d->pages) {
        target->newCopiedPage(p);
    }

    if (d->cut) {
        for (auto& p : d->pages) {
            d->doc->deletePage(p);
        }
        clear();
    }
}

void Clipboard::clear()
{
    if (d->doc != nullptr) {
        disconnect(d->doc, &QObject::destroyed, this, &Clipboard::clear);
        disconnect(d->doc, &Document::pagesChanged, this, &Clipboard::clear);
        d->doc = nullptr;
        d->pages.clear();
        emit emptyChanged();
    }
}

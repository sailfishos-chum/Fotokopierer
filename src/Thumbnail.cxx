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

#include "Thumbnail.hxx"

#include "Page.hxx"

#include <QtGui/QImage>

struct Thumbnail::Data {
    QImage image;
    Page* page = nullptr;
};

Thumbnail::Thumbnail() : d(new Data) {}

Thumbnail::~Thumbnail() = default;

QImage Thumbnail::sourceImage() const
{
    return d->image;
}

Page* Thumbnail::page() const
{
    return d->page;
}

void Thumbnail::setPage(Page* page)
{
    qDebug() << "Set thumbnail page";

    if (page != d->page) {
        if (d->page != nullptr) {
            disconnect(d->page, &Page::thumbnailChanged, this, &Thumbnail::updateThumbnail);
        }

        d->page = page;

        if (d->page != nullptr) {
            connect(d->page, &Page::thumbnailChanged, this, &Thumbnail::updateThumbnail);
        }

        emit pageChanged();
        updateThumbnail();
    }
}

QImage Thumbnail::transform(const QImage& image)
{
    return image;
}

void Thumbnail::updateThumbnail()
{
    d->image = d->page->thumbnail();
    emit sourceChanged();
}

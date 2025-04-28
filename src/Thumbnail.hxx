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

#ifndef __FOTOKOPIERER_THUMBNAIL_HXX__
#define __FOTOKOPIERER_THUMBNAIL_HXX__

#include "BaseImage.hxx"

class Page;

/// A thumbnail of a page.
class Thumbnail : public BaseImage
{
    Q_OBJECT

    Q_PROPERTY(Page* page READ page WRITE setPage NOTIFY pageChanged)

public:
    Thumbnail();

    ~Thumbnail();

    QImage sourceImage() const override;

    Page* page() const;

public slots:
    void setPage(Page* page);

signals:
    void pageChanged();

protected:
    QImage transform(const QImage& image) override;

private slots:
    void updateThumbnail();

private:
    struct Data;
    QScopedPointer<Data> d;
};

#endif

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

#ifndef __FOTOKOPIERER_ASYNCIMAGE_HXX__
#define __FOTOKOPIERER_ASYNCIMAGE_HXX__

#include "BaseImage.hxx"

/// A base image with asynchronous transformation.
class AsyncImage : public BaseImage
{
    Q_OBJECT

    friend class AsyncImageTask;

public:
    AsyncImage(QQuickItem* parent = nullptr);

    ~AsyncImage();

signals:
    void startTransform(const QImage& image);

protected:
    void updateImage() override;

private slots:
    void finishTransform();

private:
    struct Data;
    QScopedPointer<Data> d;
};

#endif

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

#ifndef __FOTOKOPIERER_ROTIMAGE_HXX__
#define __FOTOKOPIERER_ROTIMAGE_HXX__

#include "BaseImage.hxx"

/// An image that can be rotated.
class RotImage : public BaseImage
{
public:
    Q_OBJECT

    Q_PROPERTY(int rotation READ rotation WRITE setRotation NOTIFY rotationChanged)

public:
    RotImage();

    ~RotImage();

    int rotation() const;

public slots:
    void setRotation(int rotation);

signals:
    void rotationChanged();

protected:
    QImage transform(const QImage& image);

private:
    int rotation_;
};

#endif

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

#ifndef __FOTOKOPIERER_CUTIMAGE_HXX__
#define __FOTOKOPIERER_CUTIMAGE_HXX__

#include "AsyncImage.hxx"

#include <QtCore/QScopedPointer>

/// An image from which an quadrangle can be cut.
class CutImage : public AsyncImage
{
    Q_OBJECT

public:
    CutImage();

    ~CutImage();

    /// Set the corner points of the cut box.
    ///
    /// If the cut box is not convex return false otherwise return
    /// true.
    Q_INVOKABLE bool setCutBox(const QPointF& topleft,
                               const QPointF& topright,
                               const QPointF& bottomright,
                               const QPointF& bottomleft);

    /// Return the auto-detected cut box.
    ///
    /// The method returns a list of four points (topleft, topright,
    /// bottomright, bottomleft).
    Q_INVOKABLE QVariantList autoDetectCutRect();

protected:
    QImage transform(const QImage& image);

private:
    struct Data;
    QScopedPointer<Data> d;
};

#endif

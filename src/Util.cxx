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

#include "Util.hxx"

#include "PlainImage.hxx"

#include <QtCore/QLineF>

bool Util::isConvex(const QPointF &x1, const QPointF &x2, const QPointF &x3, const QPointF &x4)
{
    return QLineF(x1, x3).intersect(QLineF(x2, x4), nullptr) == QLineF::BoundedIntersection;
}

PlainImage *Util::loadPlainImage(const QString &filename)
{
    auto image = new PlainImage();
    image->loadFile(filename);
    return image;
}

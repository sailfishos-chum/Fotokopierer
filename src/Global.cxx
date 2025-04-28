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

#include "Global.hxx"

#include "PlainImage.hxx"

#include <QtCore/QLineF>

bool Fotokopierer::isConvex(QPointF x1, QPointF x2, QPointF x3, QPointF x4)
{
    return QLineF(x1, x3).intersect(QLineF(x2, x4), nullptr) == QLineF::BoundedIntersection;
}

PlainImage *Fotokopierer::loadPlainImage(const QString &filename)
{
    auto image = new PlainImage();
    image->loadFile(filename);
    return image;
}

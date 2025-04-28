/*
 * Copyright (c) 2021 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include "ColorizeChooser.hxx"

#include <QtGui/QPainter>
#include "fifr/util/Range.hxx"

using namespace fifr::util;

struct ColorizeChooser::Data {
    int lattice = 100;
};

ColorizeChooser::ColorizeChooser(QQuickItem* parent)
    : QQuickPaintedItem(parent), d(new Data)
{
}

ColorizeChooser::~ColorizeChooser() = default;

void ColorizeChooser::setLattice(int lattice)
{
    lattice = qBound(1, lattice, 100);
    if (lattice != d->lattice) {
        d->lattice = lattice;
        emit latticeChanged();
    }
}

int ColorizeChooser::lattice() const
{
    return d->lattice;
}

void ColorizeChooser::paint(QPainter* painter)
{
    auto size = std::min(width(), height());

    QConicalGradient gradient(width() / 2, height() / 2, 0);

    for (auto deg : range(360)) {
        gradient.setColorAt(deg / 360.0, QColor::fromHsvF(deg / 360.0, 1, 1));
    }

    painter->setBrush(gradient);
    painter->drawEllipse(QPointF{width() / 2, height() / 2}, size / 2, size / 2);
}

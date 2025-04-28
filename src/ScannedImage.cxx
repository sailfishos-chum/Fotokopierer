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

#include "ScannedImage.hxx"

#include "ScannedImageProvider.hxx"

#include <QtGui/QPixmap>

struct ScannedImage::Data {
    QString image;
};

ScannedImage::ScannedImage(QObject* parent) : QObject(parent), d(new Data)
{
    d->image = QLatin1String("0");
}

ScannedImage::~ScannedImage() {}

QString ScannedImage::originalImage() const
{
    return d->image + QLatin1String("/original");
}

QString ScannedImage::cutImage() const
{
    return d->image + QLatin1String("/cut");
}

void ScannedImage::set_cut_image(double angle, const QPointF& topleft, const QPointF& topright,
                                 const QPointF& bottomright, const QPointF& bottomleft)
{
    ScannedImageProvider::instance->set_cut_image(d->image, angle, topleft, topright, bottomright,
                                                  bottomleft);
}

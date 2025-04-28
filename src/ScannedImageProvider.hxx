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

#ifndef __FOTOKOPIERER_SCANNEDIMAGEPROVIDER_HXX__
#define __FOTOKOPIERER_SCANNEDIMAGEPROVIDER_HXX__

#include <QtCore/QScopedPointer>
#include <QtQuick/QQuickImageProvider>

class ScannedImageProvider : public QQuickImageProvider
{
public:
    ScannedImageProvider();

    ~ScannedImageProvider();

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

    /// Load a (original) image from the given file and return the ID.
    ///
    /// Return an empty string if the image could not be loaded.
    QString loadImage(const QString& fileName);

    /// The set cut image according to the given rotation angle and corner points.
    void set_cut_image(const QString& image, double angle, const QPointF& topleft,
                       const QPointF& topright, const QPointF& bottomright,
                       const QPointF& bottomleft);

public:
    /// A global instance used throughout the app.
    static ScannedImageProvider* instance;

private:
    struct Data;
    QScopedPointer<Data> d;
};

#endif

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
    /// The color mode to be used.
    enum ColorMode {
        Gray,
        BlackAndWhite,
        Colored,
    };

public:
    ScannedImageProvider();

    ~ScannedImageProvider();

    QImage requestImage(const QString& id,
                        QSize* size,
                        const QSize& requestedSize) override;

    /// Load a (original) image from the given file and return the ID.
    ///
    /// Return an empty string if the image could not be loaded.
    QString loadImage(const QString& fileName);

    /// Set the rotation angle of a certain image.
    void setAngle(const QString& image, double angle);

    /// The set corners of the cut quadrangle of a certain image.
    ///
    /// The method returns `false` if the corners do not form a valid
    /// quadrangle (e.g. if it is not convex).
    bool setCutBox(const QString& image,
                   const QPointF& topleft,
                   const QPointF& topright,
                   const QPointF& bottomright,
                   const QPointF& bottomleft);

    /// Auto detect cut box.
    ///
    /// Sets and returns the corner points.
    QList<QPointF> autoDetectCutRect(const QString& image);

    /// Set the contrast of the image.
    void setContrast(const QString& image, double contrast);

    /// Set the brightness of the image.
    void setBrightness(const QString& image, double brightness);

    void setColorMode(const QString& image, ColorMode colormode);

    /// Set the detail level.
    void setDetails(const QString& image, double details);

public:
    /// A global instance used throughout the app.
    static ScannedImageProvider* instance;

private:
    struct Data;
    QScopedPointer<Data> d;
};

#endif

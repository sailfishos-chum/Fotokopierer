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

#ifndef __FOTOKOPIERER_COLORIZEIMAGE_HXX__
#define __FOTOKOPIERER_COLORIZEIMAGE_HXX__

#include "AsyncImage.hxx"

/// This class can be used to modify the colors of an image.
class ColorizeImage : public AsyncImage
{
    Q_OBJECT

    Q_PROPERTY(double contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    Q_PROPERTY(double brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(double details READ details WRITE setDetails NOTIFY detailsChanged)
    Q_PROPERTY(ColorMode colorMode READ colorMode WRITE setColorMode NOTIFY colorModeChanged)

public:
    /// The color mode to be used.
    enum ColorMode {
        Gray,
        BlackAndWhite,
        Colored,
    };
    Q_ENUM(ColorMode)

public:
    ColorizeImage(QQuickItem* parent = nullptr);

    ~ColorizeImage();

    /// Return the contrast level.
    double contrast() const;

    /// Return the brightness level.
    double brightness() const;

    /// Return the details level.
    double details() const;

    /// Return the colormode.
    ColorMode colorMode() const;

public slots:
    /// Set the contrast level in [0,1].
    void setContrast(double contrast);

    /// Set the brightness level in [0,1].
    void setBrightness(double brightness);

    /// Set the details level in [0,1].
    void setDetails(double details);

    /// Set the color mode.
    void setColorMode(ColorizeImage::ColorMode colormode);

signals:
    void contrastChanged();
    void brightnessChanged();
    void detailsChanged();
    void colorModeChanged();

protected:
    QImage transform(const QImage& image);

private:
    struct Data;
    QScopedPointer<Data> d;
};

#endif

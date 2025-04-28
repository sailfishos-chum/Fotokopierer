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

#ifndef __FOTOKOPIERER_SCANNEDIMAGE_HXX__
#define __FOTOKOPIERER_SCANNEDIMAGE_HXX__

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QVariant>

#include <QtGui/QPixmap>

class ScannedImage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString original READ originalImage NOTIFY originalChanged)
    Q_PROPERTY(QString image READ image NOTIFY imageChanged)
    Q_PROPERTY(double angle READ angle WRITE setAngle NOTIFY angleChanged)
    Q_PROPERTY(
        double contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    Q_PROPERTY(double brightness READ brightness WRITE setBrightness NOTIFY
                   brightnessChanged)
    Q_PROPERTY(
        double details READ details WRITE setDetails NOTIFY detailsChanged)

public:
    /// The color mode to be used.
    enum ColorMode {
        Gray,
        BlackAndWhite,
        Colored,
    };

public:
    ScannedImage(QObject* parent = nullptr);
    ~ScannedImage();

    /// Return the original, unscaled image.
    QString originalImage() const;

    /// Return the original, possibly scaled image.
    ///
    /// This is the base address for all image manipulations.
    QString image() const;

    /// Set the rotation angle.
    void setAngle(double angle);

    /// Return the rotation angle.
    double angle() const;

    /// Set the contrast level in [0,1].
    void setContrast(double contrast);

    /// Return the contrast level.
    double contrast() const;

    /// Set the brightness level in [0,1].
    void setBrightness(double brightness);

    /// Return the brightness level.
    double brightness() const;

    /// Set the details level in [0,1].
    void setDetails(double details);

    /// Return the details level.
    double details() const;

    /// Set the color mode.
    void setColorMode(ColorMode colormode);

    /// Return the colormode.
    ColorMode colorMode() const;

    /// Return the auto-detected cut box.
    ///
    /// The method returns a list of four points (topleft, topright,
    /// bottomright, bottomleft).
    Q_INVOKABLE QVariantList autoDetectCutRect();

    /// Set the corner points of the cut box.
    ///
    /// If the cut box is not convex return false otherwise return
    /// true.
    Q_INVOKABLE bool setCutBox(const QPointF& topleft,
                               const QPointF& topright,
                               const QPointF& bottomright,
                               const QPointF& bottomleft);

    Q_INVOKABLE void loadFile(const QString& fileName);

    /// Return the original image.
    QImage getOriginal();

    /// Return the rotated image.
    QImage getRotated();

    /// Return the rotated and cut image.
    QImage getCut();

    /// Return the rotated and colorized image.
    QImage getColorized();

signals:
    void originalChanged();
    void imageChanged();

    void angleChanged();
    void contrastChanged();
    void brightnessChanged();
    void detailsChanged();
    void colorModeChanged();

private:
    struct Data;
    QScopedPointer<Data> d;
};

#endif

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

#ifndef __FOTOKOPIERER_COLORIZEVIEW_HXX__
#define __FOTOKOPIERER_COLORIZEVIEW_HXX__

#include "ScanImageView.hxx"

class ColorizeView : public ScanImageView
{
    Q_OBJECT

    Q_PROPERTY(qreal contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    Q_PROPERTY(qreal brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(qreal threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)
    Q_PROPERTY(qreal blockSize READ blockSize WRITE setBlockSize NOTIFY blockSizeChanged)
    Q_PROPERTY(ColorMode colorMode READ colorMode WRITE setColorMode NOTIFY colorModeChanged)

public:
    /// The color mode to be used.
    enum ColorMode {
        Gray,
        BlackAndWhite,
        Colored,
        FullColor,
    };
    Q_ENUM(ColorMode)

public:
    ColorizeView(QQuickItem* parent = nullptr);

    ~ColorizeView() override;

    /// Return the contrast level.
    qreal contrast() const;

    /// Set the contrast level in [0,1].
    void setContrast(qreal contrast);

    /// Return the brightness level.
    qreal brightness() const;

    /// Set the brightness level in [0,1].
    void setBrightness(qreal brightness);

    /// Return the threshold level.
    qreal threshold() const;

    /// Set the threshold level in [0,1].
    void setThreshold(qreal threshold);

    /// Return the relative threshold block size for b/w.
    qreal blockSize() const;

    /// Set the relative threshold block size for b/w in [0,1].
    void setBlockSize(qreal blockSize);

    /// Return the colormode.
    ColorMode colorMode() const;

    /// Set the color mode.
    void setColorMode(ColorMode colormode);

    /// Apply the current colorization to the scan image.
    Q_INVOKABLE void apply();

signals:
    void contrastChanged();
    void brightnessChanged();
    void thresholdChanged();
    void blockSizeChanged();
    void colorModeChanged();
    void imageChanged();

protected:
    QImage image() const override;

protected slots:
    void onNewImage();

private slots:
    void onImageUpdated();
    void onCutImageChanged();

private:
    void updateView();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

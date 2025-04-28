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

#ifndef __FOTOKOPIERER_COLORIZEFILTER_HXX__
#define __FOTOKOPIERER_COLORIZEFILTER_HXX__

#include "Filter.hxx"

class ColorizeFilter : public Filter
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
        FullColor,
    };
    Q_ENUM(ColorMode)

public:
    explicit ColorizeFilter(Scanner* image);

    ColorizeFilter(Scanner* image, Filter* previous_filter);

    ColorizeFilter(const ColorizeFilter&) = delete;
    ColorizeFilter(ColorizeFilter&&) = delete;
    ColorizeFilter& operator=(const ColorizeFilter&) = delete;
    ColorizeFilter& operator=(ColorizeFilter&&) = delete;

    ~ColorizeFilter() override;

    QJsonObject saveJson() const override;

    void loadJson(const QJsonObject& object) override;

    QImage apply(QImage&& image) override;

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
    void setColorMode(ColorizeFilter::ColorMode colormode);

signals:
    void contrastChanged();
    void brightnessChanged();
    void detailsChanged();
    void colorModeChanged();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

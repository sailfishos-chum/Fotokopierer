/*
 * Copyright (c) 2018, 2019, 2021 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __FOTOKOPIERER_SCANIMAGE_HXX__
#define __FOTOKOPIERER_SCANIMAGE_HXX__

#include <QtCore/QObject>

#include <memory>
#include <opencv2/core.hpp>

#include "ColorizeView.hxx"

class Document;
class Page;

/// A scanned image.
///
/// This is basically the model representing a scanned image in
/// memory. It should be show by one of the corresponding views that
/// allow to modify some of the properties.
class ScanImage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)

    Q_PROPERTY(QPointF topLeft READ topLeft WRITE setTopLeft NOTIFY topLeftChanged)
    Q_PROPERTY(QPointF topRight READ topRight WRITE setTopRight NOTIFY topRightChanged)
    Q_PROPERTY(QPointF bottomRight READ bottomRight WRITE setBottomRight NOTIFY bottomRightChanged)
    Q_PROPERTY(QPointF bottomLeft READ bottomLeft WRITE setBottomLeft NOTIFY bottomLeftChanged)

    Q_PROPERTY(double contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    Q_PROPERTY(double brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(double threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)
    Q_PROPERTY(double blockSize READ blockSize WRITE setBlockSize NOTIFY blockSizeChanged)
    Q_PROPERTY(ColorMode colorMode READ colorMode WRITE setColorMode NOTIFY colorModeChanged)

public:
    /// The color mode to be used.
    using ColorMode = ColorizeView::ColorMode;

    struct Parameters {
        double contrast = 0.5;    ///< Contrast in [0,1]
        double brightness = 0.5;  ///< Brightness in [0,1]

        double threshold_c = 0.8;  ///< Value between [0,1] mapped to [-15,15]
        double blocksize = 0.1;    ///< Relative block size
    };

public:
    ScanImage(const QImage& original, QObject* parent = nullptr);

    ScanImage(const ScanImage&) = delete;
    ScanImage(ScanImage&&) = delete;
    ScanImage& operator=(const ScanImage&) = delete;
    ScanImage& operator=(ScanImage&&) = delete;

    ~ScanImage() override;

    /// Return the original image.
    cv::Mat original() const;

    /// Return the cut image.
    ///
    /// If the image has changed in the meantime it could be recomputed
    /// and a `cutImageChanged` signal will be emitted later.
    ///
    /// If `wait` is `true` the function will not return before the
    /// image is ready.
    cv::Mat cutImage(bool wait = false) const;

    /// Return the colorized image.
    ///
    /// If the image has changed in the meantime it could be recomputed
    /// and a `colorizedImageChanged` signal will be emitted later.
    ///
    /// If `wait` is `true` the function will not return before the
    /// image is ready.
    cv::Mat colorizedImage(bool wait = false) const;

    /// Return the orientation/rotation of the image.
    int orientation() const;

    /// Change the orientation of the image.
    void setOrientation(int orientation);

    QPointF topLeft() const;

    void setTopLeft(QPointF topleft);

    QPointF topRight() const;

    void setTopRight(QPointF topright);

    QPointF bottomRight() const;

    void setBottomRight(QPointF bottomright);

    QPointF bottomLeft() const;

    void setBottomLeft(QPointF bottomleft);

    /// Return the contrast level.
    double contrast() const;

    /// Set the contrast level in [0,1].
    void setContrast(double contrast);

    /// Return the brightness level.
    double brightness() const;

    /// Set the brightness level in [0,1].
    void setBrightness(double brightness);

    /// Return the black/white threshold level.
    double threshold() const;

    /// Set the threshold level in [0,1].
    void setThreshold(double threshold);

    /// Return the blocksize for b/w thresholding.
    double blockSize() const;

    /// Set the blocksize for b/w thresholding in [0,1] (relative size).
    void setBlockSize(double blockSize);

    /// Return all parameters.
    Parameters parameters() const;

    /// Set all parameters at once.
    void setParameters(Parameters params);

    /// Return the colormode.
    ColorMode colorMode() const;

    /// Set the color mode.
    void setColorMode(ColorMode colormode);

    /// Apply image cut.
    void applyCut();

    /// Apply image colorization.
    void applyColorize();

    /// Return all filter settings as a JSON object.
    QJsonObject saveJson() const;

    /// Load all filter settings from a JSON object.
    void loadJson(const QJsonObject& settings);

signals:
    void orientationChanged();

    void topLeftChanged();
    void topRightChanged();
    void bottomRightChanged();
    void bottomLeftChanged();

    void contrastChanged();
    void brightnessChanged();

    void thresholdChanged();
    void blockSizeChanged();

    void colorModeChanged();

    void startCutImageUpdate() const;
    void finishCutImageUpdate();
    void cutImageChanged();

    void startColorizedImageUpdate() const;
    void finishColorizedImageUpdate();
    void colorizedImageChanged();

private slots:
    void onCutReady();
    void onColorizedReady();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

cv::Mat computeColorizedImage(const cv::Mat& image, ScanImage::Parameters params, ScanImage::ColorMode colorMode, cv::Mat* hsv = nullptr, cv::Mat* mask = nullptr);

#endif

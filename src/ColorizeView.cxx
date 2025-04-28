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

#include "ColorizeView.hxx"

#include "ColorizeChooser.hxx"
#include "Convert.hxx"
#include "ScanImage.hxx"
#include "Scanner.hxx"

#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFutureWatcher>
#include <QtGui/QImage>

#include <opencv2/imgproc.hpp>

struct ColorizeView::Data {
    ScanImage::Parameters params;
    ColorMode colorMode = ColorMode::FullColor;
    ColorizeChooser* colorizeChooser = nullptr;

    std::shared_ptr<ScanImage> scanImage = nullptr;
    cv::Mat scaled;
    cv::Mat hsv, mask;

    QFutureWatcher<cv::Mat> image = QFutureWatcher<cv::Mat>();
    bool hasImage = false;
    bool need_restart = false;

    QMetaObject::Connection cutImageChangedConnection = {};
    QMetaObject::Connection startCutImageUpdateConnection = {};
};

ColorizeView::ColorizeView(QQuickItem* parent)
    : ScanImageView(parent), d(new Data)
{
    connect(&d->image, &QFutureWatcher<QImage>::finished, this, &ColorizeView::onImageUpdated);
}

ColorizeView::~ColorizeView() = default;

qreal ColorizeView::contrast() const
{
    return d->params.contrast;
}

void ColorizeView::setContrast(qreal contrast)
{
    contrast = qBound(static_cast<qreal>(0.0), contrast, static_cast<qreal>(1.0));
    if (contrast != d->params.contrast) {
        d->params.contrast = contrast;
        emit contrastChanged();
        updateView();
    }
}

qreal ColorizeView::brightness() const
{
    return d->params.brightness;
}

void ColorizeView::setBrightness(qreal brightness)
{
    brightness = qBound(static_cast<qreal>(0.0), brightness, static_cast<qreal>(1.0));
    if (brightness != d->params.brightness) {
        d->params.brightness = brightness;
        emit brightnessChanged();
        updateView();
    }
}

qreal ColorizeView::threshold() const
{
    return d->params.threshold_c;
}

void ColorizeView::setThreshold(qreal threshold)
{
    threshold = qBound(static_cast<qreal>(0.0), threshold, static_cast<qreal>(1.0));
    if (threshold != d->params.threshold_c) {
        d->params.threshold_c = threshold;
        emit thresholdChanged();
        updateView();
    }
}

qreal ColorizeView::blockSize() const
{
    return d->params.blocksize;
}

void ColorizeView::setBlockSize(qreal blockSize)
{
    blockSize = qBound(static_cast<qreal>(0.0), blockSize, static_cast<qreal>(1.0));
    if (blockSize != d->params.blocksize) {
        d->params.blocksize = blockSize;
        emit blockSizeChanged();
        updateView();
    }
}

ColorizeView::ColorMode ColorizeView::colorMode() const
{
    return d->colorMode;
}

void ColorizeView::setColorMode(ColorMode colormode)
{
    if (colormode != d->colorMode) {
        d->colorMode = colormode;
        emit colorModeChanged();
        updateView();
    }
}

ColorizeChooser* ColorizeView::colorizeChooser() const
{
    return d->colorizeChooser;
}

void ColorizeView::setColorizeChooser(ColorizeChooser* colorizeChooser)
{
    if (colorizeChooser != d->colorizeChooser) {
        d->colorizeChooser = colorizeChooser;
        emit colorizeChooserChanged();
        d->hasImage = false;
        updateView();
    }
}

void ColorizeView::updateView()
{
    if (!d->scaled.empty()) {
        d->hasImage = false;
        if (d->image.isRunning()) {
            d->need_restart = true;
        } else {
            d->image.setFuture(QtConcurrent::run([this]() {
                return computeColorizedImage(d->scaled, d->params, d->colorMode, &d->hsv, &d->mask);
            }));
        }
    }
}

void ColorizeView::onNewImage()
{
    disconnect(d->cutImageChangedConnection);
    d->cutImageChangedConnection = {};
    d->startCutImageUpdateConnection = {};

    if (auto s = scanner(); s != nullptr) {
        d->scanImage = s->currentImage();
        if (d->scanImage != nullptr) {
            d->need_restart = false;
            d->cutImageChangedConnection = connect(d->scanImage.get(), &ScanImage::cutImageChanged, this, &ColorizeView::onCutImageChanged);
            d->startCutImageUpdateConnection = connect(d->scanImage.get(), &ScanImage::startCutImageUpdate, [this]() { setBusy(true); });

            // initialize settings

            d->params = d->scanImage->parameters();
            setColorMode(d->scanImage->colorMode());
        }
    }

    updateView();
}

void ColorizeView::onImageUpdated()
{
    d->hasImage = true;
    if (d->need_restart) {
        d->need_restart = false;
        updateView();
    } else {
        setBusy(false);
        if (!d->hsv.empty() && !d->mask.empty() && d->colorizeChooser != nullptr) {
            d->colorizeChooser->updateImage(d->hsv, d->mask);
        }
        update();
    }
}

QImage ColorizeView::image() const
{
    if (d->image.isFinished() && d->hasImage) {
        return cvMatToQImage(d->image.result()).copy();
    } else {
        return {};
    };
}

cv::Mat ColorizeView::cutImage() const
{
    return d->scanImage != nullptr ? d->scanImage->cutImage() : cv::Mat();
}

void ColorizeView::onCutImageChanged()
{
    if (d->scanImage != nullptr) {
        auto image = d->scanImage->cutImage();

        if (!image.empty()) {
            auto factor = 1000.0 / std::max(image.cols, image.rows);
            cv::resize(image, d->scaled, cv::Size(), factor, factor);
        }

        emit cutImageChanged();
    }

    updateView();
}

void ColorizeView::apply()
{
    if (d->scanImage != nullptr) {
        d->scanImage->setParameters(d->params);
        d->scanImage->setColorMode(d->colorMode);
        d->scanImage->applyColorize();
    }
}

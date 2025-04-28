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

#include "ScanImage.hxx"
#include "Scanner.hxx"

#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFutureWatcher>
#include <QtGui/QImage>

struct ColorizeView::Data {
    qreal contrast = 0.5;
    qreal brightness = 0.5;
    qreal details = 0.5;
    ColorMode colorMode = ColorMode::FullColor;

    std::shared_ptr<ScanImage> scanImage = nullptr;
    QImage scaled;

    QFutureWatcher<QImage> image = QFutureWatcher<QImage>();
    bool hasImage = false;

    QMetaObject::Connection cutImageChangedConnection = {};
};

ColorizeView::ColorizeView(QQuickItem* parent)
    : ScanImageView(parent), d(new Data)
{
    connect(&d->image, &QFutureWatcher<QImage>::finished, this, [this]() {
        d->hasImage = true;
        update();
    });
}

ColorizeView::~ColorizeView() = default;

qreal ColorizeView::contrast() const
{
    return d->contrast;
}

void ColorizeView::setContrast(qreal contrast)
{
    contrast = qBound(static_cast<qreal>(0.0), contrast, static_cast<qreal>(1.0));
    if (contrast != d->contrast) {
        d->contrast = contrast;
        emit contrastChanged();
        updateView();
    }
}

qreal ColorizeView::brightness() const
{
    return d->brightness;
}

void ColorizeView::setBrightness(qreal brightness)
{
    brightness = qBound(static_cast<qreal>(0.0), brightness, static_cast<qreal>(1.0));
    if (brightness != d->brightness) {
        d->brightness = brightness;
        emit brightnessChanged();
        updateView();
    }
}

qreal ColorizeView::details() const
{
    return d->details;
}

void ColorizeView::setDetails(qreal details)
{
    details = qBound(static_cast<qreal>(0.0), details, static_cast<qreal>(1.0));
    if (details != d->details) {
        d->details = details;
        emit detailsChanged();
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

void ColorizeView::updateView()
{
    if (!d->scaled.isNull()) {
        d->hasImage = false;
        d->image.setFuture(QtConcurrent::run([this]() {
            return computeColorizedImage(d->scaled, d->contrast, d->brightness, d->details, d->colorMode);
        }));
    }
}

void ColorizeView::onNewImage()
{
    disconnect(d->cutImageChangedConnection);
    if (auto s = scanner(); s != nullptr) {
        d->scanImage = s->currentImage();
        if (d->scanImage != nullptr) {
            d->cutImageChangedConnection = connect(d->scanImage.get(), &ScanImage::cutImageChanged, this, &ColorizeView::onCutImageChanged);
            return;
        }
    }

    d->cutImageChangedConnection = {};
    updateView();
}

QImage ColorizeView::image() const
{
    if (d->image.isFinished() && d->hasImage) {
        return d->image.result();
    } else {
        return {};
    };
}

void ColorizeView::onCutImageChanged()
{
    auto image = d->scanImage->cutImage();

    if (image.width() > image.height()) {
        d->scaled = image.scaledToWidth(qMin(image.width(), 1000));
    } else {
        d->scaled = image.scaledToHeight(qMin(image.height(), 1000));
    }

    updateView();
}

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

#include "ScanImage.hxx"

#include "Convert.hxx"
#include "Document.hxx"
#include "Page.hxx"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFile>
#include <QtCore/QFutureWatcher>
#include <QtCore/QVector>
#include <QtGui/QImage>
#include <QtGui/QImageReader>

#include <opencv2/imgproc/imgproc.hpp>

#include <cassert>

static QPointF toPoint(const QJsonValue& value)
{
    auto p = value.toObject();
    return {
        static_cast<qreal>(p[QStringLiteral("x")].toDouble(0)),
        static_cast<qreal>(p[QStringLiteral("y")].toDouble(0)),
    };
}

static QJsonValue fromPoint(QPointF p)
{
    return QJsonObject{{QStringLiteral("x"), p.x()}, {QStringLiteral("y"), p.y()}};
}

struct ScanImage::Data {
    QImage original;

    QFutureWatcher<QImage> rotated = QFutureWatcher<QImage>();
    QFutureWatcher<QImage> cut = QFutureWatcher<QImage>();
    QFutureWatcher<QImage> colorized = QFutureWatcher<QImage>();

    bool rotatedReady = false;
    bool cutReady = false;
    bool colorizedReady = false;

    int orientation = 0;

    QPointF topLeft = {};
    QPointF topRight = {};
    QPointF bottomRight = {};
    QPointF bottomLeft = {};

    double contrast = 0.5;
    double brightness = 0.5;
    double details = 0.5;
    ColorMode colorMode = ColorMode::FullColor;

    QImage computeCutImage(const QImage& image) const;
    double getAspectRatio(QPointF tl, QPointF tr, QPointF br, QPointF bl) const;

    QImage computeColorizedImage(const QImage& image) const;
};

ScanImage::ScanImage(const QImage& original, QObject* parent)
    : QObject(parent), d(new Data{original})
{
    connect(&d->rotated, &QFutureWatcher<QImage>::finished, this, &ScanImage::onRotatedReady);
    connect(&d->cut, &QFutureWatcher<QImage>::finished, this, &ScanImage::onCutReady);
    connect(&d->colorized, &QFutureWatcher<QImage>::finished, this, &ScanImage::onColorizedReady);
}

ScanImage::~ScanImage() = default;

QImage ScanImage::original() const
{
    return d->original;
}

QJsonObject ScanImage::saveJson() const
{
    QJsonObject settings;

    settings[QStringLiteral("rotate")] = QJsonObject{{QStringLiteral("orientation"), d->orientation}};

    settings[QStringLiteral("cut")] = QJsonObject{
        {QStringLiteral("topleft"), fromPoint(d->topLeft)},
        {QStringLiteral("topright"), fromPoint(d->topRight)},
        {QStringLiteral("bottomleft"), fromPoint(d->bottomLeft)},
        {QStringLiteral("bottomright"), fromPoint(d->bottomRight)},
    };

    settings[QStringLiteral("colorize")] = QJsonObject{
        {QStringLiteral("contrast"), d->contrast},
        {QStringLiteral("brightness"), d->brightness},
        {QStringLiteral("details"), d->details},
        {QStringLiteral("mode"), d->colorMode},
    };

    return settings;
}

void ScanImage::loadJson(const QJsonObject& settings)
{
    auto rot = settings[QStringLiteral("rotate")].toObject();
    setOrientation(rot[QStringLiteral("orientation")].toInt(0));

    auto cut = settings[QStringLiteral("cut")].toObject();
    setTopLeft(toPoint(cut[QStringLiteral("topleft")]));
    setTopRight(toPoint(cut[QStringLiteral("topright")]));
    setBottomRight(toPoint(cut[QStringLiteral("bottomright")]));
    setBottomLeft(toPoint(cut[QStringLiteral("bottomleft")]));

    auto col = settings[QStringLiteral("colorize")].toObject();
    setContrast(static_cast<qreal>(col[QStringLiteral("contrast")].toDouble(0.5)));
    setBrightness(static_cast<qreal>(col[QStringLiteral("brightness")].toDouble(0.5)));
    setDetails(static_cast<qreal>(col[QStringLiteral("details")].toDouble(0.5)));

    auto mode = settings[QStringLiteral("mode")].toInt(ColorMode::BlackAndWhite);
    switch (mode) {
        case ColorMode::BlackAndWhite:
        case ColorMode::Gray:
        case ColorMode::Colored:
        case ColorMode::FullColor:
            setColorMode(static_cast<ColorMode>(mode));
            break;
        default:
            setColorMode(ColorMode::BlackAndWhite);
            break;
    }
}

int ScanImage::orientation() const
{
    return d->orientation;
}

void ScanImage::setOrientation(int orientation)
{
    orientation %= 4;
    if (orientation != d->orientation) {
        d->orientation = orientation;
        d->rotatedReady = false;
        d->cutReady = false;
        d->colorizedReady = false;
        emit orientationChanged();
    }
}

QPointF ScanImage::topLeft() const
{
    return d->topLeft;
}

void ScanImage::setTopLeft(QPointF topLeft)
{
    if (topLeft != d->topLeft) {
        d->topLeft = topLeft;
        d->cutReady = false;
        d->colorizedReady = false;
        emit topLeftChanged();
    }
}

QPointF ScanImage::topRight() const
{
    return d->topRight;
}

void ScanImage::setTopRight(QPointF topRight)
{
    if (topRight != d->topRight) {
        d->topRight = topRight;
        d->cutReady = false;
        d->colorizedReady = false;
        emit topRightChanged();
    }
}

QPointF ScanImage::bottomRight() const
{
    return d->bottomRight;
}

void ScanImage::setBottomRight(QPointF bottomRight)
{
    if (bottomRight != d->bottomRight) {
        d->bottomRight = bottomRight;
        d->cutReady = false;
        d->colorizedReady = false;
        emit bottomRightChanged();
    }
}

QPointF ScanImage::bottomLeft() const
{
    return d->bottomLeft;
}

void ScanImage::setBottomLeft(QPointF bottomLeft)
{
    if (bottomLeft != d->bottomLeft) {
        d->bottomLeft = bottomLeft;
        d->cutReady = false;
        d->colorizedReady = false;
        emit bottomLeftChanged();
    }
}

double ScanImage::contrast() const
{
    return d->contrast;
}

void ScanImage::setContrast(double contrast)
{
    if (contrast != d->contrast) {
        d->contrast = contrast;
        d->colorizedReady = false;
        emit contrastChanged();
    }
}

double ScanImage::brightness() const
{
    return d->brightness;
}

void ScanImage::setBrightness(double brightness)
{
    if (brightness != d->brightness) {
        d->brightness = brightness;
        d->colorizedReady = false;
        emit brightnessChanged();
    }
}

double ScanImage::details() const
{
    return d->details;
}

void ScanImage::setDetails(double details)
{
    if (details != d->details) {
        d->details = details;
        d->colorizedReady = false;
        emit detailsChanged();
    }
}

ScanImage::ColorMode ScanImage::colorMode() const
{
    return d->colorMode;
}

void ScanImage::setColorMode(ColorMode colorMode)
{
    if (colorMode != d->colorMode) {
        d->colorMode = colorMode;
        d->colorizedReady = false;
        emit colorModeChanged();
    }
}

void ScanImage::applyCut()
{
    emit cutImageChanged();
}

QImage ScanImage::rotatedImage(bool wait) const
{
    if (!d->original.isNull() && !d->rotatedReady) {
        auto orientation = d->orientation;
        auto original = d->original;
        d->rotated.setFuture(QtConcurrent::run([orientation, original]() {
            QTransform transform;
            transform.rotate(orientation * 90.0);
            return original.transformed(transform);
        }));

        if (wait) d->rotated.waitForFinished();
    }

    if (d->rotatedReady) {
        return d->rotated.result();
    } else {
        return {};
    }
}

void ScanImage::onRotatedReady()
{
    d->rotatedReady = true;
    emit rotatedImageChanged();
}

QImage ScanImage::cutImage(bool wait) const
{
    if (!d->original.isNull() && !d->cutReady) {
        d->cut.setFuture(QtConcurrent::run([this]() {
            return d->computeCutImage(rotatedImage(true));
        }));

        if (wait) d->cut.waitForFinished();
    }

    if (d->cutReady) {
        return d->cut.result();
    } else {
        return {};
    }
}

void ScanImage::onCutReady()
{
    d->cutReady = true;
    emit cutImageChanged();
}

QImage ScanImage::colorizedImage(bool wait) const
{
    if (!d->original.isNull() && !d->colorizedReady) {
        d->colorized.setFuture(QtConcurrent::run([this]() {
            return d->computeColorizedImage(cutImage(true));
        }));

        if (wait) d->colorized.waitForFinished();
    }

    if (d->colorizedReady) {
        return d->colorized.result();
    } else {
        return {};
    }
}

void ScanImage::onColorizedReady()
{
    d->colorizedReady = true;
    emit colorizedImageChanged();
}

QImage ScanImage::Data::computeCutImage(const QImage& image) const
{
    qDebug() << "Compute cut image";

    auto rotated = QImageToCvMat(image);

    auto w = static_cast<float>(rotated.cols);
    auto h = static_cast<float>(rotated.rows);

    cv::Point2f tl(topLeft.x() * w, topLeft.y() * h);
    cv::Point2f tr(topRight.x() * w, topRight.y() * h);
    cv::Point2f br(bottomRight.x() * w, bottomRight.y() * h);
    cv::Point2f bl(bottomLeft.x() * w, bottomLeft.y() * h);

    auto ratio = getAspectRatio(QPointF{tl.x - w / 2, tl.y - h / 2} / 100,
                                QPointF{tr.x - w / 2, tr.y - h / 2} / 100,
                                QPointF{br.x - w / 2, br.y - h / 2} / 100,
                                QPointF{bl.x - w / 2, bl.y - h / 2} / 100);

    if (qIsNaN(ratio)) {
        return {};
    }

    auto height = static_cast<float>(std::max(cv::norm(tr - tl), cv::norm(br - bl)));
    auto width = static_cast<float>(height * ratio);

    cv::Point2f src[4] = {tl, tr, br, bl};
    cv::Point2f dst[4] = {{0, 0}, {width - 1, 0}, {width - 1, height - 1}, {0, height - 1}};

    auto M = cv::getPerspectiveTransform(src, dst);
    cv::Mat cut;
    cv::warpPerspective(rotated, cut, M, {(int)width, (int)height});

    return cvMatToQImage(cut).copy();
}

/// Return the aspect ratio of the original image.
///
/// The aspect ratio is guessed from the four projection points
/// \f$(x_{i}, y_{i})\f$ for $i \in \{tl,tr,br,bl\}\f$ as follows.
///
/// Given the four points on the z=1 plane and the camera at (0,0,0),
/// the original points are at \f$ \alpha_{i} p_{i} = \alpha_i (x_i,
/// y_i, 1) \f$ and so on. As the four points form a parallelogram the
/// satisfy the three equations \f$ \alpha_{br} p_{br} = \alpha_{tl}
/// p_{bl} + \alpha_{tr} p_{tr} - \alpha_{tl} p_{tl} \f$. Solving
/// these equations give a linear representation of the four points
/// \f$ p_i = \beta q_i \f$ for some fixed points \f$ q_i \f$. The
/// aspect ratio can then be computed as \f$ \frac{\|q_{tr} -
/// q_{tl}\|}{\|q_{bl} - q_{tl}\|} \f$, which is the value returned by
/// this function.
double ScanImage::Data::getAspectRatio(QPointF tl, QPointF tr, QPointF br, QPointF bl) const
{
    double a_tl = (br.x() - tl.x());
    double a_tr = (tr.x() - br.x());
    double a_bl = (bl.x() - br.x());

    double b_tl = (br.y() - tl.y());
    double b_tr = (tr.y() - br.y());
    double b_bl = (bl.y() - br.y());

    // pivot, maybe swap equations
    if (std::abs(a_tl) < std::abs(b_tl)) {
        std::swap(a_tl, b_tl);
        std::swap(a_tr, b_tr);
        std::swap(a_bl, b_bl);
    }

    double c_bl = b_bl - b_tl / a_tl * a_bl;
    double c_tr = b_tr - b_tl / a_tl * a_tr;

    double d_tr = a_tl * c_bl;
    double d_bl = -c_tr * a_tl;
    double d_tl = c_tr * a_bl - a_tr * c_bl;
    // double d_br = -c_tr * a_tl + a_tl * c_bl - c_tr * a_bl + a_tr * c_bl;

    double norm_horiz = (std::pow(d_tr * tr.x() - d_tl * tl.x(), 2) +
                         std::pow(d_tr * tr.y() - d_tl * tl.y(), 2) + std::pow(d_tr - d_tl, 2));

    double norm_vert = (std::pow(d_bl * bl.x() - d_tl * tl.x(), 2) +
                        std::pow(d_bl * bl.y() - d_tl * tl.y(), 2) + std::pow(d_bl - d_tl, 2));

    return std::sqrt(norm_horiz / norm_vert);
}

QImage ScanImage::Data::computeColorizedImage(const QImage& image) const
{
    return ::computeColorizedImage(image, contrast, brightness, details, colorMode);
}

QImage computeColorizedImage(const QImage& image, qreal contrast_, qreal brightness_, qreal details_, ScanImage::ColorMode colorMode)
{
    auto img_cut = QImageToCvMat(image, false);

    // Apply contrast and brightness transform.
    auto contrast = std::pow(4.0, contrast_ * 2 - 1);
    auto brightness = 128 - contrast * 128 + (2 * brightness_ - 1) * 128;
    cv::Mat img_bright;
    img_cut.convertTo(img_bright, -1, contrast, brightness);
    img_cut.release();

    if (colorMode == ColorizeView::FullColor) {
        return cvMatToQImage(img_bright).copy();
    }

    // Compute a gray-scale image.
    cv::Mat img_gray;
    cv::cvtColor(img_bright, img_gray, cv::COLOR_BGR2GRAY);

    if (colorMode == ColorizeView::Gray) {
        return cvMatToQImage(img_gray).copy();
    }

    // Threshold filter for background mask.
    cv::Mat bg_mask;
    {
        int details = std::max(details_ * 50, static_cast<qreal>(3));
        if (details % 2 == 0) {
            details += 1;
        }

        cv::blur(img_gray, bg_mask, {3, 3});

        cv::adaptiveThreshold(
            bg_mask, bg_mask, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, details, 5);
    }
    img_gray.release();

    if (colorMode == ColorizeView::BlackAndWhite) {
        return cvMatToQImage(bg_mask).copy();
    }

    // Set background to white
    cv::Mat img_col;
    img_bright.convertTo(img_col, CV_8U);
    img_bright.release();

    // Set background to white
    img_col.setTo(cv::Scalar(255, 255, 255), bg_mask);

    // Convert to HSV for color filtering
    cv::Mat img_hsv, img_result, img_this_color;
    cv::cvtColor(img_col, img_hsv, cv::COLOR_BGR2HSV);
    cv::cvtColor(img_col, img_result, cv::COLOR_BGR2HSV);

    // Colorize everything non-white to black
    img_result.setTo(cv::Scalar(0, 0, 0), ~bg_mask);

    // colorize by hue
    for (int i = 15; i < 180; i += 30) {
        cv::inRange(img_hsv, cv::Scalar(std::max(i, 15) - 15, 50, 50), cv::Scalar(i + 15, 255, 255), img_this_color);
        img_result.setTo(cv::Scalar(i, 255, 255), img_this_color);
        if (i < 15) {
            cv::inRange(img_hsv, cv::Scalar(180 - (15 - i), 50, 50), cv::Scalar(180, 255, 255), img_this_color);
            img_result.setTo(cv::Scalar(i, 255, 255), img_this_color);
        }
    }

    // convert result back to BGR
    cv::cvtColor(img_result, img_col, cv::COLOR_HSV2BGR);

    // and to QImage
    cv::Mat colorized;
    img_col.convertTo(colorized, CV_8U);
    img_col.release();
    return cvMatToQImage(colorized).copy();
}

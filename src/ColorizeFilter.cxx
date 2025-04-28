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

#include "ColorizeFilter.hxx"

#include "Convert.hxx"

#include <QtCore/QJsonObject>
#include <QtGui/QImage>

#include <opencv2/imgproc/imgproc.hpp>

struct ColorizeFilter::Data {
    double contrast = 0.5;
    double brightness = 0.5;
    double details = 0.5;
    ColorMode colormode = ColorMode::BlackAndWhite;
};

ColorizeFilter::ColorizeFilter(Scanner* image)
    : ColorizeFilter(image, nullptr) {}

ColorizeFilter::ColorizeFilter(Scanner* image, Filter* previous_filter)
    : Filter(image, previous_filter), d(new Data)
{
    connect(this, &ColorizeFilter::contrastChanged, this, &Filter::filterChanged);
    connect(this, &ColorizeFilter::brightnessChanged, this, &Filter::filterChanged);
    connect(this, &ColorizeFilter::detailsChanged, this, &Filter::filterChanged);
    connect(this, &ColorizeFilter::colorModeChanged, this, &Filter::filterChanged);
}

ColorizeFilter::~ColorizeFilter() = default;

void ColorizeFilter::setContrast(double contrast)
{
    contrast = std::max(0.0, std::min(1.0, contrast));
    if (contrast != d->contrast) {
        d->contrast = contrast;
        emit contrastChanged();
    }
}

double ColorizeFilter::contrast() const
{
    return d->contrast;
}

void ColorizeFilter::setBrightness(double brightness)
{
    brightness = std::max(0.0, std::min(1.0, brightness));
    if (brightness != d->brightness) {
        d->brightness = brightness;
        emit brightnessChanged();
    }
}

double ColorizeFilter::brightness() const
{
    return d->brightness;
}

void ColorizeFilter::setDetails(double details)
{
    details = std::max(0.0, std::min(1.0, details));
    if (details != d->details) {
        d->details = details;
        emit detailsChanged();
    }
}

double ColorizeFilter::details() const
{
    return d->details;
}

void ColorizeFilter::setColorMode(ColorMode colorMode)
{
    if (colorMode != d->colormode) {
        d->colormode = colorMode;
        emit colorModeChanged();
    }
}

ColorizeFilter::ColorMode ColorizeFilter::colorMode() const
{
    return d->colormode;
}

void ColorizeFilter::reset()
{
    setContrast(0.5);
    setBrightness(0.5);
    setDetails(0.5);
    setColorMode(ColorMode::BlackAndWhite);
}

QString ColorizeFilter::name() const
{
    return QStringLiteral("colorize");
}

QJsonObject ColorizeFilter::saveJson() const
{
    return {};
}

void ColorizeFilter::loadJson(const QJsonObject& object) {}

QImage ColorizeFilter::apply(QImage&& image)
{
    if (image.isNull()) {
        return image;
    }

    auto img_cut = QImageToCvMat(image, false);

    // Apply contrast and brightness transform.
    auto contrast = std::pow(4.0, d->contrast * 2 - 1);
    auto brightness = 128 - contrast * 128 + (2 * d->brightness - 1) * 128;
    cv::Mat img_bright;
    img_cut.convertTo(img_bright, -1, contrast, brightness);
    img_cut.release();

    if (d->colormode == FullColor) {
        return cvMatToQImage(img_bright).copy();
    }

    // Compute a gray-scale image.
    cv::Mat img_gray;
    cv::cvtColor(img_bright, img_gray, cv::COLOR_BGR2GRAY);

    if (d->colormode == Gray) {
        return cvMatToQImage(img_gray).copy();
    }

    // Threshold filter for background mask.
    cv::Mat bg_mask;
    {
        int details = std::max(d->details * 50, 3.0);
        if (details % 2 == 0) {
            details += 1;
        }

        cv::adaptiveThreshold(
            img_gray, bg_mask, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, details, 5);
    }
    img_gray.release();

    if (d->colormode == BlackAndWhite) {
        return cvMatToQImage(bg_mask).copy();
    }

    /// Set background to white
    cv::Mat img_col;
    img_bright.convertTo(img_col, CV_32F);
    img_bright.release();
    img_col.setTo(cv::Scalar(255, 255, 255), bg_mask);

    std::vector<cv::Point3f> points;
    {
        auto itimg = img_col.begin<cv::Point3f>();
        auto itimgend = img_col.end<cv::Point3f>();
        auto itmask = bg_mask.begin<uchar>();
        for (; itimg != itimgend; ++itimg, ++itmask) {
            if (*itmask == 0) {
                points.push_back(*itimg);
            }
        }
    }

    if (!points.empty()) {
        std::vector<int> labels;
        cv::Mat centers;
        cv::kmeans(points,
                   8,
                   labels,
                   cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0),
                   3,
                   cv::KMEANS_PP_CENTERS,
                   centers);
        points.clear();

        // stretch colors
        auto min = *std::min_element(centers.begin<float>(), centers.end<float>());
        auto max = *std::max_element(centers.begin<float>(), centers.end<float>());

        centers = 255 * (centers - min) / (max - min);
        cv::Mat ucenters;
        centers.convertTo(ucenters, CV_8U);
        centers.release();

        auto itimg = img_col.begin<cv::Vec3b>();
        auto itimgend = img_col.end<cv::Vec3b>();
        auto itmask = bg_mask.begin<uchar>();
        auto itpnts = labels.begin();
        for (; itimg != itimgend; ++itimg, ++itmask) {
            if (*itmask == 0) {
                *itimg = ucenters.row(*itpnts);
                ++itpnts;
            }
        }
    }

    cv::Mat colorized;
    img_col.convertTo(colorized, CV_8U);
    img_col.release();
    return cvMatToQImage(colorized).copy();
}

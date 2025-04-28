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

#include "ColorizeChooser.hxx"

#include <QDebug>
#include <QtGui/QPainter>

#include <algorithm>
#include <cmath>

#include <opencv2/imgproc.hpp>

#include "fifr/util/Range.hxx"

using namespace fifr::util;

struct ColorizeChooser::Data {
    int lattice = 100;
    cv::Mat hist = {};
    double minHist = 0;
    double maxHist = 0;

    std::vector<qreal> angles = {30, 90, 150, 210, 270, 330};
    int blackLevel = 50;
};

ColorizeChooser::ColorizeChooser(QQuickItem* parent)
    : QQuickPaintedItem(parent), d(new Data)
{
}

ColorizeChooser::~ColorizeChooser() = default;

void ColorizeChooser::setLattice(int lattice)
{
    lattice = qBound(1, lattice, 100);
    if (lattice != d->lattice) {
        d->lattice = lattice;
        emit latticeChanged();
    }
}

int ColorizeChooser::lattice() const
{
    return d->lattice;
}

void ColorizeChooser::setBlackLevel(int blackLevel)
{
    blackLevel = qBound(0, blackLevel, 255);
    if (blackLevel != d->blackLevel) {
        d->blackLevel = blackLevel;
        emit blackLevelChanged();
        update();
    }
}

int ColorizeChooser::blackLevel() const
{
    return d->blackLevel;
}

void ColorizeChooser::updateImage(const cv::Mat& image, const cv::Mat& mask)
{
    if (image.empty()) return;

    int channels[] = {0, 1};
    int histSize[] = {d->lattice, d->lattice};
    float hranges[] = {0, 180};
    float sranges[] = {0, 256};
    const float* ranges[] = {hranges, sranges};
    cv::calcHist(&image, 1, channels, mask, d->hist, 2, histSize, ranges, true, false);
    cv::normalize(d->hist, d->hist, 0, 255, cv::NORM_MINMAX);

    cv::minMaxLoc(d->hist, &d->minHist, &d->maxHist);
    update();
}

void ColorizeChooser::paint(QPainter* painter)
{
    auto size = std::min(width(), height());

    QConicalGradient gradient(width() / 2, height() / 2, 0);

    for (auto deg : range(360)) {
        gradient.setColorAt(deg / 360.0, QColor::fromHsvF(deg / 360.0, 0.75, 1));
    }

    painter->setBrush(gradient);
    painter->drawEllipse(QPointF{width() / 2, height() / 2}, size / 2, size / 2);

    painter->setBrush(Qt::black);

    int maxradius = 0;
    for (auto i : range(d->hist.rows)) {
        for (auto j : range(d->hist.cols)) {
            if (d->hist.at<float>(i, j) < 10) continue;
            maxradius = std::max(j, maxradius);
        }
    }

    int cur_segment = 0;
    int cur_angle = (d->angles.front() + d->angles.back()) / 2 - 180;
    if (cur_angle < 0) cur_angle += 360;
    auto cur_color = QColor::fromHsvF(cur_angle / 360.0, 1, 1);

    for (auto i : range(d->hist.rows)) {
        auto deg = static_cast<double>(i) / d->hist.rows * 360;

        if (cur_segment < d->angles.size() && deg >= d->angles[cur_segment]) {
            if (cur_segment + 1 < d->angles.size()) {
                cur_angle = (d->angles[cur_segment] + d->angles[cur_segment + 1]) / 2;
            } else {
                cur_angle = (d->angles[cur_segment] + d->angles[0] + 360) / 2;
            }
            if (cur_angle < 0) cur_angle += 360;
            cur_segment += 1;
            cur_color = QColor::fromHsvF(cur_angle / 360.0, 1, 1);
        }

        auto black = (d->blackLevel * d->lattice + 255) / 256;
        painter->setBrush(Qt::black);

        for (auto j : range(maxradius)) {
            if (j == black) painter->setBrush(cur_color);
            if (d->hist.at<float>(i, j) < 1) continue;
            auto r = size / d->lattice / 2;

            auto angle = deg / 180 * M_PI;  // angle in radiant
            auto radius = static_cast<double>(j) / maxradius * size / 2;
            auto x = std::cos(angle) * radius + width() / 2;
            auto y = -std::sin(angle) * radius + height() / 2;
            painter->drawEllipse(QPointF{x, y}, r, r);
        }
    }

    painter->setPen(Qt::black);
    painter->setBrush({});
    auto r = d->blackLevel * d->lattice * size / (maxradius * 512);
    painter->drawEllipse(QPointF{width() / 2, height() / 2}, r, r);
    for (auto angle : d->angles) {
        auto x = std::cos(angle / 180.0 * M_PI) * size / 2 + width() / 2;
        auto y = -std::sin(angle / 180.0 * M_PI) * size / 2 + height() / 2;
        painter->drawLine(width() / 2, height() / 2, x, y);
    }
}

void ColorizeChooser::setColorAngle(int which, qreal angle)
{
    which %= d->angles.size();
    if (which < 0) which += d->angles.size();
    if (d->angles[which] != angle) {
        d->angles[which] = angle;
        std::sort(d->angles.begin(), d->angles.end());
        emit colorAnglesChanged();
    }
}

qreal ColorizeChooser::colorAngle(int which) const
{
    which %= d->angles.size();
    if (which < 0) which += d->angles.size();
    return d->angles[which];
}

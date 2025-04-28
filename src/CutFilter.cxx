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

#include "CutFilter.hxx"

#include "Convert.hxx"
#include "Fotokopierer.hxx"
#include "Scanner.hxx"

#include <QtCore/QJsonObject>
#include <QtCore/QVariant>
#include <QtGui/QImage>

#include <opencv2/imgproc/imgproc.hpp>

#include <cmath>

struct CutFilter::Data {
    QPointF topleft;
    QPointF topright;
    QPointF bottomleft;
    QPointF bottomright;

    double getAspectRatio(QPointF tl, QPointF tr, QPointF br, QPointF bl);
};

CutFilter::CutFilter(Scanner* image)
    : CutFilter(image, nullptr) {}

CutFilter::CutFilter(Scanner* image, Filter* previous_filter)
    : Filter(image, previous_filter), d(new Data)
{
}

CutFilter::~CutFilter() = default;

bool CutFilter::setCutBox(QPointF topleft,
                          QPointF topright,
                          QPointF bottomright,
                          QPointF bottomleft)
{
    static Fotokopierer util;

    if (!util.isConvex(topleft, topright, bottomright, bottomleft)) {
        return false;
    }
    d->topleft = topleft;
    d->topright = topright;
    d->bottomright = bottomright;
    d->bottomleft = bottomleft;
    emit filterChanged();
    return true;
}

QVariantList CutFilter::autoDetectCutRect()
{
    QImage img =
        previous_filter_ != nullptr ? previous_filter_->filteredImage() : image()->originalImage();
    auto img_cut = QImageToCvMat(img, false);
    auto width = img_cut.cols;
    auto height = img_cut.rows;

    cv::Mat img_gray;
    cv::cvtColor(img_cut, img_gray, cv::COLOR_BGR2GRAY);
    img_cut.release();

    cv::blur(img_gray, img_gray, {3, 3});
    cv::Mat img_edges;
    cv::Canny(img_gray, img_edges, 10, 40);
    img_gray.release();

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(img_edges, lines, 1, CV_PI / 180, 80, 30, width / 10);
    img_edges.release();

    // partition the lines according to their angles into horizontal
    // and vertical ones
    auto mid = std::partition(lines.begin(), lines.end(), [](cv::Vec4i& line) {
        return std::abs(line[0] - line[2]) > std::abs(line[1] - line[3]);
    });

    // horizontal scores
    auto h_score = [height](const cv::Vec4i& l) {
        auto len = std::sqrt(std::pow(l[0] - l[2], 2) + std::pow(l[1] - l[3], 2));
        auto pos = (l[1] + l[3] - height) / 2.0;
        return len * pos;
    };

    // vertical scores
    auto v_score = [width](const cv::Vec4i& l) {
        auto len = std::sqrt(std::pow(l[0] - l[2], 2) + std::pow(l[1] - l[3], 2));
        auto pos = (l[0] + l[2] - width) / 2.0;
        return len * pos;
    };

    std::sort(lines.begin(), mid, [&](const cv::Vec4i& l1, const cv::Vec4i& l2) {
        return h_score(l1) < h_score(l2);
    });

    std::sort(mid, lines.end(), [&](const cv::Vec4i& l1, const cv::Vec4i& l2) {
        return v_score(l1) < v_score(l2);
    });

    QLineF top_line, bottom_line, left_line, right_line;

    if (mid != lines.begin()) {
        top_line.setLine(lines[0][0], lines[0][1], lines[0][2], lines[0][3]);
        bottom_line.setLine(mid[-1][0], mid[-1][1], mid[-1][2], mid[-1][3]);
    } else {
        top_line.setLine(0, 0, width, 0);
        bottom_line.setLine(0, height, width, height);
    }

    if (mid != lines.end()) {
        left_line.setLine((*mid)[0], (*mid)[1], (*mid)[2], (*mid)[3]);
        right_line.setLine(
            lines.end()[-1][0], lines.end()[-1][1], lines.end()[-1][2], lines.end()[-1][3]);
    } else {
        left_line.setLine(0, 0, 0, height);
        right_line.setLine(width, 0, width, height);
    }

    QPointF topleft, topright, bottomright, bottomleft;
    if (top_line.intersect(left_line, &topleft) == QLineF::NoIntersection) {
        topleft = {0, 0};
    }
    if (top_line.intersect(right_line, &topright) == QLineF::NoIntersection) {
        topright = {(qreal)width, 0};
    }
    if (bottom_line.intersect(left_line, &bottomleft) == QLineF::NoIntersection) {
        bottomleft = {0, (qreal)height};
    }
    if (bottom_line.intersect(right_line, &bottomright) == QLineF::NoIntersection) {
        bottomright = {(qreal)width, (qreal)height};
    }

    auto project = [width, height](QPointF& p) {
        p.setX(std::max((qreal)0.0, std::min((qreal)1.0, p.x() / width)));
        p.setY(std::max((qreal)0.0, std::min((qreal)1.0, p.y() / height)));
    };

    project(topleft);
    project(topright);
    project(bottomleft);
    project(bottomright);

    QVariantList lst;
    lst << topleft << topright << bottomright << bottomleft;
    return lst;
}

QJsonObject CutFilter::saveJson() const
{
    return {};
}

void CutFilter::loadJson(const QJsonObject& object) {}

QImage CutFilter::apply(QImage&& image)
{
    if (image.isNull()) {
        return image;
    }

    auto rotated = QImageToCvMat(image);

    auto w = static_cast<float>(rotated.cols);
    auto h = static_cast<float>(rotated.rows);

    cv::Point2f tl(d->topleft.x() * w, d->topleft.y() * h);
    cv::Point2f tr(d->topright.x() * w, d->topright.y() * h);
    cv::Point2f br(d->bottomright.x() * w, d->bottomright.y() * h);
    cv::Point2f bl(d->bottomleft.x() * w, d->bottomleft.y() * h);

    auto ratio = d->getAspectRatio(QPointF{tl.x - w / 2, tl.y - h / 2} / 100,
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
double CutFilter::Data::getAspectRatio(QPointF tl, QPointF tr, QPointF br, QPointF bl)
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

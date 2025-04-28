/*
 * Copyright (c) 2020, 2021 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include "EdgeDetection.hxx"

#include "Convert.hxx"

#include <QtCore/QLineF>
#include <QtGui/QImage>

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>

#include <algorithm>
#include "fifr/util/Range.hxx"

using namespace fifr::util;

namespace
{
/// A quadrangle represented by its corner points.
struct Quadrangle {
    QPointF tl, tr, br, bl;
};

void transpose(QLineF& l)
{
    l.setLine(l.y1(), l.x1(), l.y2(), l.x2());
}

}  // namespace

struct EdgeDetection::Data {
    // The default lower threshold value for canny edge detection
    const int DefaultCannyMin = 10;
    // The default upper threshold value for canny edge detection
    const int DefaultCannyMax = 50;
    // The default blur radius
    const int DefaultBlurRadius = 10;
    // The default contrast factor
    const qreal DefaultContractFactor = 1.5;

    int cannyMinVal = DefaultCannyMin;
    int cannyMaxVal = DefaultCannyMax;
    int blurRadius = DefaultBlurRadius;
    qreal contrastFactor = DefaultContractFactor;

    std::vector<QLineF> hlines;
    std::vector<QLineF> vlines;

    std::vector<std::vector<std::size_t>> left_lines, right_lines;
    std::vector<std::vector<std::size_t>> top_lines, bottom_lines;

    Quadrangle quad;  ///< The currently selected quadrangle.

    int width = 0;
    int height = 0;

    QImage image;
    QImage gray_image;
    QImage bw_image;

    void find_edge_candidates(std::vector<QLineF>& all_lines);
    static void filter_by_length(std::vector<QLineF>& edges);
    static void filter_by_angle(const std::vector<QLineF>& all_lines, std::vector<QLineF>& hlines, std::vector<QLineF>& vlines);

    void cluster_edges();
    static void cluster_edges(std::vector<QLineF>& hlines, qreal w, qreal h);

    void find_best_match();

    static bool distances_to_intersection(const QLineF& l1, const QLineF& l2, qreal& alpha, qreal& beta);
    std::pair<qreal, Quadrangle> compute_area(std::size_t ileft, std::size_t iright, std::size_t itop, std::size_t ibottom) const;
};

EdgeDetection::EdgeDetection(const QImage& image)
    : d(std::make_unique<Data>())
{
    d->image = image;
}

EdgeDetection::EdgeDetection(std::unique_ptr<Data>&& d)
    : d(std::move(d)) {}

EdgeDetection::EdgeDetection(EdgeDetection&&) noexcept = default;

EdgeDetection& EdgeDetection::operator=(EdgeDetection&&) noexcept = default;

EdgeDetection::~EdgeDetection() = default;

void EdgeDetection::setCannyMinValue(int minVal)
{
    d->cannyMinVal = qMax(minVal, 0);
}

int EdgeDetection::cannyMinValue() const
{
    return d->cannyMinVal;
}

void EdgeDetection::setCannyMaxValue(int maxVal)
{
    d->cannyMaxVal = qMax(0, maxVal);
}

int EdgeDetection::cannyMaxValue() const
{
    return d->cannyMaxVal;
}

void EdgeDetection::setBlurRadius(int radius)
{
    d->blurRadius = qMax(0, radius);
}

int EdgeDetection::blurRadius() const
{
    return d->blurRadius;
}

void EdgeDetection::setContrastFactor(qreal factor)
{
    d->contrastFactor = qMax(static_cast<qreal>(0), factor);
}

qreal EdgeDetection::contrastFactor() const
{
    return d->contrastFactor;
}

int EdgeDetection::width() const
{
    return d->image.width();
}

int EdgeDetection::height() const
{
    return d->image.height();
}

QImage EdgeDetection::image() const
{
    return d->image;
}

QImage EdgeDetection::gray_image() const
{
    return d->gray_image;
}

QImage EdgeDetection::bw_image() const
{
    return d->bw_image;
}

std::vector<QPointF> EdgeDetection::points() const
{
    return {topLeft(), topRight(), bottomRight(), bottomLeft()};
}

void EdgeDetection::setTopLeft(const QPointF& tl)
{
    d->quad.tl = tl;
}

QPointF EdgeDetection::topLeft() const
{
    return d->quad.tl;
}

void EdgeDetection::setTopRight(const QPointF& tr)
{
    d->quad.tr = tr;
}

QPointF EdgeDetection::topRight() const
{
    return d->quad.tr;
}

void EdgeDetection::setBottomLeft(const QPointF& bl)
{
    d->quad.bl = bl;
}

QPointF EdgeDetection::bottomLeft() const
{
    return d->quad.bl;
}

void EdgeDetection::setTopPoint(const QPointF& p)
{
    // move the top line
    QLineF left(d->quad.tl, d->quad.bl);
    QLineF right(d->quad.tr, d->quad.br);
    QLineF top(d->quad.tl, d->quad.tr);
    top.translate(p - top.center());
    top.intersect(left, &d->quad.tl);
    top.intersect(right, &d->quad.tr);
}

QPointF EdgeDetection::topPoint() const
{
    return QLineF(d->quad.tl, d->quad.tr).center();
}

void EdgeDetection::setBottomPoint(const QPointF& p)
{
}

QPointF EdgeDetection::bottomPoint() const
{
    return QLineF(d->quad.bl, d->quad.br).center();
}

void EdgeDetection::setLeftPoint(const QPointF& p)
{
}

QPointF EdgeDetection::leftPoint() const
{
    return QLineF(d->quad.tl, d->quad.bl).center();
}

void EdgeDetection::setRightPoint(const QPointF& p)
{
}

QPointF EdgeDetection::rightPoint() const
{
    return QLineF(d->quad.tr, d->quad.br).center();
}

void EdgeDetection::setBottomRight(const QPointF& br)
{
    d->quad.br = br;
}

QPointF EdgeDetection::bottomRight() const
{
    return d->quad.br;
}

std::vector<QLineF> EdgeDetection::vertical_lines() const
{
    std::vector<QLineF> vlines;
    vlines.reserve(d->vlines.size());
    for (auto& l : d->vlines) {
        vlines.push_back(l);
    }
    return vlines;
}

std::vector<QLineF> EdgeDetection::horizontal_lines() const
{
    std::vector<QLineF> hlines;
    hlines.reserve(d->hlines.size());
    for (auto& l : d->hlines) {
        hlines.push_back(l);
    }
    return hlines;
}

void EdgeDetection::Data::find_edge_candidates(std::vector<QLineF>& all_lines)
{
    // the original image
    auto img_cut = QImageToCvMat(image, false);

    // determine the size
    width = img_cut.cols;
    height = img_cut.rows;

    // convert to grayscale
    cv::Mat img_gray;
    cv::cvtColor(img_cut, img_gray, cv::COLOR_BGR2GRAY);
    img_cut.release();

    // blur a little bit to remove fragments
    cv::blur(img_gray, img_gray, {blurRadius, blurRadius});

    img_gray *= contrastFactor;

    gray_image = cvMatToQImage(img_gray).copy();

    // canny edge detection
    cv::Mat img_edges;
    cv::Canny(img_gray, img_edges, cannyMinVal, cannyMaxVal, 3);
    bw_image = cvMatToQImage(img_edges).copy();
    img_gray.release();

    // find raw lines
    std::vector<cv::Vec4i> lines;
    double len = std::min(width, height);
    cv::HoughLinesP(img_edges, lines, 1, CV_PI / 180, 80, len / 10, len / 40);
    img_edges.release();

    all_lines.reserve(lines.size());
    for (auto l : lines) {
        all_lines.emplace_back(l[0], l[1], l[2], l[3]);
    }
}

void EdgeDetection::Data::filter_by_angle(const std::vector<QLineF>& all_lines, std::vector<QLineF>& hlines, std::vector<QLineF>& vlines)
{
    for (auto l : all_lines) {
        auto dy = std::abs(l.y2() - l.y1());
        auto dx = std::abs(l.x2() - l.x1());

        if (dx > dy) {
            if (dy < 0.1 * dx) {
                if (l.x1() > l.x2()) {
                    hlines.push_back({l.x2(), l.y2(), l.x1(), l.y1()});
                } else {
                    hlines.push_back(l);
                }
            }
        } else {
            if (dx < 0.1 * dy) {
                if (l.y1() > l.y2()) {
                    vlines.push_back({l.x2(), l.y2(), l.x1(), l.y1()});
                } else {
                    vlines.push_back(l);
                }
            }
        }
    }
}

void EdgeDetection::autoDetect()
{
    // find candidate lines
    std::vector<QLineF> all_lines;
    d->find_edge_candidates(all_lines);

    // filter lines by angle
    d->filter_by_angle(all_lines, d->hlines, d->vlines);

    d->cluster_edges();

    d->find_best_match();
}

EdgeDetection EdgeDetection::detect_in_image(const QImage& image)
{
    auto edges = EdgeDetection(image);
    edges.autoDetect();
    return edges;
}

bool EdgeDetection::Data::distances_to_intersection(const QLineF& l1, const QLineF& l2, qreal& alpha, qreal& beta)
{
    auto a = -l1.dx();
    auto b = l2.dx();
    auto c = -l1.dy();
    auto d = l2.dy();
    auto det = a * d - b * c;

    if (std::abs(det) < 1e-6) return false;

    auto rhs1 = l1.x1() - l2.x1();
    auto rhs2 = l1.y1() - l2.y1();
    alpha = qreal(d * rhs1 - b * rhs2) / det;
    beta = qreal(-c * rhs1 + a * rhs2) / det;

    return true;
}

void EdgeDetection::Data::filter_by_length(std::vector<QLineF>& edges)
{
    if (edges.empty()) return;

    auto max_l = edges[0].length();
    for (auto l : edges) {
        max_l = std::max(max_l, l.length());
    }

    std::size_t ndel = 0;
    for (auto i : indices(edges)) {
        if (edges[i].length() < 0.6 * max_l) {
            ndel += 1;
        } else {
            edges[i - ndel] = edges[i];
        }
    }
    edges.resize(edges.size() - ndel, edges[0]);
}

void EdgeDetection::Data::cluster_edges()
{
    cluster_edges(hlines, width, height);

    for (auto& e : vlines) transpose(e);
    cluster_edges(vlines, height, width);
    for (auto& e : vlines) transpose(e);
}

void EdgeDetection::Data::cluster_edges(std::vector<QLineF>& hlines, qreal w, qreal h)
{
    std::vector<QLineF> finaledges;
    std::vector<qreal> y_0, y_w;
    y_0.reserve(hlines.size());
    y_w.reserve(hlines.size());
    for (auto i : range(hlines.size())) {
        auto alpha_0 = (0 - hlines[i].x1()) / (hlines[i].x2() - hlines[i].x1());
        y_0.push_back(hlines[i].y1() + alpha_0 * (hlines[i].y2() - hlines[i].y1()));

        auto alpha_w = (w - hlines[i].x1()) / (hlines[i].x2() - hlines[i].x1());
        y_w.push_back(hlines[i].y1() + alpha_w * (hlines[i].y2() - hlines[i].y1()));
    }

    std::vector<std::vector<int>> neighbors(hlines.size());

    for (auto i : range(hlines.size())) {
        for (auto j : range(i + 1, hlines.size())) {
            // compute left and right y values for each line.
            if (std::abs(y_0[i] - y_0[j]) < 0.02 * h && std::abs(y_w[i] - y_w[j]) < 0.02 * h) {
                neighbors[i].push_back(j);
                neighbors[j].push_back(i);
            }
        }
    }

    std::vector<std::size_t> degrees(hlines.size());
    std::vector<std::size_t> nodes(hlines.size());
    for (auto i : range(hlines.size())) {
        nodes[i] = i;
        degrees[i] = neighbors[i].size();
    }

    std::sort(nodes.begin(), nodes.end(), [&](auto i, auto j) {
        if (degrees[i] != degrees[j]) {
            return degrees[i] > degrees[j];
        } else {
            return hlines[i].length() > hlines[j].length();
        }
    });

    std::vector<bool> used(hlines.size(), false);
    for (auto i : nodes) {
        if (used[i]) continue;

        used[i] = true;

        QPointF left = {hlines[i].x1(), hlines[i].y1()};
        QPointF right = {hlines[i].x2(), hlines[i].y2()};

        if (left.x() > right.x()) std::swap(left, right);

        for (auto j : neighbors[i]) {
            used[j] = true;
            QPointF l = {hlines[j].x1(), hlines[j].y1()};
            QPointF r = {hlines[j].x2(), hlines[j].y2()};
            if (l.x() > r.x()) std::swap(l, r);
            if (l.x() < left.x()) left = l;
            if (r.x() > right.x()) right = r;
        }

        auto d = right - left;
        left += 0.05 * d;
        right -= 0.05 * d;

        finaledges.emplace_back(left.x(), left.y(), right.x(), right.y());
    }

    hlines = finaledges;
}

void EdgeDetection::Data::find_best_match()
{
    // find possible matches for each line
    left_lines.assign(hlines.size(), {});
    right_lines.assign(hlines.size(), {});
    top_lines.assign(vlines.size(), {});
    bottom_lines.assign(vlines.size(), {});

    qreal alpha, beta;
    for (auto i : indices(hlines)) {
        auto& h = hlines[i];
        for (auto j : indices(vlines)) {
            auto& v = vlines[j];
            if (distances_to_intersection(h, v, alpha, beta)) {
                if (alpha <= 0 && beta <= 0) {
                    left_lines[i].push_back(j);
                } else if (alpha <= 0 && beta >= 1) {
                    bottom_lines[j].push_back(i);
                } else if (alpha >= 1 && beta <= 0) {
                    right_lines[i].push_back(j);
                } else if (alpha >= 1 && beta >= 1) {
                    bottom_lines[j].push_back(i);
                }
            }
        }
    }

    for (auto ls : left_lines) std::sort(ls.begin(), ls.end());
    for (auto ls : right_lines) std::sort(ls.begin(), ls.end());
    for (auto ls : top_lines) std::sort(ls.begin(), ls.end());
    for (auto ls : bottom_lines) std::sort(ls.begin(), ls.end());

    // By default we simply select everything. This is a fallback in case we can't
    // detect proper points.
    quad.tl = {0, 0};
    quad.tr = {static_cast<qreal>(width), 0};
    quad.br = {static_cast<qreal>(width), static_cast<qreal>(height)};
    quad.bl = {0, static_cast<qreal>(height)};

    qreal max_area = 0;
    for (auto i : indices(hlines)) {
        if (left_lines[i].empty() || right_lines[i].empty()) continue;

        for (auto l : left_lines[i]) {
            for (auto r : right_lines[i]) {
                std::size_t a = 0, b = 0;
                while (a < bottom_lines[l].size() && b < bottom_lines[r].size()) {
                    if (bottom_lines[l][a] == bottom_lines[r][b]) {
                        auto [area, q] = compute_area(l, r, i, bottom_lines[l][a]);
                        if (area > max_area) {
                            max_area = area;
                            quad = q;
                        }
                        ++a;
                        ++b;
                    } else if (bottom_lines[l][a] < bottom_lines[r][b]) {
                        ++a;
                    } else {
                        ++b;
                    }
                }
            }
        }
    }
}

auto EdgeDetection::Data::compute_area(std::size_t ileft, std::size_t iright, std::size_t itop, std::size_t ibottom) const -> std::pair<qreal, Quadrangle>
{
    QPointF tl, tr, br, bl;

    vlines[ileft].intersect(hlines[itop], &tl);
    vlines[iright].intersect(hlines[itop], &tr);
    vlines[iright].intersect(hlines[ibottom], &br);
    vlines[ileft].intersect(hlines[ibottom], &bl);

    auto e = (tl - br);
    auto f = tr - bl;

    auto a = tr - tl;
    auto b = tr - br;
    auto c = br - bl;
    auto d = tl - bl;

    auto a2 = QPointF::dotProduct(a, a);
    auto b2 = QPointF::dotProduct(b, b);
    auto c2 = QPointF::dotProduct(c, d);
    auto d2 = QPointF::dotProduct(d, d);
    auto e2 = QPointF::dotProduct(e, e);
    auto f2 = QPointF::dotProduct(f, f);

    auto area = std::sqrt(4 * e2 * f2 - std::pow(b2 + d2 - a2 - c2, 2)) / 4;
    return {area, {tl, tr, br, bl}};
}

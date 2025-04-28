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

#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFutureWatcher>
#include <QtCore/QLineF>
#include <QtGui/QImage>

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>

#include <algorithm>

#include "Convert.hxx"
#include "fifr/util/Range.hxx"

using namespace fifr::util;

namespace
{
constexpr qreal operator"" _qr(long double a)
{
    return static_cast<qreal>(a);
}

/// A quadrangle represented by its corner points.
struct Quadrangle {
    QPointF tl, tr, br, bl;
};

void transpose(QLineF& l)
{
    l.setLine(l.y1(), l.x1(), l.y2(), l.x2());
}

QPointF center(const QLineF& line)
{
    return {0.5_qr * (line.x1() + line.x2()), 0.5_qr * (line.y1() + line.y2())};
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
    // The default snappy square size in pixels
    const qreal DefaultSnappySize = 10;
    // The default snappy distance size in pixels
    const qreal DefaultSnappyDistance = 20;

    int cannyMinVal = DefaultCannyMin;
    int cannyMaxVal = DefaultCannyMax;
    int blurRadius = DefaultBlurRadius;
    qreal contrastFactor = DefaultContractFactor;

    std::vector<QLineF> hlines;
    std::vector<QLineF> vlines;

    /// The size in pixels of each "pixel-square" for snappy edges
    size_t snappy_size = DefaultSnappySize;
    /// The size in pixels for snappy detection.
    size_t snappy_dist = DefaultSnappyDistance;

    /// Horizontal snappy edges for each pixel/square
    std::vector<std::vector<std::size_t>> hsnappy;
    /// Vertical snappy edges for each pixel/square
    std::vector<std::vector<std::size_t>> vsnappy;

    QLineF top_non_snappy;
    QLineF bottom_non_snappy;
    QLineF left_non_snappy;
    QLineF right_non_snappy;

    Quadrangle autoquad;  ///< The automatically detected quadrangle.
    Quadrangle quad;      ///< The currently selected quadrangle.

    int width;
    int height;

    cv::Mat image;
#ifndef NDEBUG
    cv::Mat gray_image;
    cv::Mat bw_image;
#endif

    bool auto_detect_running = false;
    bool auto_select_finished = false;
    bool have_auto_select = false;
    QFutureWatcher<bool> auto_detection = QFutureWatcher<bool>();

    void doAutoDetect();

    void find_edge_candidates(std::vector<QLineF>& all_lines);
    static void filter_by_length(std::vector<QLineF>& edges);
    static void filter_by_angle(const std::vector<QLineF>& all_lines, std::vector<QLineF>& hlines, std::vector<QLineF>& vlines);

    void cluster_edges();
    static void cluster_edges(std::vector<QLineF>& hlines, qreal w, qreal h);

    void find_best_match();

    void find_snappy_edges();

    bool get_snappy_line(const QPointF& p, bool horizontal, QLineF& sline) const;

    /// Project the selected quadrangle to the image boundaries
    void project_quadrangle();

    /// Project point to image boundaries.
    QPointF project(const QPointF& p) const;

    static bool distances_to_intersection(const QLineF& l1, const QLineF& l2, qreal& alpha, qreal& beta);
    std::pair<qreal, Quadrangle> compute_area(std::size_t ileft, std::size_t iright, std::size_t itop, std::size_t ibottom) const;
};

EdgeDetection::EdgeDetection(QObject* parent)
    : QObject(parent),
      d(std::make_unique<Data>())
{
    connect(&d->auto_detection, &QFutureWatcher<bool>::finished, this, &EdgeDetection::onAutoDetectFinished);
}

EdgeDetection::EdgeDetection(const cv::Mat& image, QObject* parent)
    : EdgeDetection(parent)
{
    d->image = image;
    d->width = image.cols;
    d->height = image.rows;
}

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

void EdgeDetection::setSnappySize(std::size_t snappy_size)
{
    d->snappy_size = std::max(snappy_size, static_cast<std::size_t>(1));
}

size_t EdgeDetection::snappySize() const
{
    return d->snappy_size;
}

void EdgeDetection::fixNonSnappyEdges()
{
    d->top_non_snappy = QLineF(topLeft(), topRight());
    d->bottom_non_snappy = QLineF(bottomLeft(), bottomRight());
    d->left_non_snappy = QLineF(topLeft(), bottomLeft());
    d->right_non_snappy = QLineF(topRight(), bottomRight());
}

int EdgeDetection::width() const
{
    return d->width;
}

int EdgeDetection::height() const
{
    return d->height;
}

#ifndef NDEBUG
QImage EdgeDetection::image() const
{
    return cvMatToQImage(d->image).copy();
}

QImage EdgeDetection::gray_image() const
{
    return cvMatToQImage(d->gray_image).copy();
}

QImage EdgeDetection::bw_image() const
{
    return cvMatToQImage(d->bw_image).copy();
}
#endif

std::vector<QPointF> EdgeDetection::points() const
{
    return {topLeft(), topRight(), bottomRight(), bottomLeft()};
}

void EdgeDetection::setTopLeft(const QPointF& tl)
{
    d->quad.tl = d->project(tl);
}

QPointF EdgeDetection::topLeft() const
{
    return d->quad.tl;
}

void EdgeDetection::setTopRight(const QPointF& tr)
{
    d->quad.tr = d->project(tr);
}

QPointF EdgeDetection::topRight() const
{
    return d->quad.tr;
}

void EdgeDetection::setBottomLeft(const QPointF& bl)
{
    d->quad.bl = d->project(bl);
}

QPointF EdgeDetection::bottomLeft() const
{
    return d->quad.bl;
}

void EdgeDetection::setBottomRight(const QPointF& br)
{
    d->quad.br = d->project(br);
}

QPointF EdgeDetection::bottomRight() const
{
    return d->quad.br;
}

void EdgeDetection::setTopPoint(const QPointF& p)
{
    QLineF top;
    if (!d->get_snappy_line(p, true, top)) {
        top = d->top_non_snappy;
        top.translate(p - center(top));
    }

    top.intersect(d->left_non_snappy, &d->quad.tl);
    top.intersect(d->right_non_snappy, &d->quad.tr);
    d->project_quadrangle();
}

QPointF EdgeDetection::topPoint() const
{
    return center(QLineF(d->quad.tl, d->quad.tr));
}

void EdgeDetection::setBottomPoint(const QPointF& p)
{
    QLineF bottom;
    if (!d->get_snappy_line(p, true, bottom)) {
        bottom = d->bottom_non_snappy;
        bottom.translate(p - center(bottom));
    }

    bottom.intersect(d->left_non_snappy, &d->quad.bl);
    bottom.intersect(d->right_non_snappy, &d->quad.br);
    d->project_quadrangle();
}

QPointF EdgeDetection::bottomPoint() const
{
    return center(QLineF(d->quad.bl, d->quad.br));
}

void EdgeDetection::setLeftPoint(const QPointF& p)
{
    QLineF left;
    if (!d->get_snappy_line(p, false, left)) {
        left = d->left_non_snappy;
        left.translate(p - center(left));
    }

    left.translate(p - center(left));
    left.intersect(d->top_non_snappy, &d->quad.tl);
    left.intersect(d->bottom_non_snappy, &d->quad.bl);
    d->project_quadrangle();
}

QPointF EdgeDetection::leftPoint() const
{
    return center(QLineF(d->quad.tl, d->quad.bl));
}

void EdgeDetection::setRightPoint(const QPointF& p)
{
    QLineF right;
    if (!d->get_snappy_line(p, false, right)) {
        right = d->right_non_snappy;
        right.translate(p - center(right));
    }

    right.translate(p - center(right));
    right.intersect(d->top_non_snappy, &d->quad.tr);
    right.intersect(d->bottom_non_snappy, &d->quad.br);
    d->project_quadrangle();
}

QPointF EdgeDetection::rightPoint() const
{
    return center(QLineF(d->quad.tr, d->quad.br));
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
    auto img_cut = image;

    // convert to grayscale
    cv::Mat img_gray;
    cv::cvtColor(img_cut, img_gray, cv::COLOR_BGR2GRAY);
    img_cut.release();

    // blur a little bit to remove fragments
    cv::blur(img_gray, img_gray, {blurRadius, blurRadius});

    img_gray *= contrastFactor;

#ifndef NDEBUG
    this->gray_image = img_gray;
#endif

    // canny edge detection
    cv::Mat img_edges;
    cv::Canny(img_gray, img_edges, cannyMinVal, cannyMaxVal, 3);
    img_gray.release();

#ifndef NDEBUG
    this->bw_image = img_edges;
#endif

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

bool EdgeDetection::isAutoDetectionRunning() const
{
    return d->auto_detect_running;
}

bool EdgeDetection::hasAutoDetection() const
{
    return d->have_auto_select;
}

void EdgeDetection::startAutoDetect()
{
    if (d->auto_select_finished) {
        emit edgeDetectionFinished();
    } else {
        d->auto_detect_running = true;
        emit isAutoDetectionRunningChanged();
        d->auto_detection.setFuture(QtConcurrent::run([this]() {
            d->doAutoDetect();
            return true;
        }));
    }
}

void EdgeDetection::onAutoDetectFinished()
{
    d->auto_detect_running = false;
    emit isAutoDetectionRunningChanged();

    d->auto_select_finished = true;
    auto have_auto_select = d->auto_detection.result();
    if (have_auto_select != d->have_auto_select) {
        d->have_auto_select = have_auto_select;
        emit hasAutoDetectionChanged();
    }

    emit edgeDetectionFinished();
}

bool EdgeDetection::selectAuto()
{
    if (d->have_auto_select) {
        d->quad = d->autoquad;
        fixNonSnappyEdges();
        return true;
    } else {
        return false;
    }
}

void EdgeDetection::Data::doAutoDetect()
{
    // find candidate lines
    std::vector<QLineF> all_lines;
    find_edge_candidates(all_lines);

    // filter lines by angle
    filter_by_angle(all_lines, hlines, vlines);

    cluster_edges();

    find_best_match();

    find_snappy_edges();
}

void EdgeDetection::selectAll()
{
    d->quad.tl = QPointF{0.0, 0.0};
    d->quad.tr = QPointF{static_cast<qreal>(d->width), 0.0};
    d->quad.br = QPointF{static_cast<qreal>(d->width), static_cast<qreal>(d->height)};
    d->quad.bl = QPointF{0.0, static_cast<qreal>(d->height)};

    fixNonSnappyEdges();
    d->project_quadrangle();
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
    std::vector<std::vector<std::size_t>> left_lines(hlines.size());
    std::vector<std::vector<std::size_t>> right_lines(hlines.size());
    std::vector<std::vector<std::size_t>> top_lines(vlines.size());
    std::vector<std::vector<std::size_t>> bottom_lines(vlines.size());

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
    autoquad.tl = {0, 0};
    autoquad.tr = {static_cast<qreal>(width), 0};
    autoquad.br = {static_cast<qreal>(width), static_cast<qreal>(height)};
    autoquad.bl = {0, static_cast<qreal>(height)};

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
                            autoquad = q;
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

    // finally project auto rect onto image boundaries
    autoquad.tl = project(autoquad.tl);
    autoquad.tr = project(autoquad.tr);
    autoquad.br = project(autoquad.br);
    autoquad.bl = project(autoquad.bl);
}

void EdgeDetection::Data::find_snappy_edges()
{
    auto s = std::max(snappy_size, static_cast<std::size_t>(1));

    std::vector<std::vector<qreal>> hdists(width / s, std::vector(height / s, std::numeric_limits<qreal>::infinity()));
    std::vector<std::vector<qreal>> vdists(width / s, std::vector(height / s, std::numeric_limits<qreal>::infinity()));

    hsnappy.assign(width / s, std::vector(height / s, std::numeric_limits<std::size_t>::max()));
    vsnappy.assign(width / s, std::vector(height / s, std::numeric_limits<std::size_t>::max()));

    for (auto i : indices(hlines)) {
        auto normal = hlines[i].normalVector().unitVector();
        auto n = QPointF{normal.dx(), normal.dy()};
        auto b = QPointF::dotProduct(n, {normal.x1(), normal.y1()});

        for (auto x : indices(hsnappy)) {
            for (auto y : indices(hsnappy[x])) {
                auto dist = std::abs(QPointF::dotProduct(n, {(x + 0.5_qr) * s, (y + 0.5_qr) * s}) - b);

                if (dist <= snappy_dist && dist < hdists[x][y]) {
                    hdists[x][y] = dist;
                    hsnappy[x][y] = i;
                }
            }
        }
    }

    for (auto i : indices(vlines)) {
        auto normal = vlines[i].normalVector().unitVector();
        auto n = QPointF{normal.dx(), normal.dy()};
        auto b = QPointF::dotProduct(n, {normal.x1(), normal.y1()});

        for (auto x : indices(vsnappy)) {
            for (auto y : indices(vsnappy[x])) {
                auto dist = std::abs(QPointF::dotProduct(n, {(x + 0.5_qr) * s, (y + 0.5_qr) * s}) - b);

                if (dist <= snappy_dist && dist < vdists[x][y]) {
                    vdists[x][y] = dist;
                    vsnappy[x][y] = i;
                }
            }
        }
    }
}

QPointF EdgeDetection::Data::project(const QPointF& p) const
{
    return {qBound(static_cast<qreal>(0), p.x(), static_cast<qreal>(width)),
            qBound(static_cast<qreal>(0), p.y(), static_cast<qreal>(height))};
}

void EdgeDetection::Data::project_quadrangle()
{
    quad.tl = project(quad.tl);
    quad.tr = project(quad.tr);
    quad.br = project(quad.br);
    quad.bl = project(quad.bl);
}

bool EdgeDetection::Data::get_snappy_line(const QPointF& p, bool horizontal, QLineF& sline) const
{
    auto x = static_cast<std::size_t>(p.x() / snappy_size);
    auto y = static_cast<std::size_t>(p.y() / snappy_size);

    auto& snappy = horizontal ? hsnappy : vsnappy;
    auto& lines = horizontal ? hlines : vlines;

    QLineF top;
    if (x >= 0 && y >= 0 && x < snappy.size() && y < snappy[x].size() && snappy[x][y] < lines.size()) {
        sline = lines[snappy[x][y]];
        return true;
    } else {
        return false;
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

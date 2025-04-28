/*
 * Copyright (c) 2018, 2019, 2020, 2021 Frank Fischer <frank-fischer@shadow-soft.de>
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
#include "EdgeDetection.hxx"
#include "Fotokopierer.hxx"
#include "Scanner.hxx"

#include <QDebug>
#include <QtCore/QJsonObject>
#include <QtCore/QPointF>
#include <QtCore/QVariant>
#include <QtGui/QImage>
#include <QtGui/QPainter>

#include <opencv2/imgproc/imgproc.hpp>

#include <cmath>

static QPointF scale(const QPointF& p, int width, int height)
{
    return {p.x() / width, p.y() / height};
}

static QPointF scale(const QPointF& p, const QImage& img)
{
    return scale(p, img.width(), img.height());
}

static QPointF unscale(const QPointF& p, int width, int height)
{
    return {p.x() * width, p.y() * height};
}

struct CutFilter::Data {
    QPointF topleft;
    QPointF topright;
    QPointF bottomleft;
    QPointF bottomright;

    std::unique_ptr<EdgeDetection> edges = nullptr;

    double getAspectRatio(QPointF tl, QPointF tr, QPointF br, QPointF bl);
};

CutFilter::CutFilter(Scanner* image)
    : CutFilter(image, nullptr) {}

CutFilter::CutFilter(Scanner* image, Filter* previous_filter)
    : Filter(image, previous_filter), d(new Data)
{
}

CutFilter::~CutFilter() = default;

void CutFilter::reset()
{
    setTopLeft({0, 0});
    setTopRight({1, 0});
    setBottomRight({1, 1});
    setBottomLeft({0, 1});
}

bool CutFilter::updateCut()
{
    static Fotokopierer util;

    if (!util.isConvex(d->topleft, d->topright, d->bottomright, d->bottomleft)) {
        return false;
    }

    /// Emitting this signal will cause the filtered image to be updated.
    emit filterChanged();
    return true;
}

void CutFilter::setTopLeft(QPointF topleft)
{
    if (d->topleft != topleft) {
        d->topleft = topleft;
        emit topLeftChanged();
        if (d->edges != nullptr) {
            d->edges->setTopLeft(unscale(topleft, d->edges->width(), d->edges->height()));
            emit topChanged();
            emit leftChanged();
        }
    }
}

QPointF CutFilter::topLeft() const
{
    return d->topleft;
}

void CutFilter::setTopRight(QPointF topright)
{
    if (d->topright != topright) {
        d->topright = topright;
        emit topRightChanged();
        if (d->edges != nullptr) {
            d->edges->setTopRight(unscale(topright, d->edges->width(), d->edges->height()));
            emit topChanged();
            emit rightChanged();
        }
    }
}

QPointF CutFilter::topRight() const
{
    return d->topright;
}

void CutFilter::setBottomLeft(QPointF bottomleft)
{
    if (d->bottomleft != bottomleft) {
        d->bottomleft = bottomleft;
        emit bottomLeftChanged();
        if (d->edges != nullptr) {
            d->edges->setBottomLeft(unscale(bottomleft, d->edges->width(), d->edges->height()));
            emit bottomChanged();
            emit leftChanged();
        }
    }
}

QPointF CutFilter::bottomLeft() const
{
    return d->bottomleft;
}

void CutFilter::setBottomRight(QPointF bottomright)
{
    if (d->bottomright != bottomright) {
        d->bottomright = bottomright;
        emit bottomRightChanged();
        if (d->edges != nullptr) {
            d->edges->setBottomRight(unscale(bottomright, d->edges->width(), d->edges->height()));
            emit bottomChanged();
            emit rightChanged();
        }
    }
}

QPointF CutFilter::bottomRight() const
{
    return d->bottomright;
}

void CutFilter::setTop(QPointF top)
{
    if (d->edges == nullptr) return;

    auto p = unscale(top, d->edges->width(), d->edges->height());
    if (p != d->edges->topPoint()) {
        d->edges->setTopPoint(p);
        emit topChanged();

        auto tl = scale(d->edges->topLeft(), d->edges->width(), d->edges->height());
        if (tl != d->topleft) {
            d->topleft = tl;
            emit topLeftChanged();
        }

        auto tr = scale(d->edges->topRight(), d->edges->width(), d->edges->height());
        if (tr != d->topright) {
            d->topright = tr;
            emit topRightChanged();
        }
    }
}

QPointF CutFilter::top() const
{
    if (d->edges != nullptr) {
        return scale(d->edges->topPoint(), d->edges->width(), d->edges->height());
    } else {
        return {};
    }
}

void CutFilter::setBottom(QPointF bottom)
{
    if (d->edges == nullptr) return;

    auto p = unscale(bottom, d->edges->width(), d->edges->height());
    if (p != d->edges->bottomPoint()) {
        d->edges->setBottomPoint(p);
        emit bottomChanged();

        auto bl = scale(d->edges->bottomLeft(), d->edges->width(), d->edges->height());
        if (bl != d->bottomleft) {
            d->bottomleft = bl;
            emit bottomLeftChanged();
        }

        auto br = scale(d->edges->bottomRight(), d->edges->width(), d->edges->height());
        if (br != d->bottomright) {
            d->bottomright = br;
            emit bottomRightChanged();
        }
    }
}

QPointF CutFilter::bottom() const
{
    if (d->edges != nullptr) {
        return scale(d->edges->bottomPoint(), d->edges->width(), d->edges->height());
    } else {
        return {};
    }
}

void CutFilter::setLeft(QPointF left)
{
    if (d->edges == nullptr) return;

    auto p = unscale(left, d->edges->width(), d->edges->height());
    if (p != d->edges->leftPoint()) {
        d->edges->setLeftPoint(p);
        emit leftChanged();

        auto tl = scale(d->edges->topLeft(), d->edges->width(), d->edges->height());
        if (tl != d->topleft) {
            d->topleft = tl;
            emit topLeftChanged();
        }

        auto bl = scale(d->edges->bottomLeft(), d->edges->width(), d->edges->height());
        if (bl != d->bottomleft) {
            d->bottomleft = bl;
            emit bottomLeftChanged();
        }
    }
}

QPointF CutFilter::left() const
{
    if (d->edges != nullptr) {
        return scale(d->edges->leftPoint(), d->edges->width(), d->edges->height());
    } else {
        return {};
    }
}

void CutFilter::setRight(QPointF right)
{
    if (d->edges == nullptr) return;

    auto p = unscale(right, d->edges->width(), d->edges->height());
    if (p != d->edges->rightPoint()) {
        d->edges->setRightPoint(p);
        emit rightChanged();

        auto tr = scale(d->edges->topRight(), d->edges->width(), d->edges->height());
        if (tr != d->topright) {
            d->topright = tr;
            emit topRightChanged();
        }

        auto br = scale(d->edges->bottomRight(), d->edges->width(), d->edges->height());
        if (br != d->bottomright) {
            d->bottomright = br;
            emit bottomRightChanged();
        }
    }
}

QPointF CutFilter::right() const
{
    if (d->edges != nullptr) {
        return scale(d->edges->rightPoint(), d->edges->width(), d->edges->height());
    } else {
        return {};
    }
}

void CutFilter::rotateLeft()
{
    if (d->edges == nullptr) return;

    auto tl = d->edges->topLeft();
    auto tr = d->edges->topRight();
    auto br = d->edges->bottomRight();
    auto bl = d->edges->bottomLeft();

    QImage img =
        previous_filter_ != nullptr ? previous_filter_->filteredImage() : image()->originalImage();

    d->edges = std::make_unique<EdgeDetection>(EdgeDetection::detect_in_image(img));
    d->edges->setTopLeft({tr.y(), img.height() - tr.x()});
    d->edges->setTopRight({br.y(), img.height() - br.x()});
    d->edges->setBottomRight({bl.y(), img.height() - bl.x()});
    d->edges->setBottomLeft({tl.y(), img.height() - tl.x()});

    emit topLeftChanged();
    emit topRightChanged();
    emit bottomRightChanged();
    emit bottomLeftChanged();
    emit topChanged();
    emit bottomChanged();
    emit leftChanged();
    emit rightChanged();
}

void CutFilter::rotateRight()
{
    if (d->edges == nullptr) return;

    auto tl = d->edges->topLeft();
    auto tr = d->edges->topRight();
    auto br = d->edges->bottomRight();
    auto bl = d->edges->bottomLeft();

    QImage img =
        previous_filter_ != nullptr ? previous_filter_->filteredImage() : image()->originalImage();

    d->edges = std::make_unique<EdgeDetection>(EdgeDetection::detect_in_image(img));
    d->edges->setTopLeft({img.width() - bl.y(), bl.x()});
    d->edges->setTopRight({img.width() - tl.y(), tl.x()});
    d->edges->setBottomRight({img.width() - tr.y(), tr.x()});
    d->edges->setBottomLeft({img.width() - br.y(), br.x()});

    emit topLeftChanged();
    emit topRightChanged();
    emit bottomRightChanged();
    emit bottomLeftChanged();
    emit topChanged();
    emit bottomChanged();
    emit leftChanged();
    emit rightChanged();
}

QVariantList CutFilter::selectAll()
{
    if (d->edges == nullptr) {
        QImage img =
            previous_filter_ != nullptr ? previous_filter_->filteredImage() : image()->originalImage();

        d->edges = std::make_unique<EdgeDetection>(EdgeDetection::detect_in_image(img));
    }

    d->edges->selectAll();

    d->topleft = scale(d->edges->topLeft(), d->edges->width(), d->edges->height());
    d->topright = scale(d->edges->topRight(), d->edges->width(), d->edges->height());
    d->bottomright = scale(d->edges->bottomRight(), d->edges->width(), d->edges->height());
    d->bottomleft = scale(d->edges->bottomLeft(), d->edges->width(), d->edges->height());

    QVariantList lst;
    lst << d->topleft << d->topright << d->bottomright << d->bottomleft;

    emit topLeftChanged();
    emit topRightChanged();
    emit bottomRightChanged();
    emit bottomLeftChanged();
    emit topChanged();
    emit bottomChanged();
    emit leftChanged();
    emit rightChanged();

    return lst;
}

QVariantList CutFilter::autoDetectCutRect()
{
    QImage img =
        previous_filter_ != nullptr ? previous_filter_->filteredImage() : image()->originalImage();

    d->edges = std::make_unique<EdgeDetection>(EdgeDetection::detect_in_image(img));

    d->topleft = scale(d->edges->topLeft(), img);
    d->topright = scale(d->edges->topRight(), img);
    d->bottomright = scale(d->edges->bottomRight(), img);
    d->bottomleft = scale(d->edges->bottomLeft(), img);

    QVariantList lst;
    lst << d->topleft << d->topright << d->bottomright << d->bottomleft;

    emit topLeftChanged();
    emit topRightChanged();
    emit bottomRightChanged();
    emit bottomLeftChanged();
    emit topChanged();
    emit bottomChanged();
    emit leftChanged();
    emit rightChanged();

    return lst;
}

void CutFilter::fixSnappyEdges()
{
    if (d->edges == nullptr) {
        QImage img =
            previous_filter_ != nullptr ? previous_filter_->filteredImage() : image()->originalImage();

        d->edges = std::make_unique<EdgeDetection>(EdgeDetection::detect_in_image(img));
        emit topChanged();
        emit bottomChanged();
        emit leftChanged();
        emit rightChanged();
    }

    if (d->edges != nullptr) d->edges->fixNonSnappyEdges();
}

QString CutFilter::name() const
{
    return QStringLiteral("cut");
}

static QJsonValue fromPoint(QPointF p)
{
    return QJsonObject{{QStringLiteral("x"), p.x()}, {QStringLiteral("y"), p.y()}};
}

QJsonObject CutFilter::saveJson() const
{
    return {
        {QStringLiteral("topleft"), fromPoint(d->topleft)},
        {QStringLiteral("topright"), fromPoint(d->topright)},
        {QStringLiteral("bottomleft"), fromPoint(d->bottomleft)},
        {QStringLiteral("bottomright"), fromPoint(d->bottomright)},
    };
}

static QPointF toPoint(const QJsonValue& value)
{
    auto p = value.toObject();
    return {
        static_cast<qreal>(p[QStringLiteral("x")].toDouble(0)),
        static_cast<qreal>(p[QStringLiteral("y")].toDouble(0)),
    };
}

void CutFilter::loadJson(const QJsonObject& object)
{
    setTopLeft(toPoint(object[QStringLiteral("topleft")]));
    setTopRight(toPoint(object[QStringLiteral("topright")]));
    setBottomRight(toPoint(object[QStringLiteral("bottomright")]));
    setBottomLeft(toPoint(object[QStringLiteral("bottomleft")]));
}

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

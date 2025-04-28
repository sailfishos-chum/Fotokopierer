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

#include "CutView.hxx"

#include "Convert.hxx"
#include "EdgeDetection.hxx"
#include "ScanImage.hxx"
#include "Scanner.hxx"

#include <QDebug>
#include <QtGui/QImage>
#include <QtGui/QPainter>

static QPointF scale(const QPointF& p, int width, int height)
{
    return {p.x() / width, p.y() / height};
}

static QPointF scale(const QPointF& p, const std::unique_ptr<EdgeDetection>& edges)
{
    return scale(p, edges->width(), edges->height());
}

static QPointF unscale(const QPointF& p, int width, int height)
{
    return {p.x() * width, p.y() * height};
}

static QPointF unscale(const QPointF& p, const std::unique_ptr<EdgeDetection>& edges)
{
    return unscale(p, edges->width(), edges->height());
}

struct CutView::Data {
    std::unique_ptr<EdgeDetection> edges;
    std::shared_ptr<ScanImage> scanImage;

    QMetaObject::Connection orientationChangedConnection = {};
    bool updatePaintedSize = false;

    QPointF getPoint(int which) const
    {
        if (scanImage == nullptr || edges == nullptr) return {};

        auto orien = scanImage->orientation();
        QPointF pnt;
        switch ((4 + which - orien) % 4) {
            case 0: pnt = edges->topLeft(); break;
            case 1: pnt = edges->topRight(); break;
            case 2: pnt = edges->bottomRight(); break;
            case 3: pnt = edges->bottomLeft(); break;
            default: Q_ASSERT(false); break;
        };

        pnt = ::scale(pnt, edges);

        switch (orien % 4) {
            case 0: return pnt;
            case 1: return {1 - pnt.y(), pnt.x()};
            case 2: return {1 - pnt.x(), 1 - pnt.y()};
            case 3: return {pnt.y(), 1 - pnt.x()};
        }

        return {};
    }

    bool setPoint(int which, const QPointF& p)
    {
        if (scanImage == nullptr || edges == nullptr) return false;

        if (getPoint(which) == p) return false;

        auto orien = scanImage->orientation();
        QPointF pnt = p;
        switch (orien % 4) {
            case 0: break;
            case 1: pnt = {pnt.y(), 1 - pnt.x()}; break;
            case 2: pnt = {1 - pnt.x(), 1 - pnt.y()}; break;
            case 3: pnt = {1 - pnt.y(), pnt.x()}; break;
        }

        pnt = ::unscale(pnt, edges);

        switch ((4 + which - orien) % 4) {
            case 0: edges->setTopLeft(pnt); break;
            case 1: edges->setTopRight(pnt); break;
            case 2: edges->setBottomRight(pnt); break;
            case 3: edges->setBottomLeft(pnt); break;
        };

        return true;
    }

    QPointF getEdgePoint(int which) const
    {
        if (scanImage == nullptr || edges == nullptr) return {};

        auto orien = scanImage->orientation();
        QPointF pnt;
        switch ((4 + which - orien) % 4) {
            case 0: pnt = edges->topPoint(); break;
            case 1: pnt = edges->rightPoint(); break;
            case 2: pnt = edges->bottomPoint(); break;
            case 3: pnt = edges->leftPoint(); break;
            default: Q_ASSERT(false); break;
        };

        pnt = ::scale(pnt, edges);

        switch (orien % 4) {
            case 0: return pnt;
            case 1: return {1 - pnt.y(), pnt.x()};
            case 2: return {1 - pnt.x(), 1 - pnt.y()};
            case 3: return {pnt.y(), 1 - pnt.x()};
        }

        return {};
    }

    bool setEdgePoint(int which, const QPointF& p)
    {
        if (scanImage == nullptr || edges == nullptr) return false;

        if (getPoint(which) == p) return false;

        auto orien = scanImage->orientation();
        QPointF pnt = p;
        switch (orien % 4) {
            case 0: break;
            case 1: pnt = {pnt.y(), 1 - pnt.x()}; break;
            case 2: pnt = {1 - pnt.x(), 1 - pnt.y()}; break;
            case 3: pnt = {1 - pnt.y(), pnt.x()}; break;
        }

        pnt = ::unscale(pnt, edges);

        switch ((4 + which - orien) % 4) {
            case 0: edges->setTopPoint(pnt); break;
            case 1: edges->setRightPoint(pnt); break;
            case 2: edges->setBottomPoint(pnt); break;
            case 3: edges->setLeftPoint(pnt); break;
        };

        return true;
    }
};

CutView::CutView(QQuickItem* parent)
    : ScanImageView(parent),
      d(new Data)
{
}

CutView::~CutView() = default;

int CutView::orientation() const
{
    return d->scanImage != nullptr ? d->scanImage->orientation() : 0;
}

void CutView::setOrientation(int orientation)
{
    if (d->scanImage != nullptr) d->scanImage->setOrientation(orientation);
}

QPointF CutView::topLeft() const
{
    return d->getPoint(0);
}

void CutView::setTopLeft(QPointF topleft)
{
    if (d->setPoint(0, topleft)) {
        emit topLeftChanged();
        emit topChanged();
        emit leftChanged();
    }
}

QPointF CutView::topRight() const
{
    return d->getPoint(1);
}

void CutView::setTopRight(QPointF topright)
{
    if (d->setPoint(1, topright)) {
        emit topRightChanged();
        emit topChanged();
        emit rightChanged();
    }
}

QPointF CutView::bottomRight() const
{
    return d->getPoint(2);
}

void CutView::setBottomRight(QPointF bottomright)
{
    if (d->setPoint(2, bottomright)) {
        emit bottomRightChanged();
        emit bottomChanged();
        emit rightChanged();
    }
}

QPointF CutView::bottomLeft() const
{
    return d->getPoint(3);
}

void CutView::setBottomLeft(QPointF bottomleft)
{
    if (d->setPoint(3, bottomleft)) {
        emit bottomLeftChanged();
        emit bottomChanged();
        emit leftChanged();
    }
}

void CutView::setTop(QPointF top)
{
    if (d->setEdgePoint(0, top)) {
        emit topChanged();
        emit topLeftChanged();
        emit topRightChanged();
        emit leftChanged();
        emit rightChanged();
    }
}

QPointF CutView::top() const
{
    return d->getEdgePoint(0);
}

void CutView::setRight(QPointF right)
{
    if (d->setEdgePoint(1, right)) {
        emit rightChanged();
        emit topRightChanged();
        emit bottomRightChanged();
        emit topChanged();
        emit bottomChanged();
    }
}

QPointF CutView::right() const
{
    return d->getEdgePoint(1);
}

void CutView::setBottom(QPointF bottom)
{
    if (d->setEdgePoint(2, bottom)) {
        emit bottomChanged();
        emit bottomLeftChanged();
        emit bottomRightChanged();
        emit leftChanged();
        emit rightChanged();
    }
}

QPointF CutView::bottom() const
{
    return d->getEdgePoint(2);
}

void CutView::setLeft(QPointF left)
{
    if (d->setEdgePoint(3, left)) {
        emit leftChanged();
        emit topLeftChanged();
        emit bottomLeftChanged();
        emit topChanged();
        emit bottomChanged();
    }
}

QPointF CutView::left() const
{
    return d->getEdgePoint(3);
}

void CutView::rotateLeft()
{
    if (d->scanImage != nullptr) {
        d->scanImage->setOrientation(d->scanImage->orientation() - 1);
        d->updatePaintedSize = true;
    }
}

void CutView::rotateRight()
{
    if (d->scanImage != nullptr) {
        d->scanImage->setOrientation(d->scanImage->orientation() + 1);
        d->updatePaintedSize = true;
    }
}

bool CutView::hasAutoSelection() const
{
    return d->edges != nullptr ? d->edges->hasAutoDetection() : false;
}

bool CutView::isAutoDetectionRunning() const
{
    return d->edges != nullptr ? d->edges->isAutoDetectionRunning() : false;
}

void CutView::selectAll()
{
    if (d->edges != nullptr) {
        d->edges->setTopLeft({0, 0});
        d->edges->setTopRight({static_cast<qreal>(d->edges->width()), 0});
        d->edges->setBottomRight({static_cast<qreal>(d->edges->width()), static_cast<qreal>(d->edges->height())});
        d->edges->setBottomLeft({0, static_cast<qreal>(d->edges->height())});

        emit topLeftChanged();
        emit topRightChanged();
        emit bottomRightChanged();
        emit bottomLeftChanged();
        emit topChanged();
        emit bottomChanged();
        emit leftChanged();
        emit rightChanged();
    }
}

void CutView::selectAuto()
{
    if (d->edges != nullptr && d->edges->selectAuto()) {
        emit topLeftChanged();
        emit topRightChanged();
        emit bottomRightChanged();
        emit bottomLeftChanged();
        emit topChanged();
        emit bottomChanged();
        emit leftChanged();
        emit rightChanged();
    }
}

void CutView::paint(QPainter* painter)
{
    QImage image = d->scanImage->original();

    auto orien = orientation();
    auto imgw = image.width();
    auto imgh = image.height();

    if (imgw > 0 && imgh > 0) {
        auto wratio = static_cast<qreal>(orien % 2 == 0 ? width() : height()) / imgw;
        auto hratio = static_cast<qreal>(orien % 2 == 0 ? height() : width()) / imgh;
        auto ratio = std::min(wratio, hratio);

        auto w = imgw * ratio;
        auto h = imgh * ratio;

        if (orien % 2 == 1) std::swap(w, h);

        setPaintedSize(w, h);

        painter->save();
        painter->rotate(orien * 90);

        QPointF target;
        switch (orien) {
            case 0: target = {(width() - w) / 2, (height() - h) / 2}; break;
            case 1: target = {height() / 2 - h / 2, -width() / 2 - w / 2}; break;
            case 2: target = {-width() / 2 - w / 2, -height() / 2 - h / 2}; break;
            case 3: target = {-height() / 2 - h / 2, width() / 2 - w / 2}; break;
            default: Q_ASSERT(false);
        }

        painter->drawImage(QRectF{target, QSize(imgw * ratio, imgh * ratio)}, image);

        painter->restore();
    }

    if (d->updatePaintedSize) {
        d->updatePaintedSize = false;

        // We need to postpone the rotation until the repaint is
        // complete because otherwise paintedWidth and paintedHeight
        // will not be up-to-date. The problem is that we do not know
        // `width` and `height` until we actually repaint the widget,
        // hence we do not now `paintedWidth` and `paintedHeight`
        // either.

        emit topLeftChanged();
        emit topRightChanged();
        emit bottomRightChanged();
        emit bottomLeftChanged();
        emit topChanged();
        emit bottomChanged();
        emit leftChanged();
        emit rightChanged();
        emit rotationChanged();
    }
}

void CutView::onNewImage()
{
    disconnect(d->orientationChangedConnection);
    d->orientationChangedConnection = {};
    if (auto s = scanner(); s != nullptr) {
        d->scanImage = s->currentImage();
        if (d->scanImage != nullptr) {
            // set up connections for the new ScanImage ...
            d->orientationChangedConnection = connect(d->scanImage.get(), &ScanImage::orientationChanged, this, &CutView::onOrientationChanged);

            // ... and its edge-detection data structure
            d->edges = std::make_unique<EdgeDetection>(QImageToCvMat(d->scanImage->original()));
            connect(d->edges.get(), &EdgeDetection::hasAutoDetectionChanged, this, &CutView::hasAutoSelectionChanged);
            connect(d->edges.get(), &EdgeDetection::isAutoDetectionRunningChanged, this, &CutView::isAutoDetectionRunningChanged);

            emit hasAutoSelectionChanged();
            emit isAutoDetectionRunningChanged();

            // start right away
            d->edges->startAutoDetect();

            // this means that the settings will be initialized from the scanImage
            d->setPoint(0, d->scanImage->topLeft());
            d->setPoint(1, d->scanImage->topRight());
            d->setPoint(2, d->scanImage->bottomRight());
            d->setPoint(3, d->scanImage->bottomLeft());

            emit topLeftChanged();
            emit topRightChanged();
            emit bottomRightChanged();
            emit bottomLeftChanged();
            emit topChanged();
            emit bottomChanged();
            emit leftChanged();
            emit rightChanged();
            emit rotationChanged();
        }
    }

    update();
}

QImage CutView::image() const
{
    return d->scanImage->original();
}

void CutView::onOrientationChanged()
{
    if (d->scanImage != nullptr) {
        update();
    }

    emit topLeftChanged();
    emit topRightChanged();
    emit bottomRightChanged();
    emit bottomLeftChanged();
    emit topChanged();
    emit bottomChanged();
    emit leftChanged();
    emit rightChanged();
    emit rotationChanged();

    emit orientationChanged();
}

void CutView::updateSnappyEdges()
{
    if (d->edges != nullptr) {
        d->edges->fixNonSnappyEdges();
    }
}

void CutView::apply()
{
    if (d->edges != nullptr && d->scanImage != nullptr) {
        d->scanImage->setTopLeft(topLeft());
        d->scanImage->setTopRight(topRight());
        d->scanImage->setBottomRight(bottomRight());
        d->scanImage->setBottomLeft(bottomLeft());
        d->scanImage->applyCut();
    }
}

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

#include "EdgeDetection.hxx"
#include "ScanImage.hxx"
#include "Scanner.hxx"

#include <QDebug>
#include <QtGui/QImage>

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
    QImage image;

    int doRotate = 0;
    QPointF rotate_tl, rotate_tr, rotate_bl, rotate_br;

    QMetaObject::Connection rotatedImageChangedConnection = {};
    QMetaObject::Connection startRotatedImageUpdateConnection = {};
    QMetaObject::Connection finishRotatedImageUpdateConnection = {};
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
    return d->edges != nullptr ? ::scale(d->edges->topLeft(), d->edges) : QPointF{};
}

void CutView::setTopLeft(QPointF topleft)
{
    if (d->edges != nullptr) {
        auto tl = unscale(topleft, d->edges);
        if (tl != d->edges->topLeft()) {
            d->edges->setTopLeft(tl);
            emit topLeftChanged();
            emit topChanged();
            emit leftChanged();
        }
    }
}

QPointF CutView::topRight() const
{
    return d->edges != nullptr ? ::scale(d->edges->topRight(), d->edges) : QPointF{};
}

void CutView::setTopRight(QPointF topright)
{
    if (d->edges != nullptr) {
        auto tr = unscale(topright, d->edges);
        if (tr != d->edges->topRight()) {
            d->edges->setTopRight(tr);
            emit topRightChanged();
            emit topChanged();
            emit rightChanged();
        }
    }
}

QPointF CutView::bottomRight() const
{
    return d->edges != nullptr ? ::scale(d->edges->bottomRight(), d->edges) : QPointF{};
}

void CutView::setBottomRight(QPointF bottomright)
{
    if (d->edges != nullptr) {
        auto br = unscale(bottomright, d->edges);
        if (br != d->edges->bottomRight()) {
            d->edges->setBottomRight(br);
            emit bottomRightChanged();
            emit bottomChanged();
            emit rightChanged();
        }
    }
}

QPointF CutView::bottomLeft() const
{
    return d->edges != nullptr ? ::scale(d->edges->bottomLeft(), d->edges) : QPointF{};
}

void CutView::setBottomLeft(QPointF bottomleft)
{
    if (d->edges != nullptr) {
        auto bl = unscale(bottomleft, d->edges);
        if (bl != d->edges->bottomLeft()) {
            d->edges->setBottomLeft(bl);
            emit bottomLeftChanged();
            emit bottomChanged();
            emit leftChanged();
        }
    }
}

void CutView::setTop(QPointF top)
{
    if (d->edges != nullptr) {
        auto topPoint = unscale(top, d->edges);
        if (topPoint != d->edges->topPoint()) {
            d->edges->setTopPoint(topPoint);
            emit topChanged();
            emit topLeftChanged();
            emit topRightChanged();
            emit leftChanged();
            emit rightChanged();
        }
    }
}

QPointF CutView::top() const
{
    return d->edges != nullptr ? ::scale(d->edges->topPoint(), d->edges) : QPointF{};
}

void CutView::setBottom(QPointF bottom)
{
    if (d->edges != nullptr) {
        auto bottomPoint = unscale(bottom, d->edges);
        if (bottomPoint != d->edges->bottomPoint()) {
            d->edges->setBottomPoint(bottomPoint);
            emit bottomChanged();
            emit bottomLeftChanged();
            emit bottomRightChanged();
            emit leftChanged();
            emit rightChanged();
        }
    }
}

QPointF CutView::bottom() const
{
    return d->edges != nullptr ? ::scale(d->edges->bottomPoint(), d->edges) : QPointF{};
}

void CutView::setLeft(QPointF left)
{
    if (d->edges != nullptr) {
        auto leftPoint = unscale(left, d->edges);
        if (leftPoint != d->edges->leftPoint()) {
            d->edges->setLeftPoint(leftPoint);
            emit leftChanged();
            emit topLeftChanged();
            emit bottomLeftChanged();
            emit topChanged();
            emit bottomChanged();
        }
    }
}

QPointF CutView::left() const
{
    return d->edges != nullptr ? ::scale(d->edges->leftPoint(), d->edges) : QPointF{};
}

void CutView::setRight(QPointF right)
{
    if (d->edges != nullptr) {
        auto rightPoint = unscale(right, d->edges);
        if (rightPoint != d->edges->rightPoint()) {
            d->edges->setRightPoint(rightPoint);
            emit rightChanged();
            emit topRightChanged();
            emit bottomRightChanged();
            emit topChanged();
            emit bottomChanged();
        }
    }
}

QPointF CutView::right() const
{
    return d->edges != nullptr ? ::scale(d->edges->rightPoint(), d->edges) : QPointF{};
}

void CutView::rotateLeft()
{
    if (d->scanImage != nullptr) {
        d->scanImage->setOrientation(d->scanImage->orientation() - 1);

        auto tl = topLeft();
        auto tr = topRight();
        auto br = bottomRight();
        auto bl = bottomLeft();

        d->doRotate = 1;
        d->rotate_tl = {tr.y(), 1 - tr.x()};
        d->rotate_tr = {br.y(), 1 - br.x()};
        d->rotate_br = {bl.y(), 1 - bl.x()};
        d->rotate_bl = {tl.y(), 1 - tl.x()};
        // force the image to be rotated
        d->scanImage->rotatedImage();
    }
}

void CutView::rotateRight()
{
    if (d->scanImage != nullptr) {
        d->scanImage->setOrientation(d->scanImage->orientation() + 1);

        auto tl = topLeft();
        auto tr = topRight();
        auto br = bottomRight();
        auto bl = bottomLeft();

        d->doRotate = 1;
        d->rotate_tl = {1 - bl.y(), bl.x()};
        d->rotate_tr = {1 - tl.y(), tl.x()};
        d->rotate_br = {1 - tr.y(), tr.x()};
        d->rotate_bl = {1 - br.y(), br.x()};
        // force the image to be rotated
        d->scanImage->rotatedImage();
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
    ScanImageView::paint(painter);

    if (d->doRotate == 2) {
        // We need postpone the rotation until the repaint is complete because
        // otherwise paintedWidth and paintedHeight will not be up-to-date

        d->edges->setTopLeft(unscale(d->rotate_tl, d->edges));
        d->edges->setTopRight(unscale(d->rotate_tr, d->edges));
        d->edges->setBottomRight(unscale(d->rotate_br, d->edges));
        d->edges->setBottomLeft(unscale(d->rotate_bl, d->edges));

        emit topLeftChanged();
        emit topRightChanged();
        emit bottomRightChanged();
        emit bottomLeftChanged();
        emit topChanged();
        emit bottomChanged();
        emit leftChanged();
        emit rightChanged();
        emit rotationChanged();

        d->doRotate = 0;
    }
}

void CutView::onNewImage()
{
    disconnect(d->rotatedImageChangedConnection);
    disconnect(d->startRotatedImageUpdateConnection);
    disconnect(d->finishRotatedImageUpdateConnection);
    d->rotatedImageChangedConnection = {};
    d->startRotatedImageUpdateConnection = {};
    d->finishRotatedImageUpdateConnection = {};
    if (auto s = scanner(); s != nullptr) {
        d->scanImage = s->currentImage();
        if (d->scanImage != nullptr) {
            d->rotatedImageChangedConnection = connect(d->scanImage.get(), &ScanImage::rotatedImageChanged, this, &CutView::onRotatedImageChanged);
            d->startRotatedImageUpdateConnection = connect(d->scanImage.get(), &ScanImage::startRotatedImageUpdate, [this]() { setBusy(true); });
            d->finishRotatedImageUpdateConnection = connect(d->scanImage.get(), &ScanImage::finishRotatedImageUpdate, [this]() { setBusy(false); });

            // this means that the settings will be initialized from the scanImage
            d->doRotate = -1;
        }
    }

    update();
}

QImage CutView::image() const
{
    return d->image;
}

void CutView::onRotatedImageChanged()
{
    if (d->scanImage != nullptr) {
        d->image = d->scanImage->rotatedImage();
        d->edges = std::make_unique<EdgeDetection>(d->image);
        connect(d->edges.get(), &EdgeDetection::hasAutoDetectionChanged, this, &CutView::hasAutoSelectionChanged);
        connect(d->edges.get(), &EdgeDetection::isAutoDetectionRunningChanged, this, &CutView::isAutoDetectionRunningChanged);

        emit hasAutoSelectionChanged();
        emit isAutoDetectionRunningChanged();

        d->edges->startAutoDetect();
        if (d->doRotate == -1) {
            setTopLeft(d->scanImage->topLeft());
            setTopRight(d->scanImage->topRight());
            setBottomRight(d->scanImage->bottomRight());
            setBottomLeft(d->scanImage->bottomLeft());
            d->doRotate = 0;
        } else if (d->doRotate == 1) {
            d->doRotate = 2;
        }
        update();
    }
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

/*
 * Copyright (c) 2018-2021 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include "ZoomImage.hxx"

#include <QtGui/QImage>
#include <QtGui/QPainter>

#include "Convert.hxx"
#include "ScanImage.hxx"

struct ZoomImage::Data {
    QPointF viewSize = {0.1, 0.1};
    QPointF center;
    QColor cross_color = Qt::red;
    QColor border_color = Qt::white;

    Scanner* scanner = nullptr;
};

ZoomImage::ZoomImage(QQuickItem* parent)
    : QQuickPaintedItem(parent), d(new Data) {}

ZoomImage::~ZoomImage() = default;

QPointF ZoomImage::viewSize() const
{
    return d->viewSize;
}

void ZoomImage::setViewSize(QPointF viewSize)
{
    // all ratio coordinates must be in [0,1]
    QPointF vs = QPointF{qBound<qreal>(0, viewSize.x(), 1), qBound<qreal>(0, viewSize.y(), 1)};
    if (vs != d->viewSize) {
        d->viewSize = vs;
        emit viewSizeChanged();
        update();
    }
}

QPointF ZoomImage::center() const
{
    return d->center;
}

void ZoomImage::setCenter(QPointF center)
{
    QPointF c = QPointF{qBound<qreal>(0, center.x(), 1), qBound<qreal>(0, center.y(), 1)};
    if (c != d->center) {
        d->center = c;
        emit centerChanged();
        update();
    }
}

QColor ZoomImage::borderColor() const
{
    return d->border_color;
}

void ZoomImage::setBorderColor(const QColor& color)
{
    if (color != d->border_color) {
        d->border_color = color;
        emit borderColorChanged();
        update();
    }
}

QColor ZoomImage::crossColor() const
{
    return d->cross_color;
}

void ZoomImage::setCrossColor(const QColor& color)
{
    if (color != d->cross_color) {
        d->cross_color = color;
        emit crossColorChanged();
        update();
    }
}

void ZoomImage::setScanner(Scanner* scanner)
{
    if (scanner != d->scanner) {
        if (d->scanner != nullptr) {
            disconnect(d->scanner, &Scanner::currentImageChanged, this, &ZoomImage::onImageChanged);
        }
        d->scanner = scanner;
        if (d->scanner != nullptr) {
            connect(d->scanner, &Scanner::currentImageChanged, this, &ZoomImage::onImageChanged);
        }

        emit scannerChanged();

        update();
    }
}

Scanner* ZoomImage::scanner() const
{
    return d->scanner;
}

void ZoomImage::paint(QPainter* p)
{
    auto scanImage = d->scanner->currentImage();
    if (scanImage == nullptr) return;

    auto image = scanImage->original();
    if (image.empty()) return;

    auto w = width();
    auto h = height();

    // get the source image
    auto iw = image.cols;
    auto ih = image.rows;

    // prepare the clipping path (a circle) so that the picture is only shown
    // within the preview area.
    auto linewidth = std::min(w, h) / 20;
    QPainterPath clip;
    clip.addEllipse(0, 0, width(), height());

    // draw the part of image with clipping, note that we use ration coordinates
    p->setClipPath(clip);

    // fill background with black
    p->fillRect(0, 0, w, h, Qt::black);

    // copy the part of the image to be shown
    QPointF pnt;
    double angle = 0;
    auto sizex = d->viewSize.x();
    auto sizey = d->viewSize.y();
    switch (scanImage->orientation()) {
        case 0:
            pnt = {d->center.x(), d->center.y()};
            angle = 0;
            break;
        case 1:
            pnt = {d->center.y(), 1 - d->center.x()};
            angle = 90;
            std::swap(sizex, sizey);
            break;
        case 2:
            pnt = {1 - d->center.x(), 1 - d->center.y()};
            angle = 180;
            break;
        case 3:
            pnt = {1 - d->center.y(), d->center.x()};
            angle = 270;
            std::swap(sizex, sizey);
            break;
    }

    auto zoom = cvMatToQImage(image)
                    .copy(QRect{qRound(iw * (pnt.x() - sizex / 2)),
                                qRound(ih * (pnt.y() - sizey / 2)),
                                qRound(iw * sizex),
                                qRound(ih * sizey)});

    if (angle != 0) {
        QTransform tr;
        tr.rotate(angle);
        zoom = zoom.transformed(tr);
    }

    p->drawImage(QRectF{0, 0, w, h}, zoom);

    // disable clipping for drawing the boundary
    p->setClipping(false);

    // draw the boundary
    QPen pen(d->border_color);
    pen.setWidth(linewidth);
    p->setPen(pen);
    p->drawEllipse(linewidth / 2, linewidth / 2, w - linewidth, h - linewidth);

    // draw the cross
    p->setPen(d->cross_color);
    p->drawLine(w / 2, h / 2 - h / 6, w / 2, h / 2 + h / 6);
    p->drawLine(w / 2 - w / 6, h / 2, w / 2 + w / 6, h / 2);
}

void ZoomImage::onImageChanged()
{
    update();
}

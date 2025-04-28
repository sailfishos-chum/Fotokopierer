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

#include "ScanImageView.hxx"

#include <QtGui/QImage>
#include <QtGui/QPainter>

#include "Scanner.hxx"

struct ScanImageView::Data {
    Scanner* scanner = nullptr;
    bool busy = false;
    qreal painted_width = 0;
    qreal painted_height = 0;
};

ScanImageView::ScanImageView(QQuickItem* parent)
    : QQuickPaintedItem(parent),
      d(new Data)
{
}

ScanImageView::~ScanImageView() = default;

Scanner* ScanImageView::scanner() const
{
    return d->scanner;
}

void ScanImageView::setScanner(Scanner* scanner)
{
    if (scanner != d->scanner) {
        if (d->scanner != nullptr) {
            disconnect(d->scanner, &Scanner::currentImageChanged, this, &ScanImageView::onNewImage);
        }
        d->scanner = scanner;
        if (d->scanner != nullptr) {
            connect(d->scanner, &Scanner::currentImageChanged, this, &ScanImageView::onNewImage);
            onNewImage();
        }
        emit scannerChanged();
    }
}

void ScanImageView::setBusy(bool busy)
{
    if (busy != d->busy) {
        d->busy = busy;
        emit busyChanged();
    }
}

bool ScanImageView::busy() const
{
    return d->busy;
}

qreal ScanImageView::paintedWidth() const
{
    return d->painted_width;
}

qreal ScanImageView::paintedHeight() const
{
    return d->painted_height;
}

void ScanImageView::setPaintedSize(qreal pwidth, qreal pheight)
{
    if (pwidth != d->painted_width || pheight != d->painted_height) {
        d->painted_width = pwidth;
        d->painted_height = pheight;
        emit paintedSizeChanged();
    }
}

void ScanImageView::paint(QPainter* painter)
{
    QImage image = this->image();

    if (image.width() > 0 && image.height() > 0) {
        auto wratio = static_cast<qreal>(width()) / image.width();
        auto hratio = static_cast<qreal>(height()) / image.height();
        auto ratio = std::min(wratio, hratio);
        d->painted_width = image.width() * ratio;
        d->painted_height = image.height() * ratio;
        emit paintedSizeChanged();
        painter->drawImage(QRectF{(width() - d->painted_width) / 2,
                                  (height() - d->painted_height) / 2,
                                  d->painted_width,
                                  d->painted_height},
                           image);
    }
}

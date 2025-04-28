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

#ifndef __FOTOKOPIERER_SCANIMAGEVIEW_HXX__
#define __FOTOKOPIERER_SCANIMAGEVIEW_HXX__

#include <QtQuick/QQuickPaintedItem>

#include <memory>

class Scanner;

class ScanImageView : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(qreal paintedWidth READ paintedWidth NOTIFY paintedSizeChanged)
    Q_PROPERTY(qreal paintedHeight READ paintedHeight NOTIFY paintedSizeChanged)

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

    Q_PROPERTY(Scanner* scanner READ scanner WRITE setScanner NOTIFY scannerChanged)

public:
    ScanImageView(QQuickItem* parent = nullptr);

    ~ScanImageView() override;

    Scanner* scanner() const;

    void setScanner(Scanner* scanner);

    qreal paintedWidth() const;

    qreal paintedHeight() const;

    bool busy() const;

    void paint(QPainter* painter) override;

signals:
    void scannerChanged();
    void busyChanged();
    void paintedSizeChanged();

protected:
    /// Change the busy marker.
    void setBusy(bool busy);

    void setPaintedSize(qreal pwidth, qreal pheight);

    /// Return the image to be drawn.
    virtual QImage image() const = 0;

protected slots:
    virtual void onNewImage() = 0;

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

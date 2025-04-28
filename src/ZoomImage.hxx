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

#ifndef __FOTOKOPIERER_ZOOMIMAGE_HXX__
#define __FOTOKOPIERER_ZOOMIMAGE_HXX__

#include <QtQuick/QQuickPaintedItem>

#include "Scanner.hxx"

/// A zoomed view of an image.
///
/// This Component displays a part of a image in a circle. It is used as a
/// preview image in the cut page.
///
/// The position and portion of the source image to be shown are specified in
/// ratio coordinates (i.e. center (x,y) refers to the pixel `(x *
/// image().width(), y * image().height())`). Using these kind of coordinates
/// makes ZoomImage independent of the actual resolution of the source image.
class ZoomImage : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QPointF viewSize READ viewSize WRITE setViewSize NOTIFY viewSizeChanged);
    Q_PROPERTY(QPointF center READ center WRITE setCenter NOTIFY centerChanged);
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged);
    Q_PROPERTY(QColor crossColor READ crossColor WRITE setCrossColor NOTIFY crossColorChanged);

    Q_PROPERTY(Scanner* scanner READ scanner WRITE setScanner NOTIFY scannerChanged)

public:
    explicit ZoomImage(QQuickItem* parent = nullptr);

    ~ZoomImage() override;

    /// Return the viewSize ratio.
    QPointF viewSize() const;

    /// Return the center point.
    QPointF center() const;

    /// Return the border color.
    QColor borderColor() const;

    /// Return the color of the cross.
    QColor crossColor() const;

    /// Return the image scanner.
    Scanner* scanner() const;

    void paint(QPainter* painter) override;

public slots:
    /// Set the view ratio.
    void setViewSize(QPointF viewSize);

    /// Set the center point.
    void setCenter(QPointF center);

    /// Set the border color.
    void setBorderColor(const QColor& color);

    /// Set the cross color.
    void setCrossColor(const QColor& color);

    /// Set the image scanner.
    void setScanner(Scanner* scanner);

signals:
    void viewSizeChanged();

    void centerChanged();

    void borderColorChanged();

    void crossColorChanged();

    /// The source image has been changed.
    void scannerChanged();

private slots:
    void onImageChanged();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

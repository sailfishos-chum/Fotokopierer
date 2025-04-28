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

#ifndef __FOTOKOPIERER_BASEMAGE_HXX__
#define __FOTOKOPIERER_BASEMAGE_HXX__

#include <QtCore/QScopedPointer>
#include <QtQuick/QQuickPaintedItem>

/// Base class for images to be shown in the app.
///
/// A BaseImage represents an image that can change, e.g. by representing some
/// filter applied to some original image and changing the filter parameter.
/// Hence a BaseImage has some signals to be emitted whenever the image is about
/// to be changed (because some parameter changed) and after the change is
/// complete (because it may take some time).
class BaseImage : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(qreal paintedWidth READ paintedWidth NOTIFY paintedSizeChanged)
    Q_PROPERTY(qreal paintedHeight READ paintedHeight NOTIFY paintedSizeChanged)
    Q_PROPERTY(BaseImage* source READ source WRITE setSource NOTIFY sourceChanged)

public:
    /// The default constructor.
    BaseImage(QQuickItem* parent = nullptr);

    ~BaseImage();

    void paint(QPainter* painter);

    /// Return the current image source.
    BaseImage* source();

    /// Return the current source image.
    virtual QImage sourceImage() const;

    /// Return the current image.
    QImage image() const;

    qreal paintedWidth() const;

    qreal paintedHeight() const;

public slots:
    /// Set the source image.
    void setSource(BaseImage* source);

protected slots:
    /// Set the result image.
    void setImage(const QImage& image);

signals:
    /// The source image has been changed.
    void sourceChanged();

    /// The image is about to be changed.
    void imageChanging();

    /// The image has been changed.
    void imageChanged();

    void paintedSizeChanged();

protected:
    /// Called to apply this image's transformation to `image`.
    ///
    /// This method may be called asynchronously.
    virtual QImage transform(const QImage& image) = 0;

protected slots:
    /// Called to trigger a new transformation when the source image had been changed.
    virtual void updateImage();

private:
    struct Data;
    QScopedPointer<Data> d;

    friend class TransformWorker;
};

#endif

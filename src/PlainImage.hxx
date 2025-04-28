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

#ifndef __FOTOKOPIERER_PLAINIMAGE_HXX__
#define __FOTOKOPIERER_PLAINIMAGE_HXX__

#include "BaseImage.hxx"

#include <QtCore/QScopedPointer>

/// An image just representing a picture.
class PlainImage : public BaseImage
{
    Q_OBJECT

    Q_PROPERTY(bool scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(bool maxSize READ maxSize WRITE setMaxSize NOTIFY maxSizeChanged)

public:
    PlainImage();

    ~PlainImage();

    QImage sourceImage() const;

    bool scale() const;

    int maxSize() const;

    Q_INVOKABLE void loadFile(const QString& file_name);

public slots:
    void setScale(bool enabled);

    void setMaxSize(int maxSize);

signals:
    /// Emitted if loading a file failed.
    void loadFailed();

    void scaleChanged();

    void maxSizeChanged();

protected:
    QImage transform(const QImage& image);

private:
    struct Data;
    QScopedPointer<Data> d;
};

#endif

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

#ifndef __FOTOKOPIERER_ROTIMAGE_HXX__
#define __FOTOKOPIERER_ROTIMAGE_HXX__

#include "BaseImage.hxx"

/// An image that can be rotated.
class RotImage : public BaseImage
{
public:
    Q_OBJECT

    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)

public:
    explicit RotImage(QQuickItem* parent = nullptr);

    ~RotImage() override;

    int orientation() const;

public slots:
    void setOrientation(int orientation);

signals:
    void orientationChanged();

protected:
    QImage transform(const QImage& image) override;

private:
    int orientation_;
};

#endif

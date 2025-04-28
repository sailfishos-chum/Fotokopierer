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

#ifndef __FOTOKOPIERER_COLORIZECHOOSER_HXX__
#define __FOTOKOPIERER_COLORIZECHOOSER_HXX__

#include <QtQuick/QQuickPaintedItem>
#include <memory>
#include <opencv2/core.hpp>

class ColorizeView;

class ColorizeChooser : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int lattice READ lattice WRITE setLattice NOTIFY latticeChanged);
    Q_PROPERTY(int blackLevel READ blackLevel WRITE setBlackLevel NOTIFY blackLevelChanged);

public:
    ColorizeChooser(QQuickItem* parent = nullptr);

    ~ColorizeChooser() override;

    int lattice() const;

    void setLattice(int lattice);

    int blackLevel() const;

    void setBlackLevel(int blackLevel);

    void updateImage(const cv::Mat& image, const cv::Mat& mask);

    void paint(QPainter* painter) override;

    qreal colorAngle(int which) const;

public slots:
    void setColorAngle(int which, qreal angle);

signals:
    void colorizeViewChanged();
    void latticeChanged();
    void blackLevelChanged();
    void colorAnglesChanged();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

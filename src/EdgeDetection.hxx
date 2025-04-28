/*
 * Copyright (c) 2020, 2021 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __FOTOKOPIERER_EDGEDETECTION_HXX__
#define __FOTOKOPIERER_EDGEDETECTION_HXX__

#include <memory>
#include <vector>

#include <Qt>

class QImage;
class QLineF;
class QPointF;

class EdgeList
{
public:
    EdgeList(const EdgeList&) = delete;

    EdgeList(EdgeList&&) noexcept;

    EdgeList& operator=(const EdgeList&) = delete;

    EdgeList& operator=(EdgeList&&) noexcept;

    ~EdgeList();

    void setCannyMinValue(int minVal);

    int cannyMinValue() const;

    void setCannyMaxValue(int maxVal);

    int cannyMaxValue() const;

    void setBlurRadius(int radius);

    int blurRadius() const;

    void setContrastFactor(qreal factor);

    qreal contrastFactor() const;

    QImage image() const;
    QImage gray_image() const;
    QImage bw_image() const;

    std::vector<QLineF> vertical_lines() const;

    std::vector<QLineF> horizontal_lines() const;

    std::vector<QPointF> points() const;

    QPointF best_topLeft() const;
    QPointF best_topRight() const;
    QPointF best_bottomLeft() const;
    QPointF best_bottomRight() const;

    void update();

    void corner_points(int top, int bottom, int left, int right, QPointF& topleft, QPointF& topright, QPointF& bottomright, QPointF& bottomleft);

    static EdgeList detect_in_image(const QImage& image);

private:
    struct Data;
    std::unique_ptr<Data> d;

    EdgeList(std::unique_ptr<Data>&& d);
};

#endif

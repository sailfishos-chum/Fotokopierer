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

    /// Set the lower threshold value for canny edge detection.
    void setCannyMinValue(int minVal);

    /// Return the lower threshold value for canny edge detection.
    int cannyMinValue() const;

    /// Set the upper threshold value for canny edge detection.
    void setCannyMaxValue(int maxVal);

    /// Return the upper threshold value for canny edge detection.
    int cannyMaxValue() const;

    /// Set the radius for Gaussian blur preprocessing.
    void setBlurRadius(int radius);

    /// Return the radius for Gaussian blur preprocessing.
    int blurRadius() const;

    /// Set the contrast scaling factor before running canny edge detection.
    void setContrastFactor(qreal factor);

    /// Return the contrast scaling factor before running canny edge detection.
    qreal contrastFactor() const;

    /// Rerun the edge detection after changing some parameter.
    void update();

    /// Return the original image.
    QImage image() const;

    /// Return the gray image.
    QImage gray_image() const;

    /// Return the result image of canny edge detection.
    QImage bw_image() const;

    /// Return the list of detected vertical lines.
    std::vector<QLineF> vertical_lines() const;

    /// Return the list of detected horizontal lines.
    std::vector<QLineF> horizontal_lines() const;

    /// Return a list of potential candidate points.
    std::vector<QPointF> points() const;

    /// Return the top left point of the detected rectangle.
    QPointF topLeft() const;

    /// Return the top right point of the detected rectangle.
    QPointF topRight() const;

    /// Return the bottom left point of the detected rectangle.
    QPointF bottomLeft() const;

    /// Return the bottom right point of the detected rectangle.
    QPointF bottomRight() const;

    /// Return a new edge list for the given image.
    static EdgeList detect_in_image(const QImage& image);

private:
    struct Data;
    std::unique_ptr<Data> d;

    EdgeList(std::unique_ptr<Data>&& d);
};

#endif

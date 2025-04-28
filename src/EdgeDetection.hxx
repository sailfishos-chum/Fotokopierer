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
#include <QtCore/QObject>

class QImage;
class QLineF;
class QPointF;

class EdgeDetection : public QObject
{
    Q_OBJECT

public:
    EdgeDetection(QObject* parent = nullptr);

    /// Initialize edge detection for the given image.
    EdgeDetection(const QImage& image, QObject* parent = nullptr);

    EdgeDetection(const EdgeDetection&) = delete;

    EdgeDetection(EdgeDetection&&) noexcept = delete;

    EdgeDetection& operator=(const EdgeDetection&) = delete;

    EdgeDetection& operator=(EdgeDetection&&) noexcept = delete;

    ~EdgeDetection();

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

    /// Set the discretization size (in pixels) for snappy-edge selection.
    void setSnappySize(std::size_t snappy_size);

    /// Return the discretization size (in pixels) for snappy-edge selection.
    size_t snappySize() const;

    /// Fix current lines as "default" non-snappy lines.
    ///
    /// If one of the edge mid points is changed and the current pixel
    /// is non-snappy, use the latest non-snappy lines. Otherwise the snappy line
    /// for the current pixel is used.
    ///
    /// Should be called before any edge mid point interaction.
    void fixNonSnappyEdges();

    /// Run the edge detection.
    ///
    /// Must be called after changing a parameter.
    void autoDetect();

    /// Select everything.
    void selectAll();

    /// Return the width in pixels of the current image.
    int width() const;

    /// Return the height in pixels of the current image.
    int height() const;

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

    /// Set the top left point of the rectangle.
    void setTopLeft(const QPointF& tl);

    /// Return the top left point of the detected rectangle.
    QPointF topLeft() const;

    /// Set the top right point of the rectangle.
    void setTopRight(const QPointF& tr);

    /// Return the top right point of the detected rectangle.
    QPointF topRight() const;

    /// Set the bottom left point of the rectangle.
    void setBottomLeft(const QPointF& bl);

    /// Return the bottom left point of the detected rectangle.
    QPointF bottomLeft() const;

    /// Set the bottom right point of the rectangle.
    void setBottomRight(const QPointF& br);

    /// Return the bottom right point of the detected rectangle.
    QPointF bottomRight() const;

    /// Set the middle control point of the top edge.
    void setTopPoint(const QPointF& p);

    /// Return the middle control point of the top edge.
    QPointF topPoint() const;

    /// Set the middle control point of the bottom edge.
    void setBottomPoint(const QPointF& p);

    /// Return the middle control point of the bottom edge.
    QPointF bottomPoint() const;

    /// Set the middle control point of the left edge.
    void setLeftPoint(const QPointF& p);

    /// Return the middle control point of the left edge.
    QPointF leftPoint() const;

    /// Set the middle control point of the right edge.
    void setRightPoint(const QPointF& p);

    /// Return the middle control point of the right edge.
    QPointF rightPoint() const;

    /// Return a new edge list for the given image.
    static EdgeDetection* detect_in_image(const QImage& image, QObject* parent = nullptr);

private:
    struct Data;
    std::unique_ptr<Data> d;

    EdgeDetection(std::unique_ptr<Data>&& d);
};

#endif

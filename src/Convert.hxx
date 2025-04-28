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

/// \file
/// Functions to convert between OpenCV's cv::Mat and Qt's QImage and QPixmap.
///
///   Andy Maloney <asmaloney@gmail.com>
///   https://asmaloney.com/2013/11/code/converting-between-cvmat-and-qimage-or-qpixmap

class QImage;
class QPixmap;

namespace cv
{
class Mat;
}

/// Convert a cv::Mat to QImage.
QImage cvMatToQImage(const cv::Mat& inMat);

/// Convert QImage to cv::Mat.
///
/// If inImage exists for the lifetime of the resulting cv::Mat, pass false to
/// inCloneImageData to share inImage's data with the cv::Mat directly.
///
/// \note `Format_RGB888` is an exception since we need to use a local
/// QImage
/// and thus must clone the data regardless
/// \note This does not cover all cases - it should be easy to add new
/// ones as required.
cv::Mat QImageToCvMat(const QImage& inImage, bool inCloneImageData = true);

/// Convert a cv::Mat to QPixmap.
QPixmap cvMatToQPixmap(const cv::Mat& inMat);

/// Convert QPixmap to cv::Mat.
///
/// If inPixmap exists for the lifetime of the resulting cv::Mat, pass false to
/// inCloneImageData to share inPixmap's data with the cv::Mat directly
///
/// \note Format_RGB888 is an exception since we need to use a local
/// QImage and thus must clone the data regardless
cv::Mat QPixmapToCvMat(const QPixmap& inPixmap, bool inCloneImageData = true);

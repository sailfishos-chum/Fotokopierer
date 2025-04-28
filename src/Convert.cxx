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

#include "Convert.hxx"

#include <QtCore/QDebug>
#include <QtGui/QImage>
#include <QtGui/QPixmap>

#include <opencv2/imgproc/imgproc.hpp>

// NOTE: This does not cover all cases - it should be easy to add new ones as
// required.
QImage cvMatToQImage(const cv::Mat &inMat)
{
    switch (inMat.type()) {
        // 8-bit, 4 channel
        case CV_8UC4: {
            QImage image(inMat.data,
                         inMat.cols,
                         inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_ARGB32);

            return image;
        }

        // 8-bit, 3 channel
        case CV_8UC3: {
            QImage image(inMat.data,
                         inMat.cols,
                         inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_RGB888);

            return image.rgbSwapped();
        }

        // 8-bit, 1 channel
        case CV_8UC1: {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
            QImage image(inMat.data,
                         inMat.cols,
                         inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_Grayscale8);
#else
            static QVector<QRgb> sColorTable;

            // only create our color table the first time
            if (sColorTable.isEmpty()) {
                sColorTable.resize(256);

                for (int i = 0; i < 256; ++i) {
                    sColorTable[i] = qRgb(i, i, i);
                }
            }

            QImage image(inMat.data,
                         inMat.cols,
                         inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_Indexed8);

            image.setColorTable(sColorTable);
#endif

            return image;
        }

        default:
            qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not "
                          "handled in switch:"
                       << inMat.type();
            break;
    }

    return QImage();
}

inline cv::Mat QImageToCvMat(const QImage &inImage, bool inCloneImageData)
{
    switch (inImage.format()) {
        // 8-bit, 4 channel
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied: {
            cv::Mat mat(inImage.height(),
                        inImage.width(),
                        CV_8UC4,
                        const_cast<uchar *>(inImage.bits()),
                        static_cast<size_t>(inImage.bytesPerLine()));

            return (inCloneImageData ? mat.clone() : mat);
        }

        // 8-bit, 3 channel
        case QImage::Format_RGB32:
        case QImage::Format_RGB888: {
            if (!inCloneImageData) {
                qWarning() << "ASM::QImageToCvMat() - Conversion requires "
                              "cloning because we use a temporary QImage";
            }

            QImage swapped = inImage;

            if (inImage.format() == QImage::Format_RGB32) {
                swapped = swapped.convertToFormat(QImage::Format_RGB888);
            }

            swapped = swapped.rgbSwapped();

            return cv::Mat(swapped.height(),
                           swapped.width(),
                           CV_8UC3,
                           const_cast<uchar *>(swapped.bits()),
                           static_cast<size_t>(swapped.bytesPerLine()))
                .clone();
        }

        // 8-bit, 1 channel
        case QImage::Format_Grayscale8: {
            cv::Mat mat(inImage.height(),
                        inImage.width(),
                        CV_8UC1,
                        const_cast<uchar *>(inImage.bits()),
                        static_cast<size_t>(inImage.bytesPerLine()));

            return (inCloneImageData ? mat.clone() : mat);
        }

        default:
            qWarning() << "ASM::QImageToCvMat() - QImage format not handled in switch:"
                       << inImage.format();
            break;
    }

    return cv::Mat();
}

QPixmap cvMatToQPixmap(const cv::Mat &inMat)
{
    return QPixmap::fromImage(cvMatToQImage(inMat));
}

cv::Mat QPixmapToCvMat(const QPixmap &inPixmap, bool inCloneImageData)
{
    return QImageToCvMat(inPixmap.toImage(), inCloneImageData);
}

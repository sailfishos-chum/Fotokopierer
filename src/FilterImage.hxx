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

#ifndef __FOTOKOPIERER_FILTERIMAGE_HXX__
#define __FOTOKOPIERER_FILTERIMAGE_HXX__

#include <QtQuick/QQuickPaintedItem>

#include <memory>

#include "ScanImage.hxx"

class Filter;

class FilterImage : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(qreal paintedWidth READ paintedWidth NOTIFY paintedSizeChanged)
    Q_PROPERTY(qreal paintedHeight READ paintedHeight NOTIFY paintedSizeChanged)

    Q_PROPERTY(ScanImage::FilterType filterType READ filterType WRITE setFilterType NOTIFY
                   filterTypeChanged)
    Q_PROPERTY(ScanImage* image READ image WRITE setImage NOTIFY imageChanged)

public:
    FilterImage(QQuickItem* parent = nullptr);

    ~FilterImage() override;

    qreal paintedWidth() const;

    qreal paintedHeight() const;

    ScanImage::FilterType filterType() const;

    ScanImage* image() const;

    void paint(QPainter* painter) override;

    QVariant filter() const;

public slots:
    void setFilterType(ScanImage::FilterType type);

    void setImage(ScanImage* image);

private:
    void updateFilter();

private slots:
    void update();

signals:
    void paintedSizeChanged();

    void filterTypeChanged();

    void imageChanged();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

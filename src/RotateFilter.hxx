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

#ifndef __FOTOKOPIERER_ROTATEFILTER_HXX__
#define __FOTOKOPIERER_ROTATEFILTER_HXX__

#include "Filter.hxx"

class RotateFilter : public Filter
{
    Q_OBJECT

    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)

public:
    explicit RotateFilter(ScanImage* image);

    explicit RotateFilter(ScanImage* image, const std::shared_ptr<Filter>& previous_filter);

    ~RotateFilter() override;

    QJsonObject saveJson() const override;

    void loadJson(QJsonObject& object) override;

    QImage apply(QImage&& image) override;

    int orientation() const;

public slots:
    void setOrientation(int orientation);

signals:
    void orientationChanged();

private:
    int orientation_;
};

#endif

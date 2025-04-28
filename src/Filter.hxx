/*
 * Copyright (c) 2018, 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __FOTOKOPIERER_FILTER_HXX__
#define __FOTOKOPIERER_FILTER_HXX__

#include <QtCore/QObject>

#include <memory>

class Scanner;

class Filter : public QObject
{
    Q_OBJECT

public:
    explicit Filter(Scanner* image);

    Filter(Scanner* image, Filter* previous_filter);

    Filter(const Filter&) = delete;
    Filter(Filter&&) = delete;
    Filter& operator=(const Filter&) = delete;
    Filter& operator=(Filter&&) = delete;

    ~Filter() override;

    Scanner* image();

    QImage filteredImage();

    /// Reset filter to default settings.
    virtual void reset() = 0;

    /// Return the name of this filter.
    ///
    /// The name should be unique among all filter types.
    virtual QString name() const = 0;

    virtual QJsonObject saveJson() const = 0;

    virtual void loadJson(const QJsonObject& object) = 0;

    virtual QImage apply(QImage&& image) = 0;

signals:
    void filterChanged();

protected:
    Filter* previous_filter_;
};

#endif

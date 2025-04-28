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

#ifndef __FOTOKOPIERER_CUTFILTER_HXX__
#define __FOTOKOPIERER_CUTFILTER_HXX__

#include "Filter.hxx"

#include <QtCore/QPointF>

#include <memory>

class CutFilter : public Filter
{
    Q_OBJECT

    Q_PROPERTY(QPointF topLeft READ topLeft WRITE setTopLeft NOTIFY topLeftChanged)
    Q_PROPERTY(QPointF topRight READ topRight WRITE setTopRight NOTIFY topRightChanged)
    Q_PROPERTY(QPointF bottomRight READ bottomRight WRITE setBottomRight NOTIFY bottomRightChanged)
    Q_PROPERTY(QPointF bottomLeft READ bottomLeft WRITE setBottomLeft NOTIFY bottomLeftChanged)

public:
    explicit CutFilter(Scanner* image);

    CutFilter(Scanner* image, Filter* previous_filter);

    CutFilter(const CutFilter&) = delete;
    CutFilter(CutFilter&&) = delete;
    CutFilter& operator=(const CutFilter&) = delete;
    CutFilter& operator=(CutFilter&&) = delete;

    ~CutFilter() override;

    void reset() override;

    QString name() const override;

    QJsonObject saveJson() const override;

    void loadJson(const QJsonObject& object) override;

    QImage apply(QImage&& image) override;

    QPointF topLeft() const;

    void setTopLeft(QPointF topleft);

    QPointF topRight() const;

    void setTopRight(QPointF topright);

    QPointF bottomRight() const;

    void setBottomRight(QPointF bottomright);

    QPointF bottomLeft() const;

    void setBottomLeft(QPointF bottomleft);

    /// Return the auto-detected cut box.
    ///
    /// The method returns a list of four points (topleft, topright,
    /// bottomright, bottomleft).
    Q_INVOKABLE QVariantList autoDetectCutRect();

public slots:
    /// Apply the current cut area to the image.
    ///
    /// The function returns true if the action has been successful. It returns
    /// false if the current box is invalid (i.e. non-convex).
    bool updateCut();

signals:
    void topLeftChanged();
    void topRightChanged();
    void bottomRightChanged();
    void bottomLeftChanged();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

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

#ifndef __FOTOKOPIERER_CUTVIEW_HXX__
#define __FOTOKOPIERER_CUTVIEW_HXX__

#include <memory>

#include "ScanImageView.hxx"

class Scanner;

class CutView : public ScanImageView
{
    Q_OBJECT

    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)

    Q_PROPERTY(QPointF topLeft READ topLeft WRITE setTopLeft NOTIFY topLeftChanged)
    Q_PROPERTY(QPointF topRight READ topRight WRITE setTopRight NOTIFY topRightChanged)
    Q_PROPERTY(QPointF bottomRight READ bottomRight WRITE setBottomRight NOTIFY bottomRightChanged)
    Q_PROPERTY(QPointF bottomLeft READ bottomLeft WRITE setBottomLeft NOTIFY bottomLeftChanged)

    Q_PROPERTY(QPointF top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(QPointF bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(QPointF left READ left WRITE setLeft NOTIFY leftChanged)
    Q_PROPERTY(QPointF right READ right WRITE setRight NOTIFY rightChanged)

    Q_PROPERTY(bool hasAutoSelection READ hasAutoSelection NOTIFY hasAutoSelectionChanged)
    Q_PROPERTY(bool isAutoDetectionRunning READ isAutoDetectionRunning NOTIFY isAutoDetectionRunningChanged)

public:
    CutView(QQuickItem* parent = nullptr);

    ~CutView() override;

    int orientation() const;

    void setOrientation(int orientation);

    QPointF topLeft() const;

    void setTopLeft(QPointF topleft);

    QPointF topRight() const;

    void setTopRight(QPointF topright);

    QPointF bottomRight() const;

    void setBottomRight(QPointF bottomright);

    QPointF bottomLeft() const;

    void setBottomLeft(QPointF bottomleft);

    void setTop(QPointF top);

    QPointF top() const;

    void setBottom(QPointF bottom);

    QPointF bottom() const;

    void setLeft(QPointF left);

    QPointF left() const;

    void setRight(QPointF right);

    QPointF right() const;

    bool hasAutoSelection() const;

    bool isAutoDetectionRunning() const;

    void paint(QPainter* painter) override;

public:
    Q_INVOKABLE void rotateLeft();

    Q_INVOKABLE void rotateRight();

    Q_INVOKABLE void selectAll();

    Q_INVOKABLE void selectAuto();

    /// Update the snappy edges.
    Q_INVOKABLE void updateSnappyEdges();

    /// Apply the current cut to the scan image.
    Q_INVOKABLE void apply();

signals:
    void scannerChanged();

    void orientationChanged();

    void topLeftChanged();
    void topRightChanged();
    void bottomRightChanged();
    void bottomLeftChanged();

    void topChanged();
    void bottomChanged();
    void leftChanged();
    void rightChanged();

    void rotationChanged();

    void hasAutoSelectionChanged();
    void isAutoDetectionRunningChanged();

protected:
    QImage image() const override;

protected slots:
    void onNewImage() override;

private slots:
    void onOrientationChanged();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

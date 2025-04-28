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

#include "Convert.hxx"
#include "EdgeDetection.hxx"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSlider>
#include <iostream>

using namespace cv;
using namespace std;

class Images : public QWidget
{
public:
    Images(const QImage& img, QWidget* parent = nullptr)
        : QWidget(parent), img_orig_(img), which_(-1)
    {
        updateEdges();
    }

    void paintEvent(QPaintEvent* ev) override
    {
        QPainter p(this);

        if (which_ >= 0 && which_ < 4) {
            QImage img;
            switch (which_) {
                case 0: img = img_orig_; break;
                case 1: img = img_gray_; break;
                case 2: img = img_bw_; break;
                case 3: img = img_result_; break;
            }
            p.drawImage(QRect(0, 0, width(), height()), img);
            return;
        }

        auto margin = 5;
        auto img_w = (width() - 4 * margin) / 2;
        auto img_h = (height() - 4 * margin) / 2;

        QSize target;
        if (img_w * img_orig_.height() > img_orig_.width() * img_h) {
            target = {img_orig_.width() * img_h / img_orig_.height(), img_h};
        } else {
            target = {img_w, img_orig_.height() * img_w / img_orig_.width()};
        }

        p.drawImage(QRect(width() / 4 - target.width() / 2, height() / 4 - target.height() / 2, target.width(), target.height()), img_orig_);
        p.drawImage(QRect(3 * width() / 4 - target.width() / 2, height() / 4 - target.height() / 2, target.width(), target.height()), img_gray_);
        p.drawImage(QRect(width() / 4 - target.width() / 2, 3 * height() / 4 - target.height() / 2, target.width(), target.height()), img_bw_);
        p.drawImage(QRect(3 * width() / 4 - target.width() / 2, 3 * height() / 4 - target.height() / 2, target.width(), target.height()), img_result_);
    }

    void updateEdges()
    {
        QImage image = img_orig_;
        auto edges = EdgeList::detect_in_image(image);
        edges.setCannyMinValue(minValue_);
        edges.setCannyMaxValue(maxValue_);
        edges.update();

        QPointF tl, tr, br, bl;

        auto w = image.width();
        auto h = image.height();

        QPainter p(&image);

        p.setPen(QPen(Qt::green, 10));
        for (auto& l : edges.horizontal_lines()) {
            p.drawLine(l);
        }

        p.setPen(QPen(Qt::blue, 10));
        for (auto& l : edges.vertical_lines()) {
            p.drawLine(l);
        }

        p.setPen(QPen(Qt::red, 20));
        p.setBrush(Qt::red);
        for (auto pnt : edges.points()) {
            p.drawEllipse(pnt, 20, 20);
        }

        p.drawLine(edges.topLeft(), edges.topRight());
        p.drawLine(edges.topLeft(), edges.bottomLeft());
        p.drawLine(edges.bottomRight(), edges.topRight());
        p.drawLine(edges.bottomLeft(), edges.bottomRight());

        img_gray_ = edges.gray_image();
        img_bw_ = edges.bw_image();
        img_result_ = image;

        update();
    }

    void keyPressEvent(QKeyEvent* ev) override
    {
        switch (ev->key()) {
            case Qt::Key_1: which_ = which_ != 0 ? 0 : -1; break;
            case Qt::Key_2: which_ = which_ != 1 ? 1 : -1; break;
            case Qt::Key_3: which_ = which_ != 2 ? 2 : -1; break;
            case Qt::Key_4: which_ = which_ != 3 ? 3 : -1; break;
            case Qt::Key_0:
            case Qt::Key_Escape: which_ = -1; break;
        }
        update();
    }

public slots:
    void setMinValue(int minValue)
    {
        minValue_ = minValue;
        updateEdges();
    }

    void setMaxValue(int maxValue)
    {
        maxValue_ = maxValue;
        updateEdges();
    }

private:
    static void draw(QPainter& p, const QImage& img, const QPointF& x, const QPointF& y)
    {
        auto w = img.width();
        auto h = img.height();
        p.drawLine(x.x() * w, x.y() * h, y.x() * w, y.y() * h);
    }

private:
    QImage img_orig_;
    QImage img_gray_;
    QImage img_bw_;
    QImage img_result_;
    int which_;

    int minValue_ = 10;
    int maxValue_ = 50;
};

class MainWindow : public QWidget
{
public:
    MainWindow(const QImage& img)
        : QWidget()
    {
        auto vbox = new QVBoxLayout(this);
        imgs_ = new Images(img, this);
        vbox->addWidget(imgs_);

        auto* minSlider = addSlider(vbox);
        connect(minSlider, &QSlider::valueChanged, imgs_, &Images::setMinValue);

        auto* maxSlider = addSlider(vbox);
        connect(maxSlider, &QSlider::valueChanged, imgs_, &Images::setMaxValue);

        setLayout(vbox);
    }

    void keyPressEvent(QKeyEvent* ev) override
    {
        imgs_->keyPressEvent(ev);
    }

private:
    QSlider* addSlider(QVBoxLayout* layout)
    {
        auto l = new QHBoxLayout();
        auto lbl = new QLabel("Bla", this);
        l->addWidget(lbl);

        auto slider = new QSlider(Qt::Horizontal, this);
        slider->setTickPosition(QSlider::TicksAbove);
        slider->setTracking(false);
        slider->setMinimum(1);
        slider->setMaximum(100);
        l->addWidget(slider);

        connect(slider, &QSlider::sliderMoved, [lbl](int v) { lbl->setText(QString::number(v)); });

        layout->addLayout(l);

        return slider;
    }

private:
    Images* imgs_;
};

int main(int argc, char** argv)
{
    if (argc < 2) return -1;

    QApplication app(argc, argv);

    const char* filename = argv[1];
    auto image = QImage(filename);

    auto win = new MainWindow(image);
    win->resize(800, 600);
    win->show();

    return app.exec();
}

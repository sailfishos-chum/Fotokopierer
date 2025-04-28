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

#ifndef __FOTOKOPIERER_BASEIMAGETRANSFORMWORKER_HXX__
#define __FOTOKOPIERER_BASEIMAGETRANSFORMWORKER_HXX__

#include <QtCore/QObject>

class BaseImage;

class BaseImageTransformWorker : public QObject
{
    Q_OBJECT

public:
    BaseImageTransformWorker(BaseImage* base_image);

public slots:
    void doTransform(const QImage& image);

signals:
    void resultReady(const QImage& image);

private:
    BaseImage* base_image_;
};

#endif

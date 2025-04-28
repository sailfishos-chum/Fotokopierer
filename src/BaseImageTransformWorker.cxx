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

#include "BaseImageTransformWorker.hxx"

#include "BaseImage.hxx"

#include <QtGui/QImage>

BaseImageTransformWorker::BaseImageTransformWorker(BaseImage* base_image) : base_image_(base_image)
{
}

void BaseImageTransformWorker::doTransform(const QImage& image)
{
    emit resultReady(base_image_->transform(image));
}

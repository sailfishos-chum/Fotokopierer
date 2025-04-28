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

#include "AsyncImage.hxx"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFutureWatcher>
#include <QtGui/QImage>

#include <functional>

struct AsyncImage::Data {
    bool thread_running = false;
    bool restart_thread = true;
    QFutureWatcher<QImage> result_image;
};

AsyncImage::AsyncImage(QQuickItem* parent) : BaseImage(parent), d(new Data)
{
    connect(
        &d->result_image, &QFutureWatcher<QImage>::finished, this, &AsyncImage::finishTransform);
}

AsyncImage::~AsyncImage() = default;

void AsyncImage::updateImage()
{
    emit imageChanging();
    if (!d->result_image.isRunning()) {
        d->restart_thread = false;
        d->result_image.setFuture(
            QtConcurrent::run(std::mem_fn(&AsyncImage::transform), this, sourceImage()));
    } else {
        d->restart_thread = true;
    }
}

void AsyncImage::finishTransform()
{
    setImage(d->result_image.result());
    if (d->restart_thread) {
        d->restart_thread = false;
        d->result_image.setFuture(
            QtConcurrent::run(std::mem_fn(&AsyncImage::transform), this, sourceImage()));
    }
}

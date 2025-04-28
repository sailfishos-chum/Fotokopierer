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

#include <QtCore/QThread>
#include <QtGui/QImage>

class AsyncImageTask : public QObject
{
    Q_OBJECT

public:
    AsyncImageTask(AsyncImage* image) : image_(image) {}

public slots:
    void run(const QImage& source) { emit resultReady(image_->transform(source)); }

signals:
    void resultReady(const QImage& image);

private:
    AsyncImage* image_;
};

static QThread* transformThread()
{
    static QThread thread;
    static bool started = false;
    if (!started) {
        started = true;
        thread.start();
    }
    return &thread;
}

struct AsyncImage::Data {
    AsyncImageTask* task = nullptr;
    bool thread_running = false;
    bool restart_thread = true;
};

AsyncImage::AsyncImage() : d(new Data)
{
    d->task = new AsyncImageTask(this);
    d->task->moveToThread(transformThread());
    connect(this, &AsyncImage::startTransform, d->task, &AsyncImageTask::run);
    connect(d->task, &AsyncImageTask::resultReady, this, &AsyncImage::finishTransform);
}

AsyncImage::~AsyncImage()
{
    d->task->deleteLater();
}

void AsyncImage::updateImage()
{
    emit imageChanging();
    if (!d->thread_running) {
        d->restart_thread = false;
        d->thread_running = true;
        emit startTransform(sourceImage());
    } else {
        d->restart_thread = true;
    }
}

void AsyncImage::finishTransform(const QImage& image)
{
    setImage(image);
    if (d->restart_thread) {
        d->restart_thread = false;
        emit startTransform(sourceImage());
    } else {
        d->thread_running = false;
    }
}

#include "AsyncImage.moc"

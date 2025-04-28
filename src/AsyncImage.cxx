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

struct AsyncImage::Data {
    QThread thread;
    bool thread_running = false;
    bool restart_thread = true;

    ~Data()
    {
        thread.quit();
        thread.wait();
    }
};

AsyncImage::AsyncImage() : d(new Data)
{
    auto task = new AsyncImageTask(this);
    task->moveToThread(&d->thread);
    connect(&d->thread, &QThread::finished, task, &QObject::deleteLater);
    connect(this, &AsyncImage::startTransform, task, &AsyncImageTask::run);
    connect(task, &AsyncImageTask::resultReady, this, &AsyncImage::finishTransform);
    d->thread.start();
}

AsyncImage::~AsyncImage() = default;

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

/*
 * Copyright (c) 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include "Fotokopierer.hxx"

#include <QtCore/QDateTime>
#include <QtCore/QLineF>
#include <QtCore/QSize>
#include <QtCore/QStandardPaths>
#include <QtMultimedia/QCameraImageCapture>

const QString ApplicationName = QStringLiteral("Fotokopierer");

const QString ApplicationVersion = QStringLiteral(FOTOKOPIERER_VERSION);

const QString FilenameFormat = QStringLiteral("yyyy_MM_dd-HH_mm_ss");

const QString DocumentRoot = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
                             QStringLiteral("/Fotokopierer");

static const int MaxResolutionWidth = 4000;
static const int MaxResolutionHeight = 3000;

QDir getDocumentDirectory()
{
    auto dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

    if (!dir.exists()) {
        if (!dir.mkdir(QStringLiteral("."))) {
            return {};
        }
    }

    if (!dir.cd(ApplicationName)) {
        if (!dir.mkdir(ApplicationName) || !dir.cd(ApplicationName)) {
            return {};
        }
    }

    return dir;
}

bool Fotokopierer::isConvex(QPointF x1, QPointF x2, QPointF x3, QPointF x4)
{
    return QLineF(x1, x3).intersect(QLineF(x2, x4), nullptr) == QLineF::BoundedIntersection;
}

QString Fotokopierer::newImagePath()
{
    QDir raw = getDocumentDirectory();
    if (!raw.cd(QStringLiteral(".raw"))) {
        raw.mkpath(QStringLiteral(".raw"));
        if (!raw.cd(QStringLiteral(".raw"))) {
            return {};
        }
    }

    if (!raw.exists(QStringLiteral(".nomedia"))) {
        QFile nomedia(raw.filePath(QStringLiteral(".nomedia")));
        nomedia.open(QIODevice::WriteOnly);
    }

    return raw.absoluteFilePath(QStringLiteral("%1.jpg").arg(QDateTime::currentDateTime().toString(FilenameFormat)));
}

void cleanupImageDirectory()
{
    auto raw = getDocumentDirectory();
    if (raw.cd(QStringLiteral(".raw"))) {
        for (auto& path : raw.entryList({QStringLiteral("*.jpg")}, QDir::Files)) {
            QFile(raw.absoluteFilePath(path)).remove();
        }
    }
}

QString Fotokopierer::applicationName() const
{
    return ApplicationName;
}

QString Fotokopierer::applicationVersion() const
{
    return ApplicationVersion;
}

QString Fotokopierer::author() const
{
    return QStringLiteral(FOTOKOPIERER_AUTHOR);
}

QString Fotokopierer::licenseTitle() const
{
    return QStringLiteral("GNU GPLv3");
}

QString Fotokopierer::podofoVersion() const
{
    return QStringLiteral(PODOFO_VERSION);
}

QString Fotokopierer::opencvVersion() const
{
    return QStringLiteral(OPENCV_VERSION);
}

QSize Fotokopierer::defaultResolution(QObject* capture) const
{
    if (capture == nullptr) {
        return {};
    }

    auto captures = capture->findChildren<QCameraImageCapture*>();

    if (captures.count() > 0) {
        QSize resolution;
        for (auto&& r : captures[0]->supportedResolutions()) {
            if (r.width() * 3 == r.height() * 4 && r.width() > resolution.width() &&
                r.width() <= MaxResolutionWidth && r.height() <= MaxResolutionHeight) {
                resolution = r;
            }
        }

        return resolution;
    } else {
        return {};
    }
}

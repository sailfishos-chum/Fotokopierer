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
#include <QtCore/QStandardPaths>

const QString ApplicationName = QStringLiteral("Fotokopierer");

const QString ApplicationVersion = QStringLiteral("0.1");

const QString FilenameFormat = QStringLiteral("yyyy_MM_dd-HH_mm_ss");

const QString DocumentRoot = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
                             QStringLiteral("/Fotokopierer");

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
    return getDocumentDirectory().absoluteFilePath(QDateTime::currentDateTime().toString(FilenameFormat) + QStringLiteral(".jpg"));
}

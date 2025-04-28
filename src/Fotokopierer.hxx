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

#ifndef __FOTOKOPIERER_HXX__
#define __FOTOKOPIERER_HXX__

#include <QtCore/QDir>
#include <QtCore/QString>

/// Name of the application.
extern const QString ApplicationName;

/// The version of the application.
extern const QString ApplicationVersion;

/// Format used for image files.
extern const QString FilenameFormat;

/// The root path for all documents.
extern const QString DocumentRoot;

/// Return the document directory.
///
/// If the document root directory does not exist it is created.
QDir getDocumentDirectory();

/// General utilities exported to QML.
class Fotokopierer : public QObject
{
    Q_OBJECT

public:
    explicit Fotokopierer(QObject* parent = nullptr)
        : QObject(parent) {}

    Q_INVOKABLE bool isConvex(QPointF x1, QPointF x2, QPointF x3, QPointF x4);

    Q_INVOKABLE QString newImagePath();
};

#endif

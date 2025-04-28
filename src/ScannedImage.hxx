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

#ifndef __FOTOKOPIERER_SCANNEDIMAGE_HXX__
#define __FOTOKOPIERER_SCANNEDIMAGE_HXX__

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>

#include <QtGui/QPixmap>

class ScannedImage : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QPixmap original READ originalImage NOTIFY originalChanged)

public:
	ScannedImage(QObject* parent = nullptr);
	~ScannedImage();

	QPixmap originalImage() const;

signals:
	void originalChanged();

private:
	struct Data;
	QScopedPointer<Data> d;
};

#endif

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

#ifndef __FOTOKOPIERER_SCANIMAGE_HXX__
#define __FOTOKOPIERER_SCANIMAGE_HXX__

#include <QtCore/QObject>

#include <memory>

class Document;
class Filter;
class RotateFilter;
class CutFilter;
class ColorizeFilter;

class ScanImage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(RotateFilter* rotateFilter READ rotateFilter CONSTANT);
    Q_PROPERTY(CutFilter* cutFilter READ cutFilter CONSTANT);
    Q_PROPERTY(ColorizeFilter* ColorizeFilter READ colorizeFilter CONSTANT);

public:
    enum class FilterType {
        None = -1,
        Rotate = 0,
        Cut = 1,
        Colorize = 2,
    };
    Q_ENUM(FilterType);

public:
    ScanImage(QObject* parent = nullptr);

    ~ScanImage() override;

    std::shared_ptr<Filter> filter(FilterType type);

    Q_INVOKABLE bool loadFile(const QString& file_name);

    Q_INVOKABLE void saveAndClear(Document* doc);

    QImage originalImage() const;

    RotateFilter* rotateFilter() const;

    CutFilter* cutFilter() const;

    ColorizeFilter* colorizeFilter() const;

signals:
    void originalImageChanged();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

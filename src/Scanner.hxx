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

#ifndef __FOTOKOPIERER_SCANNER_HXX__
#define __FOTOKOPIERER_SCANNER_HXX__

#include <QtCore/QObject>

#include <memory>

class Document;
class Filter;
class RotateFilter;
class CutFilter;
class ColorizeFilter;

class Scanner : public QObject
{
    Q_OBJECT

    Q_PROPERTY(RotateFilter* rotateFilter READ rotateFilter CONSTANT);
    Q_PROPERTY(CutFilter* cutFilter READ cutFilter CONSTANT);
    Q_PROPERTY(ColorizeFilter* ColorizeFilter READ colorizeFilter CONSTANT);
    Q_PROPERTY(bool deleteOriginalOnClear READ deleteOriginalOnClear WRITE setDeleteOriginalOnClear
                   NOTIFY deleteOriginalOnClearChanged);

public:
    enum class FilterType {
        None = -1,
        Rotate = 0,
        Cut = 1,
        Colorize = 2,
    };
    Q_ENUM(FilterType);

public:
    Scanner(QObject* parent = nullptr);

    Scanner(const Scanner&) = delete;
    Scanner(Scanner&&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    Scanner& operator=(Scanner&&) = delete;

    ~Scanner() override;

    /// Return the original image.
    QImage original() const;

    /// Compute and return the filtered image.
    QImage computeFilteredImage() const;

    Filter* filter(FilterType type);

    Q_INVOKABLE bool loadFile(const QString& file_name);

    Q_INVOKABLE void clear();

    QImage originalImage() const;

    RotateFilter* rotateFilter() const;

    CutFilter* cutFilter() const;

    ColorizeFilter* colorizeFilter() const;

    void setDeleteOriginalOnClear(bool enabled);

    bool deleteOriginalOnClear() const;

    /// Add this scanned page to the given `Document`.
    Q_INVOKABLE void addPage(Document* doc);

signals:
    void originalImageChanged();

    void addPage(QImage original, QImage result);

    void imageSaved();

    void deleteOriginalOnClearChanged();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

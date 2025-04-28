/*
 * Copyright (c) 2018, 2019, 2021 Frank Fischer <frank-fischer@shadow-soft.de>
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
class Page;
class ScanImage;

/// The image scanner.
///
/// This is the basic interface to create new or modify existing
/// images. It basically creates and manages `ScanImage` objects.
class Scanner : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool deleteOriginalOnClear READ deleteOriginalOnClear WRITE setDeleteOriginalOnClear
                   NOTIFY deleteOriginalOnClearChanged);

public:
    Scanner(QObject* parent = nullptr);

    Scanner(const Scanner&) = delete;
    Scanner(Scanner&&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    Scanner& operator=(Scanner&&) = delete;

    ~Scanner() override;

    Q_INVOKABLE bool loadPage(Page* page);

    Q_INVOKABLE bool loadFile(const QString& file_name);

    bool loadFile(const QString& file_name, const QJsonObject& settings);

    Q_INVOKABLE void clear();

    void setDeleteOriginalOnClear(bool enabled);

    bool deleteOriginalOnClear() const;

    /// Return the current scan image.
    std::shared_ptr<ScanImage> currentImage() const;

    /// Add this scanned page to the given `Document`.
    Q_INVOKABLE void addPage(Document* doc);

    /// Add this scanned page to the given `Document`.
    Q_INVOKABLE void updatePage(Page* page);

signals:
    void addPage(const QImage& original, const QImage& result);

    void currentImageChanged();

    void deleteOriginalOnClearChanged();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

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

#ifndef __FOTOKOPIERER_DOCUMENT_HXX__
#define __FOTOKOPIERER_DOCUMENT_HXX__

#include <QtCore/QAbstractListModel>
#include <QtCore/QScopedPointer>

#include <memory>

/// A scanned document
///
/// This is an ordered collection of scanned pages.
class Document : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool enableAddButton READ isAddButtonEnabled WRITE setAddButtonEnabled NOTIFY
                   addButtonEnabledChanged)
public:
    enum PageRoles { PageRole = Qt::UserRole + 1, AddButtonRole };

public:
    Document(QObject *parent = nullptr);

    ~Document();

    /// True if the documents returns an additional element used for the "Add-Button".
    bool isAddButtonEnabled() const;

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool save() const;

    bool load(const QString &filename, QObject *parent = nullptr);

public slots:
    void setAddButtonEnabled(bool enabled);

signals:
    void addButtonEnabledChanged();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

#endif

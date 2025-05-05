/*
 * Copyright (c) 2025 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include "Document.hxx"

#include <QtCore/QUrl>
#include <QtGui/QPageSize>
#include <QtGui/QPainter>
#include <QtGui/QPdfWriter>

#include "Fotokopierer.hxx"

namespace {
/// Error when exporting a pdf document.
class PdfError : public QException
{
public:
    explicit PdfError(const QString& message)
        : message_(message) {}

    PdfError(const PdfError&) = default;
    PdfError(PdfError&&) noexcept = default;
    PdfError& operator=(const PdfError&) = default;
    PdfError& operator=(PdfError&&) noexcept = default;
    ~PdfError() override = default;

    void raise() const override { throw *this; }
    PdfError* clone() const override { return new PdfError(*this); }

    QString message() const { return message_; }

private:
    QString message_;
};
}

void Document::doExportToPdf(const QString& filename, const QString& title, const QStringList& pageimages)
{
    QPdfWriter pdf(filename);
    QPainter p;

    bool firstpage = true;

    for (auto& page : pageimages) {
        QImage pageimage(page);
        pdf.setPageMargins({0, 0, 0, 0});
        pdf.setPageSize(QPageSize(pageimage.size()));

        if (firstpage) {
            p.begin(&pdf);
            firstpage = false;
        } else if (!pdf.newPage()) {
            throw PdfError(tr("Cannot create a new page"));
        };

        p.drawImage(QRect{0, 0, pdf.width(), pdf.height()},
                    pageimage,
                    QRect{0, 0, pageimage.width(), pageimage.height()});
    }
    p.end();

    pdf.setCreator(ApplicationName);
    pdf.setTitle(title);
}

QUrl Document::doGetPendingPdf(const QFutureWatcher<QUrl>& pendingPdf)
{
    try {
        return pendingPdf.result();
    } catch (PdfError& e) {
        emit error(e.message());
        return {};
    }
}




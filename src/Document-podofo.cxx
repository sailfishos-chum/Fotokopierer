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

#include <podofo/podofo.h>

#include "Fotokopierer.hxx"

static PoDoFo::PdfString toPdfString(const QString& str)
{
    return {str.toUtf8().constData()};
}

void Document::doExportToPdf(const QString& filename, const QString& title, const QStringList& pageimages)
{
    using namespace PoDoFo;
    PdfMemDocument pdf;
    PdfPainter painter;

    for (auto& page : pageimages) {
        try {
            auto pageimage = pdf.CreateImage();
            pageimage->Load(page.toStdString());

            auto& pages = pdf.GetPages();
            auto& pdfpage = pages.CreatePage({0.0, 0.0, static_cast<double>(pageimage->GetWidth()), static_cast<double>(pageimage->GetHeight())});

            painter.SetCanvas(pdfpage);
            painter.DrawImage(*pageimage, 0.0, 0.0);
            painter.FinishDrawing();
        } catch (PdfError& e) {
            qWarning() << "Error exporting pdf: " << e.what() << " while processing " << page;
            e.PrintErrorMsg();

            throw;
        }
    }

    pdf.GetMetadata().SetCreator(toPdfString(ApplicationName));
    pdf.GetMetadata().SetTitle(toPdfString(title));
    pdf.Save(filename.toStdString());
}

QUrl Document::doGetPendingPdf(const QFutureWatcher<QUrl>& pendingPdf)
{
    try {
        return pendingPdf.result();
    } catch (PoDoFo::PdfError& e) {
        emit error(QString::fromUtf8(e.what()));
        return {};
    }
}





/*
 * Copyright (c) 2018-2021 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include <QtCore/QCommandLineParser>
#include <QtCore/QTranslator>

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlEngine>
#include <QtQml/QtQml>

#include "Clipboard.hxx"
#include "ColorizeChooser.hxx"
#include "ColorizeView.hxx"
#include "CutView.hxx"
#include "Scanner.hxx"
#include "ZoomImage.hxx"

#include "Document.hxx"
#include "DocumentList.hxx"
#include "Fotokopierer.hxx"
#include "Page.hxx"

void init_app(QGuiApplication& app, QQmlEngine& engine)
{
    app.setApplicationName(ApplicationName);
    app.setApplicationVersion(ApplicationVersion);

    cleanupImageDirectory();

    qmlRegisterSingletonType<Fotokopierer>(
        "Fotokopierer", 1, 0, "Fotokopierer", [](QQmlEngine*, QJSEngine*) -> QObject* {
            return new Fotokopierer();
        });

    qmlRegisterSingletonType<Clipboard>(
        "Fotokopierer", 1, 0, "PageClipboard", [](QQmlEngine* engine, QJSEngine*) -> QObject* {
            auto cb = Clipboard::instance();
            engine->setObjectOwnership(cb, QQmlEngine::CppOwnership);
            return cb;
        });

    qmlRegisterSingletonType<DocumentList>(
        "Fotokopierer", 1, 0, "DocumentList", [](QQmlEngine*, QJSEngine*) -> QObject* {
            return new DocumentList;
        });

    qmlRegisterSingletonType<Scanner>(
        "Fotokopierer", 1, 0, "Scanner", [](QQmlEngine*, QJSEngine*) -> QObject* {
            return new Scanner();
        });

    qmlRegisterType<ZoomImage>("Fotokopierer", 1, 0, "ZoomImage");
    qmlRegisterType<ColorizeChooser>("Fotokopierer", 1, 0, "ColorizeChooser");
    qmlRegisterType<ColorizeView>("Fotokopierer", 1, 0, "ColorizeView");
    qmlRegisterType<CutView>("Fotokopierer", 1, 0, "CutView");
    qmlRegisterUncreatableType<Document>(
        "Fotokopierer",
        1,
        0,
        "Document",
        QStringLiteral("Document cannot be used as QML component"));
    qmlRegisterUncreatableType<Page>(
        "Fotokopierer", 1, 0, "DocPage", QStringLiteral("Page cannot be used as QML component"));

    app.setApplicationName(QStringLiteral("Fotokopierer"));
    app.setApplicationVersion(QStringLiteral(QT_VERSION_STR));

    QTranslator qtTranslator;
    qtTranslator.load(QStringLiteral("harbour-fotokopierer-%1").arg(QLocale::system().name()),
                      QStringLiteral(":/translations/"));
    app.installTranslator(&qtTranslator);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Document Scanner"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("file"), QStringLiteral("The image file to show"));
    parser.process(app);
}

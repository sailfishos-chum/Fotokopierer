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

#include <QtCore/QCommandLineParser>
#include <QtCore/QTranslator>

#include <QtGui/QGuiApplication>

#include <QtQml/QQmlApplicationEngine>

#include <opencv2/imgproc/imgproc.hpp>


int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    app.setApplicationName(QStringLiteral("Fotokopierer"));
    app.setApplicationVersion(QLatin1String(QT_VERSION_STR));

    QTranslator qtTranslator;
    qtTranslator.load(QLatin1String("harbour-fotokopierer-") +
                      QLocale::system().name(),
                      QLatin1String(":/translations/"));
    app.installTranslator(&qtTranslator);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Document Scanner"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("file"),
                                 QStringLiteral("The image file to show"));
    parser.process(app);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///qml-desktop/main.qml")));

    return app.exec();
}

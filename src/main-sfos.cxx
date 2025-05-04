/*
 * Copyright (c) 2018, 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include <sailfishapp.h>

#include <QtCore/QTranslator>
#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>
#include <memory>

#include "init.hxx"

// #include <opencv2/highgui/highgui.hpp>

int main(int argc, char* argv[])
{
    std::unique_ptr<QGuiApplication> app(SailfishApp::application(argc, argv));

    std::unique_ptr<QQuickView> view(SailfishApp::createView());

    QTranslator translator;
    if (translator.load(QLocale(),
                        QLatin1String("harbour-fotokopierer"),
                        QLatin1String("-"),
                        QLatin1String(":/translations"))) {
        app->installTranslator(&translator);
    }

    init_app(*app, *view->engine());

    view->setSource(SailfishApp::pathTo(QStringLiteral("qml/sfos/main.qml")));
    view->show();

    return app->exec();
}

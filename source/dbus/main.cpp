#include <QCoreApplication>
#include <QQmlContext>
#include <QLocale>
#include <QTranslator>
#include <QtDBus/QDBusConnection>
#include <QDebug>

#include "Swiften/EventLoop/Qt/QtEventLoop.h"

#include "Shmoose.h"
#include "DbusCommunicator.h"

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        // app
        QCoreApplication app(argc, argv);

        qDebug() << "argc: " << argc << ", argv0: " << argv[1];

        // eventloop
        Swift::QtEventLoop eventLoop;
        Swift::BoostNetworkFactories networkFactories(&eventLoop);
        Shmoose shmoose(&networkFactories);

        // dbus
        QString dbusServiceName("org.shmoose.dbuscom");
        dbusServiceName += QString(argv[1]);
        QString dbusPath = "/client";

        DbusCommunicator dbusCommunicator(dbusPath, dbusServiceName);
        dbusCommunicator.setXmpClient(&shmoose);

        // fly
        return app.exec();
    }
}


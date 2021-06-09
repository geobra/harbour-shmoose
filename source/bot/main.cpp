#include <QCoreApplication>
#include <QQmlContext>
#include <QLocale>
#include <QTranslator>
#include <QDebug>

#include "Swiften/EventLoop/Qt/QtEventLoop.h"

#include "Shmoose.h"
#include "Settings.h"
#include "CommandProcessor.h"

int main(int argc, char *argv[])
{
    // app
    QCoreApplication app(argc, argv);

    app.setOrganizationName("shmoose");
    app.setOrganizationDomain("harbour");

    // eventloop
    Swift::QtEventLoop eventLoop;
    Swift::BoostNetworkFactories networkFactories(&eventLoop);

    // get saved credentials
    Settings settings;
    QString acountJid = settings.getJid();
    QString pass = settings.getPassword();

    // setup shmoose and the command processor
    Shmoose shmoose(&networkFactories);
    CommandProcessor commandProcessor(&shmoose);

    // connect
    shmoose.mainConnect(acountJid, pass);

    // fly
    return app.exec();
}


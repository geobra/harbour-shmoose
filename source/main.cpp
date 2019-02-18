#ifdef SFOS
#include <sailfishapp.h>
#endif

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QtQml>
#include <QLocale>
#include <QTranslator>

#include "Swiften/EventLoop/Qt/QtEventLoop.h"

#include "Shmoose.h"
#include "RosterContoller.h"
#include "RosterItem.h"
#include "Persistence.h"
#include "MessageController.h"
#include "SessionController.h"
#include "FileModel.h"
#include "System.h"

int main(int argc, char *argv[])
{
    qmlRegisterType<RosterController>( "harbour.shmoose", 1, 0, "RosterController");
    qmlRegisterType<RosterItem>( "harbour.shmoose", 1, 0, "RosterItem");

    qRegisterMetaType<Settings*>("Settings*");

    qRegisterMetaType<Persistence*>("Persistence*");
    qRegisterMetaType<MessageController*>("MessageController*");
    qRegisterMetaType<MessageController*>("SessionController*");

    // app
    QGuiApplication *pApp = NULL;

#ifdef SFOS
    QGuiApplication *app = SailfishApp::application(argc, argv);
    QQuickView *view = SailfishApp::createView();
    pApp = app;
#else
    QGuiApplication app(argc, argv);
    pApp = &app;
#endif

    // i18n
#ifdef SFOS
    QTranslator shmooseTranslator;
    QString locale = QLocale::system().name();
    if(!shmooseTranslator.load(SailfishApp::pathTo("translations").toLocalFile() + "/" + locale + ".qm"))
    {
        qDebug() << "Couldn't load translation";
    }

    pApp->installTranslator(&shmooseTranslator);
#endif

    // eventloop
    Swift::QtEventLoop eventLoop;
    Swift::BoostNetworkFactories networkFactories(&eventLoop);
    Shmoose shmoose(&networkFactories);

    FileModel fileModel;

#ifdef SFOS
    view->rootContext()->setContextProperty("shmoose", &shmoose);
    view->rootContext()->setContextProperty("fileModel", &fileModel);

    view->setSource(SailfishApp::pathTo("qml/main.qml"));
    view->showFullScreen();
#else
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("shmoose", &shmoose);
    engine.load(QUrl("qrc:/main.qml"));

    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow*>(topLevel);

    window->show();
#endif

    return pApp->exec();
}

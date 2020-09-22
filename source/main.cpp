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
#include "RosterController.h"
#include "RosterItem.h"
#include "Persistence.h"
#include "MessageController.h"
#include "SessionController.h"
#include "GcmController.h"

#include "FileModel.h"
#include "System.h"

#ifdef QMLLIVE_SOURCE
// Use QML Live headers
#include "livenodeengine.h"
#include "remotereceiver.h"
#include "qmllive_global.h"
//#include "constants.h"
#endif

int main(int argc, char *argv[])
{
    qmlRegisterType<RosterController>( "harbour.shmoose", 1, 0, "RosterController");
    qmlRegisterType<RosterItem>( "harbour.shmoose", 1, 0, "RosterItem");

    qRegisterMetaType<Settings*>("Settings*");

    qRegisterMetaType<Persistence*>("Persistence*");
    qRegisterMetaType<MessageController*>("MessageController*");
    qRegisterMetaType<MessageController*>("SessionController*");
    qRegisterMetaType<GcmController*>("GcmController*");

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


#ifndef SFOS
    // only needed on Desktop for file picker dialog. On SFOS defaults are already used and config gets written there.
    pApp->setOrganizationName("shmoose");
    pApp->setOrganizationDomain("harbour");
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
#ifdef QMLLIVE_SOURCE
    engine.load("source/qml/main.qml");
#else
    engine.load(QUrl("qrc:/main.qml"));
#endif

    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow*>(topLevel);

    window->show();

#ifdef QMLLIVE_SOURCE
    // https://doc.qt.io/QMLLive/index.html

    LiveNodeEngine node;

    // Let QML Live know your runtime
    node.setQmlEngine(&engine);

    // Allow it to display QML components with non-QQuickWindow root object
    QQuickView fallbackView(&engine, 0);
    node.setFallbackView(&fallbackView);

    // Tell it where file updates should be stored relative to
    node.setWorkspace(app.applicationDirPath(), LiveNodeEngine::AllowUpdates | LiveNodeEngine::UpdatesAsOverlay);
    //node.setWorkspace("qml", LiveNodeEngine::AllowUpdates | LiveNodeEngine::UpdatesAsOverlay);

    // Listen to IPC call from remote QML Live Bench
    RemoteReceiver receiver;
    receiver.registerNode(&node);
    receiver.listen(49156);

    // Advanced use: let it know the initially loaded QML component (do this
    // only after registering to receiver!)
    //node.usePreloadedDocument(engine.mainQml(), engine.mainWindow(), engine.warnings());
#endif

#endif

    return pApp->exec();
}

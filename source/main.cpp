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

	qRegisterMetaType<Persistence*>("Persistence*");
	qRegisterMetaType<MessageController*>("MessageController*");
	qRegisterMetaType<MessageController*>("SessionController*");

    // app
#ifdef SFOS
	QGuiApplication *app = SailfishApp::application(argc, argv);
	QQuickView *view = SailfishApp::createView();
#else
	QGuiApplication app(argc, argv);
#endif

    // i18n
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),	QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);
    // Translations
    QTranslator shmooseTranslator;
    // (QLocale::system().name() != "C")?(QLocale::system().name()):("en_GB"), "/usr/share/harbour-shmoose/translations/")
    shmooseTranslator.load(QLocale::system().name()); // loads the systems locale or none
    app.installTranslator(&shmooseTranslator);

    // eventloop
	QtEventLoop eventLoop;
	BoostNetworkFactories networkFactories(&eventLoop);
	Shmoose shmoose(&networkFactories);
    FileModel fileModel;

#ifdef SFOS
	view->rootContext()->setContextProperty("shmoose", &shmoose);
    view->rootContext()->setContextProperty("fileModel", &fileModel);

	view->setSource(QUrl::fromLocalFile("/usr/share/harbour-shmoose/qml/main.qml"));
	view->showFullScreen();

	return app->exec();
#else
	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("shmoose", &shmoose);
	engine.load(QUrl("qrc:/main.qml"));

	QObject *topLevel = engine.rootObjects().value(0);
	QQuickWindow *window = qobject_cast<QQuickWindow*>(topLevel);

	window->show();
	return app.exec();
#endif
}

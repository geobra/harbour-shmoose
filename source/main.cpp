#ifdef SFOS
#include <sailfishapp.h>
#endif

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QtQml>

#include "Swiften/EventLoop/Qt/QtEventLoop.h"

#include "Shmoose.h"
#include "RosterContoller.h"
#include "RosterItem.h"
#include "Persistence.h"
#include "MessageController.h"
#include "SessionController.h"


int main(int argc, char *argv[])
{
	qmlRegisterType<RosterController>( "harbour.shmoose", 1, 0, "RosterController");
	qmlRegisterType<RosterItem>( "harbour.shmoose", 1, 0, "RosterItem");

	qRegisterMetaType<Persistence*>("Persistence*");
	qRegisterMetaType<MessageController*>("MessageController*");
	qRegisterMetaType<MessageController*>("SessionController*");

#ifdef SFOS
	QGuiApplication *app = SailfishApp::application(argc, argv);
	QQuickView *view = SailfishApp::createView();
#else
	QGuiApplication app(argc, argv);
#endif

	QtEventLoop eventLoop;
	BoostNetworkFactories networkFactories(&eventLoop);
	Shmoose shmoose(&networkFactories);

#ifdef SFOS
	view->rootContext()->setContextProperty("shmoose", &shmoose);

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

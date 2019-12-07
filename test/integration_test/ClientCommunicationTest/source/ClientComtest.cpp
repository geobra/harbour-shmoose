#include "ClientComtest.h"
#include "DbusInterfaceWrapper.h"

#include <QSignalSpy>

void ClientComTest::initTestCase()
{
    QString dbusServiceNameCommon("org.shmoose.dbuscom");

    QString dbusServiceNameLhs = dbusServiceNameCommon + "lhs";
    QString dbusServiceNameRhs = dbusServiceNameCommon + "rhs";

    QString dbusObjectPath("/client");

    interfaceLhs = new DbusInterfaceWrapper(dbusServiceNameLhs, dbusObjectPath, "", QDBusConnection::sessionBus(), this);
    interfaceRhs = new DbusInterfaceWrapper(dbusServiceNameRhs, dbusObjectPath, "", QDBusConnection::sessionBus(), this);
}

void ClientComTest::cleanupTestCase()
{

}

// connection test
void ClientComTest::connectionTest()
{
    connectionTestCommon(interfaceLhs, "user1@localhost", "user1");
    connectionTestCommon(interfaceRhs, "user2@localhost", "user2");
}

void ClientComTest::connectionTestCommon(DbusInterfaceWrapper *interface, const QString& jid, const QString& pass)
{
    QList<QVariant> arguments{jid, pass};
    interface->callDbusMethodWithArgument("tryToConnect", arguments);

    QSignalSpy spySignalConnected(interface->getInterface(), SIGNAL(connected()));
    spySignalConnected.wait();
}

void ClientComTest::receiveConnectedSignal(QString str)
{
    qDebug() << "from daemon::connected signal: " << str;
}

// request roster test
void ClientComTest::requestRosterTest()
{
    requestRosterTestCommon(interfaceLhs);
    requestRosterTestCommon(interfaceRhs);
}

void ClientComTest::requestRosterTestCommon(DbusInterfaceWrapper *interface)
{
    interface->callDbusMethodWithArgument("requestRoster", QList<QVariant>());

    QSignalSpy spySignalConnected(interface->getInterface(), SIGNAL(newRosterEntry()));
    spySignalConnected.wait();
}

// add contact test
void ClientComTest::addContactTest()
{
    addContactTestCommon(interfaceLhs, "user2@localhost", "user2");
    addContactTestCommon(interfaceRhs, "user1@localhost", "user1");
}

void ClientComTest::addContactTestCommon(DbusInterfaceWrapper *interface, const QString& jid, const QString& name)
{
    QList<QVariant> arguments {jid, name};
    interface->callDbusMethodWithArgument("addContact", arguments);

    QSignalSpy spySignalConnected(interface->getInterface(), SIGNAL(newRosterEntry()));
    spySignalConnected.wait(1000);
}

// send msg test
void ClientComTest::sendMsgTest()
{
    // send msg user1 to user2
    QList<QVariant> arguments {"user2@localhost", "Hi user2 from user1"};
    interfaceLhs->callDbusMethodWithArgument("sendMsg", arguments);

    // wait for msg to arrive at user2
    QSignalSpy spySignalNewMsg(interfaceRhs->getInterface(), SIGNAL(signalMsgChanged()));
    spySignalNewMsg.wait();

    // check for content of latest msg at user2 from user1
    QList<QVariant> arguments2 {"user1@localhost"};
    interfaceRhs->callDbusMethodWithArgument("requestLatestMsgForJid", arguments2);

    QSignalSpy spyLatestMsg(interfaceRhs->getInterface(), SIGNAL(signalLatestMsg(QString, int)));
    spyLatestMsg.wait();

    QCOMPARE(spyLatestMsg.count(), 1);
    QList<QVariant> spyArguments = spyLatestMsg.takeFirst();
    QVERIFY(spyArguments.at(0).toString() == "foo");
}


QTEST_MAIN(ClientComTest)

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

    QSignalSpy spySignalConnected(interface->getInterface(), SIGNAL(signalConnected()));
    spySignalConnected.wait();
    QCOMPARE(spySignalConnected.count(), 1);
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

    QSignalSpy spyNewRosterEntry(interface->getInterface(), SIGNAL(signalNewRosterEntry()));
    spyNewRosterEntry.wait();
    QCOMPARE(spyNewRosterEntry.count(), 1);
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

    QSignalSpy spyNewRosterEntry(interface->getInterface(), SIGNAL(signalNewRosterEntry()));
    spyNewRosterEntry.wait(200);
    //QCOMPARE(spyNewRosterEntry.count(), 1);
}

// send msg test
void ClientComTest::sendMsgTest()
{
    const QString msgOnWire = "Hi user2 from user1";

    // send msgOnWire from user1 to user2
    QList<QVariant> arguments {"user2@localhost", msgOnWire};
    interfaceLhs->callDbusMethodWithArgument("sendMsg", arguments);

    // wait for arrived msgOnWire at other client
    QSignalSpy spyLatestMsg(interfaceRhs->getInterface(), SIGNAL(signalLatestMsg(QString, QString, QString)));
    spyLatestMsg.wait();
    QCOMPARE(spyLatestMsg.count(), 1);

    QList<QVariant> spyArguments = spyLatestMsg.takeFirst();
    QVERIFY(spyArguments.at(2).toString() == msgOnWire);
}


QTEST_MAIN(ClientComTest)

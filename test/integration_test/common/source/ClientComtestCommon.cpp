#include "ClientComtestCommon.h"
#include "DbusInterfaceWrapper.h"

#include <QSignalSpy>
#include <QImageWriter>

/*
 * Tests:
 * Connect, Disconnect, Reconnect
 *
 * Group:
 * Add group, delete group
 * send group msg, check status. Send group msg with offline clients, check status. Reconnect and check status again
 * Check simple mam
 * Check recieving order of msg on reconnect
 *
 * Roster:
 * Add contact, delete contact, check status, avatar
 *
 */

ClientComTestCommon::ClientComTestCommon() : user1jid_("user1@localhost"), user2jid_("user2@localhost"), imageFileName_("/tmp/64x64-red.jpeg")
{
    generatePicture();
}

void ClientComTestCommon::initTestCase()
{
    QString dbusServiceNameCommon("org.shmoose.dbuscom");

    QString dbusServiceNameLhs = dbusServiceNameCommon + "lhs";
    QString dbusServiceNameRhs = dbusServiceNameCommon + "rhs";

    QString dbusObjectPath("/client");

    interfaceLhs_ = new DbusInterfaceWrapper(dbusServiceNameLhs, dbusObjectPath, "", QDBusConnection::sessionBus(), this);
    interfaceRhs_ = new DbusInterfaceWrapper(dbusServiceNameRhs, dbusObjectPath, "", QDBusConnection::sessionBus(), this);
}

void ClientComTestCommon::cleanupTestCase()
{

}

// connection test
void ClientComTestCommon::connectionTest()
{
    connectionTestCommon(interfaceLhs_, user1jid_, "user1");
    connectionTestCommon(interfaceRhs_, user2jid_, "user2");
}

void ClientComTestCommon::connectionTestCommon(DbusInterfaceWrapper *interface, const QString& jid, const QString& pass)
{
    QList<QVariant> arguments{jid, pass};
    interface->callDbusMethodWithArgument("tryToConnect", arguments);

    QSignalSpy spySignalConnected(interface->getInterface(), SIGNAL(signalConnected()));
    spySignalConnected.wait();
    QCOMPARE(spySignalConnected.count(), 1);

    spySignalConnected.wait(5000); // wait until all initial handshakes are done
}

void ClientComTestCommon::receiveConnectedSignal(QString str)
{
    qDebug() << "from daemon::connected signal: " << str;
}

// request roster test
void ClientComTestCommon::requestRosterTest()
{
    requestRosterTestCommon(interfaceLhs_);
    requestRosterTestCommon(interfaceRhs_);
}

void ClientComTestCommon::requestRosterTestCommon(DbusInterfaceWrapper *interface)
{
    QSignalSpy spyNewRosterEntry(interface->getInterface(), SIGNAL(signalNewRosterEntry()));
    interface->callDbusMethodWithArgument("requestRoster", QList<QVariant>());

    spyNewRosterEntry.wait();
    //QCOMPARE(spyNewRosterEntry.count(), 1); // temporary disabled
}

// add contact test
void ClientComTestCommon::addContactTest()
{
    addContactTestCommon(interfaceLhs_, user2jid_, "user2");
    addContactTestCommon(interfaceRhs_, user1jid_, "user1");
}

void ClientComTestCommon::addContactTestCommon(DbusInterfaceWrapper *interface, const QString& jid, const QString& name)
{
    QList<QVariant> arguments {jid, name};
    interface->callDbusMethodWithArgument("addContact", arguments);

    QSignalSpy spyNewRosterEntry(interface->getInterface(), SIGNAL(signalNewRosterEntry()));
    spyNewRosterEntry.wait(200);
    //QCOMPARE(spyNewRosterEntry.count(), 1);
}

void ClientComTestCommon::generatePicture()
{
    QString imagePath(imageFileName_);
    QImage image(64, 64, QImage::Format_RGB32);
    image.fill(Qt::red);
    {
        QImageWriter writer(imagePath);
        writer.write(image);
    }
}

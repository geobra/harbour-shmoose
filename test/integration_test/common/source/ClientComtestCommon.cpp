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

ClientComTestCommon::ClientComTestCommon() : user1jid_("user1@localhost"), user2jid_("user2@localhost"), user3jid_("user3@localhost"), roomJid_("testroom@conference.localhost"),
    dbusServiceNameCommon_("org.shmoose.dbuscom"), dbusObjectPath_("/client"),
    imageFileName_("/tmp/64x64-red.jpeg"), timeOutConnect_(4000),
    timeOut_(20000)
{
    generatePicture();
}

void ClientComTestCommon::initTestCase()
{
    QString dbusServiceNameLhs = dbusServiceNameCommon_ + "lhs";
    QString dbusServiceNameRhs = dbusServiceNameCommon_ + "rhs";

    interfaceLhs_ = new DbusInterfaceWrapper(dbusServiceNameLhs, dbusObjectPath_, "", QDBusConnection::sessionBus(), this);
    interfaceRhs_ = new DbusInterfaceWrapper(dbusServiceNameRhs, dbusObjectPath_, "", QDBusConnection::sessionBus(), this);
}

void ClientComTestCommon::cleanupTestCase()
{
    // quit clients
    interfaceLhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
    interfaceRhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());

    interfaceRhs_->callDbusMethodWithArgument("rmForcePlainMsgForJid", QList<QVariant>{user1jid_});
    interfaceLhs_->callDbusMethodWithArgument("rmForcePlainMsgForJid", QList<QVariant>{user2jid_});
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

    QSignalSpy spySignalConnected(interface->getInterface(), SIGNAL(signalConnectionStateChanged()));
    spySignalConnected.wait(timeOutConnect_);
    QCOMPARE(spySignalConnected.count(), 1);

    spySignalConnected.wait(timeOutConnect_); // wait until all initial handshakes are done
}

void ClientComTestCommon::receiveConnectedSignal(QString str)
{
    qDebug() << "from daemon::connected signal: " << str;
}

// request roster test
void ClientComTestCommon::requestRosterTest()
{
    // always request roster in first place, before trying to add new contacts!
    //requestRosterTestCommon(interfaceLhs_);
    //requestRosterTestCommon(interfaceRhs_);
}

void ClientComTestCommon::requestRosterTestCommon(DbusInterfaceWrapper *interface, bool doCompare)
{
    QSignalSpy spyNewRosterEntry(interface->getInterface(), SIGNAL(signalNewRosterEntry()));
    interface->callDbusMethodWithArgument("requestRoster", QList<QVariant>());

    spyNewRosterEntry.wait(timeOut_);
    // on travis, the inital roster list is empty
#ifdef TRAVIS
    if (doCompare == true)
    {
        QCOMPARE(spyNewRosterEntry.count(), 1);
    }
#endif
}

// add contact test
void ClientComTestCommon::addContactTest()
{
    //addContactTestCommon(interfaceLhs_, user2jid_, "user2");
    //addContactTestCommon(interfaceRhs_, user1jid_, "user1");
}

void ClientComTestCommon::addContactTestCommon(DbusInterfaceWrapper *interface, const QString& jid, const QString& name, bool doCompare)
{
    QSignalSpy spyNewRosterEntry(interface->getInterface(), SIGNAL(signalNewRosterEntry()));

    QList<QVariant> arguments {jid, name};
    interface->callDbusMethodWithArgument("addContact", arguments);

    spyNewRosterEntry.wait(timeOut_);
#ifdef TRAVIS
    if (doCompare == true)
    {
        QCOMPARE(spyNewRosterEntry.count(), 1);
    }
#endif

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

void ClientComTestCommon::collectLatestMsgRhs(QString msgId, QString jid, QString msg)
{
    qDebug() << "collectLatestMsgRhs: " << msgId << ", " << jid << ", " << msg;
    collectedMsgRhsList_.push_back(MsgContent{msgId, jid, msg});
}

#include "ClientComtest.h"
#include "DbusInterfaceWrapper.h"

#include <QSignalSpy>
#include <QImageWriter>

/*
 * Tests:
 * Connect, Disconnect, Reconnect
 *
 * 1o1:
 * Send 1to1 msg, check status. Send msg to offline client, check status. Reconnect and check status again
 * Check recieving order of msg after reconnect
 * Send pictures (http_upload) and receive them
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

ClientComTest::ClientComTest() : user1jid_("user1@localhost"), user2jid_("user2@localhost"), imageFileName_("/tmp/64x64-red.jpeg")
{
    generatePicture();
}

void ClientComTest::initTestCase()
{
    QString dbusServiceNameCommon("org.shmoose.dbuscom");

    QString dbusServiceNameLhs = dbusServiceNameCommon + "lhs";
    QString dbusServiceNameRhs = dbusServiceNameCommon + "rhs";

    QString dbusObjectPath("/client");

    interfaceLhs_ = new DbusInterfaceWrapper(dbusServiceNameLhs, dbusObjectPath, "", QDBusConnection::sessionBus(), this);
    interfaceRhs_ = new DbusInterfaceWrapper(dbusServiceNameRhs, dbusObjectPath, "", QDBusConnection::sessionBus(), this);
}

void ClientComTest::cleanupTestCase()
{

}

// connection test
void ClientComTest::connectionTest()
{
    connectionTestCommon(interfaceLhs_, user1jid_, "user1");
    connectionTestCommon(interfaceRhs_, user2jid_, "user2");
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
    requestRosterTestCommon(interfaceLhs_);
    requestRosterTestCommon(interfaceRhs_);
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
    addContactTestCommon(interfaceLhs_, user2jid_, "user2");
    addContactTestCommon(interfaceRhs_, user1jid_, "user1");
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

    // Setup msg state spyer
    QSignalSpy spyMsgStateSender(interfaceLhs_->getInterface(), SIGNAL(signalMsgState(QString, int)));
    QSignalSpy spyMsgStateReceiver(interfaceRhs_->getInterface(), SIGNAL(signalMsgState(QString, int)));

    // ####################################################
    // send msgOnWire from user1 to user2. Both are online.
    // ####################################################
    QList<QVariant> argumentsMsgForUser2 {user2jid_, msgOnWire};
    interfaceLhs_->callDbusMethodWithArgument("sendMsg", argumentsMsgForUser2);

    // wait for arrived msgOnWire at other client
    QSignalSpy spyLatestMsg(interfaceRhs_->getInterface(), SIGNAL(signalLatestMsg(QString, QString, QString)));
    spyLatestMsg.wait();
    QCOMPARE(spyLatestMsg.count(), 1);

    QList<QVariant> spyArgumentsOfMsg = spyLatestMsg.takeFirst();
    QVERIFY(spyArgumentsOfMsg.at(2).toString() == msgOnWire);

    // check the msg status as seen from the sender
    spyMsgStateSender.wait();

    if (spyMsgStateSender.size() < 2) // we expect two state changes. if it is not already here, wait another second to arive.
    {
        spyMsgStateSender.wait(1000);
        spyMsgStateReceiver.wait(1000);
    }

    qDebug() << "state changes: " << spyMsgStateSender.count();
    QVERIFY(spyMsgStateSender.count() == 2);

    int expectedState = 1; // (-1) displayedConfirmed, (0) unknown, (1) sent, (2) received, (3) displayed
    while(! spyMsgStateSender.isEmpty())
    {
        QList<QVariant> spyArguments = spyMsgStateSender.takeFirst();
        QVERIFY(spyArguments.at(1).toInt() == expectedState);

        expectedState++;
    }

    // read the message at the received client. status must change to displayed (3). Received client set status to displayedConfirmed (-1)
    QList<QVariant> argumentsCurrentChatPartnerUser1 {user1jid_};
    interfaceRhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerUser1);

    spyMsgStateSender.wait();
    QVERIFY(spyMsgStateSender.count() == 1);
    while(! spyMsgStateSender.isEmpty())
    {
        QList<QVariant> spyArguments = spyMsgStateSender.takeFirst();
        QVERIFY(spyArguments.at(1).toInt() == expectedState);
    }

    // check the state for that sent msg at the receiver side. must be -1, displayedConfirmed.
    if (spyMsgStateReceiver.count() <= 0)
    {
        spyMsgStateReceiver.wait(1000);
    }
    QCOMPARE(spyMsgStateReceiver.count(), 1);
    while(! spyMsgStateReceiver.isEmpty())
    {
        QList<QVariant> spyArguments = spyMsgStateReceiver.takeFirst();
        QVERIFY(spyArguments.at(1).toInt() == -1);
    }

    // ####################################################
    // send msgOnWire from user1 to user2. user2 is offline.
    // ####################################################
    // disconnect one client
    QList<QVariant> argumentsCurrentChatPartnerEmpty {""};
    interfaceRhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerEmpty);
    interfaceRhs_->callDbusMethodWithArgument("disconnectFromServer", QList<QVariant>());

    // send msgOnWire from user1 to user2
    interfaceLhs_->callDbusMethodWithArgument("sendMsg", argumentsMsgForUser2);

    // check the msg status as seen from the sender
    spyMsgStateSender.wait();
    QVERIFY(spyMsgStateSender.count() == 1);

    QList<QVariant> spyArgumentsForSentState = spyMsgStateSender.takeFirst();
    qDebug() << "state of msg whil partner is offline: " << spyArgumentsForSentState.at(1).toInt();
    QVERIFY(spyArgumentsForSentState.at(1).toInt() == 1); // msg has been sent to server. nothing else

    // connect the user2 client again
    interfaceRhs_->callDbusMethodWithArgument("reConnect", QList<QVariant>());

    // wait for the msg delivered to the reconnected client (as seen from the sender)
    spyMsgStateSender.wait();

    QVERIFY(spyMsgStateSender.count() == 1);

    QList<QVariant> spyArgumentsReceivedReconnected = spyMsgStateSender.takeFirst();

    qDebug() << "state: " << spyArgumentsReceivedReconnected.at(1).toInt();
    QVERIFY(spyArgumentsReceivedReconnected.at(1).toInt() == 2); // received from reconnected client

    // read the msg on user1, lhs
    interfaceRhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerUser1);

    // be sure that the state is 'read'
    spyMsgStateSender.wait();
    QVERIFY(spyMsgStateSender.count() == 1);
    QList<QVariant> spyArgumentsRead = spyMsgStateSender.takeFirst();
    QVERIFY(spyArgumentsRead.at(1).toInt() == 3);

    // check the state for that sent msg at the receiver side. must be -1, displayedConfirmed.
    if (spyMsgStateReceiver.count() <= 0)
    {
        spyMsgStateReceiver.wait(1000);
    }
    QCOMPARE(spyMsgStateReceiver.count(), 1);
    QList<QVariant> spyArgumentsDisplayConfirmed = spyMsgStateReceiver.takeFirst();
    QVERIFY(spyArgumentsDisplayConfirmed.at(1).toInt() == -1);

    // #################################################################################################
    // send two messages from user1 to user2. user2 is offline. user2 gets online. check received order.
    // #################################################################################################
    spyLatestMsg.clear();
    // disconnect one client
    interfaceRhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerEmpty);
    interfaceRhs_->callDbusMethodWithArgument("disconnectFromServer", QList<QVariant>());

    // send messages from user1 to user2
    QList<QVariant> msg1ForUser2 {user2jid_, "one"};
    interfaceLhs_->callDbusMethodWithArgument("sendMsg", msg1ForUser2);

    QList<QVariant> msg2ForUser2 {user2jid_, "two"};
    interfaceLhs_->callDbusMethodWithArgument("sendMsg", msg2ForUser2);

    // connect the user2 client again
    interfaceRhs_->callDbusMethodWithArgument("reConnect", QList<QVariant>());

    // wait for the msg delivered to the reconnected client (as seen from the sender)
    spyMsgStateSender.wait(1000);
    spyMsgStateReceiver.wait(1000);

    // two received msg are expected after reconnect.
    QCOMPARE(spyLatestMsg.count(), 2);

    // content of received msg is 'one' and 'two'
    spyArgumentsOfMsg = spyLatestMsg.takeFirst();
    QVERIFY(spyArgumentsOfMsg.at(2).toString() == "one");
    spyArgumentsOfMsg = spyLatestMsg.takeFirst();
    QVERIFY(spyArgumentsOfMsg.at(2).toString() == "two");

    // ##################################
    // send a picture from user1 to user2
    // ##################################

    // send picture from user1 to user2
    QList<QVariant> fileForUser2 {user2jid_, imageFileName_};
    interfaceLhs_->callDbusMethodWithArgument("sendFile", fileForUser2);

    spyMsgStateSender.wait(5000);
    spyMsgStateReceiver.wait(5000);

}

void ClientComTest::quitClientsTest()
{
    interfaceLhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
    interfaceRhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
}

void ClientComTest::generatePicture()
{
    QString imagePath(imageFileName_);
    QImage image(64, 64, QImage::Format_RGB32);
    image.fill(Qt::red);
    {
        QImageWriter writer(imagePath);
        writer.write(image);
    }
}

QTEST_MAIN(ClientComTest)

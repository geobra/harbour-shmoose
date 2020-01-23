#include "ClientRoomMsgTest.h"
#include "DbusInterfaceWrapper.h"

#include <QSignalSpy>
#include <QImageWriter>

/*
 * Common Tests:
 * Connect, Disconnect, Reconnect
 *
 * Group:
 * Join group, leave group
 * send group msg, check status. Send group msg with offline clients, check status. Reconnect and check status again
 * Check simple mam
 * Check recieving order of msg on reconnect
 *
 */

ClientRoomMsgTest::ClientRoomMsgTest() : ClientComTestCommon(), user3jid_("user3@localhost")
{

}

void ClientRoomMsgTest::initTestCase()
{
    ClientComTestCommon::initTestCase();

    QString dbusServiceNameCommon("org.shmoose.dbuscom");
    QString dbusServiceNameMhs = dbusServiceNameCommon + "mhs";
    QString dbusObjectPath("/client");

    interfaceMhs_ = new DbusInterfaceWrapper(dbusServiceNameMhs, dbusObjectPath, "", QDBusConnection::sessionBus(), this);
}


void ClientRoomMsgTest::sendRoomMsgTest()
{
    const QString roomJid = "testroom@conference.localhost";

    // Setup msg state spyer
    QSignalSpy spyMsgStateSender(interfaceLhs_->getInterface(), SIGNAL(signalMsgState(QString, int)));
    QSignalSpy spyMsgStateReceiver(interfaceRhs_->getInterface(), SIGNAL(signalMsgState(QString, int)));

    // user1 and user2 already logged in. Login user3
    connectionTestCommon(interfaceMhs_, user3jid_, "user3");
    requestRosterTestCommon(interfaceMhs_);

    // user1: user2 (from base), user3
    addContactTestCommon(interfaceLhs_, user3jid_, "user3");

    // user2: user1 (from base), user3
    addContactTestCommon(interfaceRhs_, user3jid_, "user3");

    // user3: user1, user2
    addContactTestCommon(interfaceMhs_, user1jid_, "user1");
    addContactTestCommon(interfaceMhs_, user2jid_, "user2");

    // all clients join the room
    QSignalSpy joinRoomLhs(interfaceLhs_->getInterface(), SIGNAL(signalRoomJoined(QString, QString)));
    QSignalSpy joinRoomMhs(interfaceMhs_->getInterface(), SIGNAL(signalRoomJoined(QString, QString)));
    QSignalSpy joinRoomRhs(interfaceRhs_->getInterface(), SIGNAL(signalRoomJoined(QString, QString)));

    QList<QVariant> argumentsJoinRoom {roomJid, "testroom"};
    interfaceLhs_->callDbusMethodWithArgument("joinRoom", argumentsJoinRoom);
    joinRoomLhs.wait();
    QVERIFY(joinRoomLhs.count() == 1);

    interfaceMhs_->callDbusMethodWithArgument("joinRoom", argumentsJoinRoom);
    joinRoomMhs.wait();
    QVERIFY(joinRoomMhs.count() == 1);

    interfaceRhs_->callDbusMethodWithArgument("joinRoom", argumentsJoinRoom);
    joinRoomRhs.wait();
    QVERIFY(joinRoomRhs.count() == 1);

    // user1 sends msg. all other clients should receive them
    const QString msgOnWireFromUser1 = "Hi room from user1";

    // Setup msg state state spyer
    QSignalSpy spyMsgStateAtUser1(interfaceLhs_->getInterface(), SIGNAL(signalMsgState(QString, int)));
    QSignalSpy spyMsgStateAtUser2(interfaceRhs_->getInterface(), SIGNAL(signalMsgState(QString, int)));
    QSignalSpy spyMsgStateAtUser3(interfaceMhs_->getInterface(), SIGNAL(signalMsgState(QString, int)));

    // setup msg text speyer
    QSignalSpy spyLatestMsgAtUser2(interfaceRhs_->getInterface(), SIGNAL(signalLatestMsg(QString, QString, QString)));
    QSignalSpy spyLatestMsgAtUser3(interfaceMhs_->getInterface(), SIGNAL(signalLatestMsg(QString, QString, QString)));

    // send the msg
    QList<QVariant> argumentsMsgToRoom {roomJid, msgOnWireFromUser1};
    interfaceLhs_->callDbusMethodWithArgument("sendMsg", argumentsMsgToRoom);

    // wait for arrived msgOnWireFromUser1 at other clients
    spyLatestMsgAtUser2.wait();
    QCOMPARE(spyLatestMsgAtUser2.count(), 1);

    spyLatestMsgAtUser3.wait();
    QCOMPARE(spyLatestMsgAtUser3.count(), 1);

    // check the msg content
    QList<QVariant> spyArgumentsOfMsgAtUser2 = spyLatestMsgAtUser2.takeFirst();
    QVERIFY(spyArgumentsOfMsgAtUser2.at(2).toString() == msgOnWireFromUser1);

    QList<QVariant> spyArgumentsOfMsgAtUser3 = spyLatestMsgAtUser3.takeFirst();
    QVERIFY(spyArgumentsOfMsgAtUser3.at(2).toString() == msgOnWireFromUser1);


    // check the msg status as seen from the sender
#if 0
    spyMsgStateSender.wait();

    if (spyMsgStateSender.size() < 2) // we expect two state changes. if it is not already here, wait another second to arive.
    {
        spyMsgStateSender.wait(1000);
        spyMsgStateReceiver.wait(1000);
    }
#endif

    // check msg status at user1 side

    // user2 reads msg. check msg status at user1 side for user2 and user3

    // user3 reads msg. check msg statua at user1 side for user2 and user3



    // quit clients
    interfaceLhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
    interfaceRhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
    interfaceMhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
}

QTEST_MAIN(ClientRoomMsgTest)
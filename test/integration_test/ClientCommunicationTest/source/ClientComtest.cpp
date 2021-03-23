#include "ClientComtest.h"
#include "DbusInterfaceWrapper.h"

#include <QSignalSpy>
#include <QImageWriter>
#include <algorithm>

/*
 * Tests:
 * Connect, Disconnect, Reconnect
 *
 * 1o1:
 * Send 1to1 msg, check status. Send msg to offline client, check status. Reconnect and check status again
 * Check recieving order of msg after reconnect
 * Send pictures (http_upload) and receive them
 *
 */

ClientComTest::ClientComTest() : ClientComTestCommon()
{

}

// send msg test
void ClientComTest::sendMsgTest()
{
    if (QFile::exists("useomemo.txt")) // can't use custom command line arguments with qtest :-(.
    {
        interfaceRhs_->callDbusMethodWithArgument("rmForcePlainMsgForJid", QList<QVariant>{user1jid_});
        interfaceLhs_->callDbusMethodWithArgument("rmForcePlainMsgForJid", QList<QVariant>{user2jid_});

        timeOut_ = timeOut_ * 2;
    }
    else
    {
        interfaceRhs_->callDbusMethodWithArgument("addForcePlainMsgForJid", QList<QVariant>{user1jid_});
        interfaceLhs_->callDbusMethodWithArgument("addForcePlainMsgForJid", QList<QVariant>{user2jid_});
    }

    requestRosterTestCommon(interfaceLhs_);
    requestRosterTestCommon(interfaceRhs_);

    // add the contacts
    addContactTestCommon(interfaceLhs_, user2jid_, "user2");
    addContactTestCommon(interfaceRhs_, user1jid_, "user1");

    // need to collect more then one signal here. qsignalspy only catches one at a time. Use an own slot to collet them all.
    QObject::connect(interfaceLhs_->getInterface(), SIGNAL(signalMsgState(QString, int)), this, SLOT(collectMsgStateLhsChanged(QString, int)));
    QObject::connect(interfaceRhs_->getInterface(), SIGNAL(signalMsgState(QString, int)), this, SLOT(collectMsgStateRhsChanged(QString, int)));

    // collect msgs
    QObject::connect(interfaceRhs_->getInterface(), SIGNAL(signalLatestMsg(QString, QString, QString)), this, SLOT(collectLatestMsgRhs(QString, QString, QString)));

    const QString msgOnWire = "Hi user2 from user1";

    // Setup msg state spyer
    QSignalSpy spyMsgStateSender(interfaceLhs_->getInterface(), SIGNAL(signalMsgState(QString, int)));
    QSignalSpy spyMsgStateReceiver(interfaceRhs_->getInterface(), SIGNAL(signalMsgState(QString, int)));

    // ####################################################
    // send msgOnWire from user1 to user2. Both are online.
    // ####################################################
    QSignalSpy spyLatestMsg(interfaceRhs_->getInterface(), SIGNAL(signalLatestMsg(QString, QString, QString)));

    QSignalSpy spyMsgSent(interfaceLhs_->getInterface(), SIGNAL(signalMsgSent(QString)));
    QList<QVariant> argumentsMsgForUser2 {user2jid_, msgOnWire};
    interfaceLhs_->callDbusMethodWithArgument("sendMsg", argumentsMsgForUser2);

    // check msgId of sent msg
    spyMsgSent.wait(timeOut_);
    QVERIFY(spyMsgSent.count() == 1);
    QList<QVariant> spyArgumentsOfMsgSent = spyMsgSent.takeFirst();
    QString msgId = spyArgumentsOfMsgSent.at(0).toString();
    qDebug() << "sent MsgId: " << msgId;

    // wait for arrived msgOnWire at other client
    spyLatestMsg.wait(timeOut_);
    QCOMPARE(spyLatestMsg.count(), 1);

    QList<QVariant> spyArgumentsOfMsg = spyLatestMsg.takeFirst();
    QVERIFY(spyArgumentsOfMsg.at(2).toString() == msgOnWire);

    // check the msg status as seen from the sender
    // there must be 2 msg's for the sent msgId. the first with state change to 1, the second with state change to 2.
    spyMsgStateSender.wait(timeOut_);

    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(lhs, msgId, QList<MsgIdState>{{"foo", 1}, {"foo", 2}}), true);

    // read the message at the received client. status must change to displayed (3). Received client set status to displayedConfirmed (-1)
    QList<QVariant> argumentsCurrentChatPartnerUser1 {user1jid_};
    interfaceRhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerUser1);

    spyMsgStateSender.wait(timeOut_);

    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(lhs, msgId, QList<MsgIdState>{{"foo", 3}}), true);

    // check the state for that sent msg at the receiver side. must be -1, displayedConfirmed.
    spyMsgStateReceiver.wait(timeOut_);
    QVERIFY(spyMsgStateReceiver.count() > 0);

    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(rhs, msgId, QList<MsgIdState>{{"foo", -1}}),true);

    // ####################################################
    // send msgOnWire from user1 to user2. user2 is offline.
    // ####################################################
    // disconnect one client
    QList<QVariant> argumentsCurrentChatPartnerEmpty {""};
    interfaceRhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerEmpty);
    interfaceRhs_->callDbusMethodWithArgument("disconnectFromServer", QList<QVariant>());

    QSignalSpy spySignalDisconnected(interfaceRhs_->getInterface(), SIGNAL(signalConnectionStateChanged()));
    spySignalDisconnected.wait(timeOutConnect_);
    QCOMPARE(spySignalDisconnected.count(), 1);

    // send msgOnWire from user1 to user2
    interfaceLhs_->callDbusMethodWithArgument("sendMsg", argumentsMsgForUser2);

    // check msgId of sent msg
    spyMsgSent.wait(timeOut_);
    QVERIFY(spyMsgSent.count() == 1);
    spyArgumentsOfMsgSent = spyMsgSent.takeFirst();
    msgId = spyArgumentsOfMsgSent.at(0).toString();
    qDebug() << "sent MsgId: " << msgId;

    // check the msg status as seen from the sender
    spyMsgStateSender.wait(timeOut_);

    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(lhs, msgId, QList<MsgIdState>{{"foo", 1}}), true); // msg has been sent to server. nothing else

    // connect the user2 client again
    interfaceRhs_->callDbusMethodWithArgument("reConnect", QList<QVariant>());

    // wait for the msg delivered to the reconnected client (as seen from the sender)
    spyMsgStateReceiver.wait(timeOutConnect_);
    spyMsgStateReceiver.wait(timeOutConnect_);  // wait until the reconnt handshake is done

    spyMsgStateSender.wait(timeOut_); // for the msg ack stanza
    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(lhs, msgId, QList<MsgIdState>{{"foo", 2}}), true); // received from reconnected client

    // read the msg on user1, lhs
    interfaceRhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerUser1);

    // be sure that the state is 'read'
    spyMsgStateSender.wait(timeOut_);
    spyMsgStateReceiver.wait(timeOut_);

    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(lhs, msgId, QList<MsgIdState>{{"foo", 3}}), true);

    // check the state for that sent msg at the receiver side. must be -1, displayedConfirmed.
    if (spyMsgStateReceiver.count() <= 0)
    {
        spyMsgStateReceiver.wait(timeOut_);
    }
    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(rhs, msgId, QList<MsgIdState>{{"foo", -1}}), true);

    // #################################################################################################
    // send two messages from user1 to user2. user2 is offline. user2 gets online. check received order.
    // #################################################################################################
    collectedMsgRhsList_.clear();
    // disconnect one client
    spySignalDisconnected.clear();
    interfaceRhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerEmpty);
    interfaceRhs_->callDbusMethodWithArgument("disconnectFromServer", QList<QVariant>());

    // wait for client to be disconnected
    spySignalDisconnected.wait(timeOutConnect_);
    QCOMPARE(spySignalDisconnected.count(), 1);


    // send messages from user1 to user2
    QList<QVariant> msg1ForUser2 {user2jid_, "one"};
    interfaceLhs_->callDbusMethodWithArgument("sendMsg", msg1ForUser2);

    spyMsgSent.clear();
    spyMsgSent.wait(timeOut_);
    QVERIFY(spyMsgSent.count() == 1);
    spyArgumentsOfMsgSent = spyMsgSent.takeFirst();
    msgId = spyArgumentsOfMsgSent.at(0).toString();
    qDebug() << "sent MsgId1: " << msgId;

    QList<QVariant> msg2ForUser2 {user2jid_, "two"};
    interfaceLhs_->callDbusMethodWithArgument("sendMsg", msg2ForUser2);

    spyMsgSent.clear();
    spyMsgSent.wait(timeOut_);
    QVERIFY(spyMsgSent.count() == 1);
    spyArgumentsOfMsgSent = spyMsgSent.takeFirst();
    msgId = spyArgumentsOfMsgSent.at(0).toString();
    qDebug() << "sent MsgId2: " << msgId;

    // connect the user2 client again
    spySignalDisconnected.clear();
    interfaceRhs_->callDbusMethodWithArgument("reConnect", QList<QVariant>());
    spySignalDisconnected.wait(timeOutConnect_);
    QCOMPARE(spySignalDisconnected.count(), 1);

    // wait for the msg delivered to the reconnected client (as seen from the sender)
    spyMsgStateReceiver.wait(timeOutConnect_);
    spyMsgStateReceiver.wait(timeOutConnect_);  // wait until the reconnt handshake is done

    spyMsgStateSender.wait(timeOut_); // for the msg ack stanza

    // two received msg are expected after reconnect.
    QVERIFY(collectedMsgRhsList_.size() == 2);
    QCOMPARE(collectedMsgRhsList_.at(0).msg, "one");
    QCOMPARE(collectedMsgRhsList_.at(1).msg, "two");

    // ##################################
    // send a picture from user1 to user2
    // ##################################
    QSignalSpy spyDownloadFinished(interfaceRhs_->getInterface(), SIGNAL(signalDownloadFinished(QString)));

    // send picture from user1 to user2
    QList<QVariant> fileForUser2 {user2jid_, imageFileName_};
    interfaceLhs_->callDbusMethodWithArgument("sendFile", fileForUser2);

    spyMsgStateSender.wait(timeOut_);
    spyMsgStateReceiver.wait(timeOut_);

    QCOMPARE(spyDownloadFinished.count(), 1);
    QList<QVariant> spyArgumentsOfDownload = spyDownloadFinished.takeFirst();
    QString localPath = spyArgumentsOfDownload.at(0).toString();

    // quit clients
    interfaceLhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
    interfaceRhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
}

bool ClientComTest::destrcutiveVerfiyStateAndCountOfMsgStates(enum side theSide, const QString& msgIdFilter, QList<MsgIdState> msgsList)
{
    auto returnValue = true;

    QList<MsgIdState> list = (theSide == lhs) ? stateChangeMsgLhsList_ : stateChangeMsgRhsList_;

    qDebug() << "before:";
    for(auto msg: list)
    {
        qDebug() << "id: " << msg.msgId << ", state: " << msg.state;
    }


    list.erase(
                std::remove_if(
                    list.begin(),
                    list.end(),
                    [&msgIdFilter](const MsgIdState msg){return (msgIdFilter.compare(msg.msgId) != 0);}
                ),
            list.end()
            );

#if 0
    QMutableListIterator<MsgIdState> i(list);
    while (i.hasNext())
    {
        if (i.next().msgId.compare(msgIdFilter) != 0)
        {
            i.remove();
        }
    }
#endif

    qDebug() << "after:";
    for(auto msg: list)
    {
        qDebug() << "id: " << msg.msgId << ", state: " << msg.state;
    }


    if(list.count() != msgsList.count())
    {
        qDebug() << "collected list.count() is " << list.count() << " and test msgsList.count() is " << msgsList.count();
        returnValue = false;
    }

    if (returnValue == true)
    {
        int loop = 0;
        for(auto msg: list)
        {
            if (msg.state != msgsList.at(loop).state)
            {
                qDebug() << "collected msg.state is " << msg.state << " and test msgList.state is " << msgsList.at(loop).state;
                returnValue = false;
                break;
            }

            loop++;
        }
    }

    if (theSide == lhs)
    {
        stateChangeMsgLhsList_.clear();
    }
    else
    {
        stateChangeMsgRhsList_.clear();
    }

    return returnValue;
}

void ClientComTest::collectMsgStateLhsChanged(QString msgId, int state)
{
    qDebug() << "collectMsgStateLhsChanged: " << msgId << ", " << state;
    MsgIdState msgState{msgId, state};
    stateChangeMsgLhsList_.push_back(msgState);
}

void ClientComTest::collectMsgStateRhsChanged(QString msgId, int state)
{
    qDebug() << "collectMsgStateRhsChanged: " << msgId << ", " << state;
    MsgIdState msgState{msgId, state};
    stateChangeMsgRhsList_.push_back(msgState);
}

QTEST_GUILESS_MAIN(ClientComTest)


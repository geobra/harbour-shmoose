#include "RosterTest.h"
#include "DbusInterfaceWrapper.h"

#include <QSignalSpy>
#include <QDebug>

/*
 * Common Tests:
 * Connect, Disconnect, Reconnect
 *
 * Roster:
 * Add contact, delete contact, check status, avatar
 * Join room, retrieve room name, leave room
 *
 */

RosterTest::RosterTest() : ClientComTestCommon()
{

}

// Availabiliy: (0)unknown, (1)offline, (2)online

void RosterTest::addDeleteRosterEntryTest()
{
    connect(interfaceLhs_->getInterface(), SIGNAL(signalRosterEntry(QString, QString, int, int, QString, QString, bool)),
                     this, SLOT(collectRosterListLhs(QString, QString, int, int, QString, QString, bool)));

    connect(interfaceLhs_->getInterface(), SIGNAL(signalSubscriptionUpdated(int)), this, SLOT(collectSubscriptionChangesLhs(int)));

    // add user2 as contact to user1
    qDebug() << "try to add a user2";
    QString nameForUser2 = "funny name for user2";
    addContactTestCommon(interfaceLhs_, "user2@localhost" , nameForUser2);

    // check and loop over subsChanged signal until we get the 'both' subscription value
    QSignalSpy spySubscriptonChangedLhs(interfaceLhs_->getInterface(), SIGNAL(signalSubscriptionUpdated(int)));
    QSignalSpy spySubscriptonChangedRhs(interfaceRhs_->getInterface(), SIGNAL(signalSubscriptionUpdated(int)));
    spySubscriptonChangedLhs.wait(timeOut_);
    qDebug() << "subs changed count: " << spySubscriptonChangedLhs.count();
    QVERIFY(spySubscriptonChangedLhs.count() > 0);
    QList<QVariant> spyArgumentsOfSubs = spySubscriptonChangedLhs.takeFirst();
    int currentSubsState = spyArgumentsOfSubs.at(0).toInt();
    qDebug() << "subs state from signal: " << currentSubsState;

    unsigned int loopCount = 0;
    while (currentSubsState != 3 && loopCount < 20)
    {
        spySubscriptonChangedLhs.wait(timeOut_);
        spySubscriptonChangedRhs.wait(timeOut_);
        if (spySubscriptonChangedLhs.count() > 0)
        {
            spyArgumentsOfSubs = spySubscriptonChangedLhs.takeFirst();
            currentSubsState = spyArgumentsOfSubs.at(0).toInt();
            qDebug() << "subs state from signal: " << currentSubsState;
        }
        else
        {
            qDebug() << "timeout wait for subs changed";
        }

        loopCount++;
    }
    QVERIFY(loopCount < 20);


    // check accepted contact add, availability and status.
    QSignalSpy spyRosterListReceivedLhs(interfaceLhs_->getInterface(), SIGNAL(signalRosterListDone()));
    rosterListLhs_.clear();
    interfaceLhs_->callDbusMethodWithArgument("requestRosterList", QList<QVariant>());
    spyRosterListReceivedLhs.wait(timeOut_);

    for (auto rosterItem: rosterListLhs_)
    {
        qDebug() << "process roster: " << rosterItem.jid << ", sub: " << rosterItem.subscription << ", avail: " << rosterItem.availability ;

        QCOMPARE(rosterItem.name, nameForUser2);
        QCOMPARE(rosterItem.availability, 1); // online
        QCOMPARE(rosterItem.subscription, 3); // both
    }

    // set user 2 offline
    qDebug() << "disconnect user2";
    interfaceRhs_->callDbusMethodWithArgument("disconnectFromServer", QList<QVariant>());
    QSignalSpy spySignalConnectionChangedRhs(interfaceRhs_->getInterface(), SIGNAL(signalConnectionStateChanged()));
    spySignalConnectionChangedRhs.wait(timeOutConnect_);
    QCOMPARE(spySignalConnectionChangedRhs.count(), 1);

    // check user 2 is offline as seen from user1
    spyRosterListReceivedLhs.clear();
    rosterListLhs_.clear();
    interfaceLhs_->callDbusMethodWithArgument("requestRosterList", QList<QVariant>());
    spyRosterListReceivedLhs.wait(timeOut_);

    for (auto rosterItem: rosterListLhs_)
    {
        qDebug() << "process roster: " << rosterItem.jid << ", sub: " << rosterItem.subscription << ", avail: " << rosterItem.availability ;

        QCOMPARE(rosterItem.name, nameForUser2);
        //QCOMPARE(rosterItem.availability, 2); // offline. msg may come late. dont check it
        QCOMPARE(rosterItem.subscription, 3); // both
    }

    // add user3 as contact
    qDebug() << "try to add offline user3";
    QString nameForUser3 = "funny name for user3";
    addContactTestCommon(interfaceLhs_, "user3@localhost" , nameForUser3);

    // check _not_ accepted contact request, availability and status
    spySubscriptonChangedLhs.wait(timeOut_);
    rosterListLhs_.clear();
    interfaceLhs_->callDbusMethodWithArgument("requestRosterList", QList<QVariant>());
    spyRosterListReceivedLhs.wait(timeOut_);

    QCOMPARE(rosterListLhs_.count(), 2); // user 2 and user3 in roster
    for (auto rosterItem: rosterListLhs_)
    {
        qDebug() << "process roster: " << rosterItem.jid << ", sub: " << rosterItem.subscription << ", avail: " << rosterItem.availability ;
        if (rosterItem.name.compare(nameForUser2, Qt::CaseSensitive) == 0)
        {
            // QCOMPARE(rosterItem.availability, 2); // offline. msg may come late. dont check it
            QCOMPARE(rosterItem.subscription, 3); // both
        }

        if (rosterItem.name.compare(nameForUser3, Qt::CaseSensitive) == 0)
        {
            QCOMPARE(rosterItem.availability, 0); // unknown
            QCOMPARE(rosterItem.subscription, 0); // unknown
        }

    }

    // connect user3
    QString dbusServiceNameMhs = dbusServiceNameCommon_ + "mhs";
    DbusInterfaceWrapper* interfaceMhs_ = new DbusInterfaceWrapper(dbusServiceNameMhs, dbusObjectPath_, "", QDBusConnection::sessionBus(), this);
    QSignalSpy spySubscriptonChangedMhs(interfaceMhs_->getInterface(), SIGNAL(signalSubscriptionUpdated(int)));

    connectionTestCommon(interfaceMhs_, user3jid_, "user3");

    // request roster list of user3
    // only then, presenc information will be sent to a client!
    // https://xmpp.org/rfcs/rfc3921.html -> 7.3
    QSignalSpy spyRosterListReceivedMhs(interfaceMhs_->getInterface(), SIGNAL(signalNewRosterEntry()));
    interfaceMhs_->callDbusMethodWithArgument("requestRoster", QList<QVariant>());
    spyRosterListReceivedMhs.wait(timeOut_);


    // check accepted contact request, availability and status
    // https://xmpp.org/rfcs/rfc3921.html -> 8.1
    // no resent of the subscription request from the server!
    int currentSubsStateMhs = 0;
    unsigned int loopCountMhs = 0;

    while (currentSubsStateMhs != 3 && loopCountMhs < 5)
    {
        qDebug() << "subs changed count: " << spySubscriptonChangedMhs.count();

        spySubscriptonChangedLhs.wait(timeOut_);
        spySubscriptonChangedMhs.wait(timeOut_);
        if (spySubscriptonChangedMhs.count() > 0)
        {
            QList<QVariant> spyArgumentsOfSubsMhs = spySubscriptonChangedMhs.takeFirst();
            currentSubsStateMhs = spyArgumentsOfSubsMhs.at(0).toInt();
            qDebug() << "mhs subs state from signal: " << currentSubsStateMhs;
        }
        else
        {
            qDebug() << "mhs: timeout wait for subs changed";
        }

        loopCountMhs++;
    }
    QCOMPARE(currentSubsStateMhs, 0);

    // TODO. Offline user3 is in uers1 roster now without subscriptions
    // add ability to request only the subscription part again
    // extend the test here!

    // remove user2 as contact.
    QSignalSpy spyRosterChangedLhs(interfaceLhs_->getInterface(), SIGNAL(signalNewRosterEntry()));
    interfaceLhs_->callDbusMethodWithArgument("removeContact", QList<QVariant>{user2jid_});
    spyRosterChangedLhs.wait(timeOut_);

    spyRosterListReceivedLhs.clear();
    rosterListLhs_.clear();
    interfaceLhs_->callDbusMethodWithArgument("requestRosterList", QList<QVariant>());
    spyRosterListReceivedLhs.wait(timeOut_);

    for (auto rosterItem: rosterListLhs_)
    {
        qDebug() << "process roster: " << rosterItem.jid << ", sub: " << rosterItem.subscription << ", avail: " << rosterItem.availability ;

        QCOMPARE(rosterItem.name, nameForUser3);
        QCOMPARE(rosterItem.availability, 0);
        QCOMPARE(rosterItem.subscription, 0);
    }

    // add room
    QSignalSpy joinRoomLhs(interfaceLhs_->getInterface(), SIGNAL(signalRoomJoined(QString, QString)));
    interfaceLhs_->callDbusMethodWithArgument("joinRoom", QList<QVariant>{roomJid_, "testroom"});
    joinRoomLhs.wait(timeOut_);
    qDebug() << "join room count: " << joinRoomLhs.count();
    QVERIFY(joinRoomLhs.count() == 1);

    spyRosterListReceivedLhs.clear();
    rosterListLhs_.clear();
    interfaceLhs_->callDbusMethodWithArgument("requestRosterList", QList<QVariant>());
    spyRosterListReceivedLhs.wait(timeOut_);

    QCOMPARE(rosterListLhs_.count(), 2); // user3 and the room
    for (auto rosterItem: rosterListLhs_)
    {
        qDebug() << "process roster: " << rosterItem.jid << ", sub: " << rosterItem.subscription << ", avail: " << rosterItem.availability ;

        // TODO check for real room name. will be updated in a later stanza.
        if (rosterItem.name.compare(roomJid_, Qt::CaseSensitive) == 0)
        {
            QCOMPARE(rosterItem.availability, 1); // online
            //QCOMPARE(rosterItem.subscription, 0); // ignored on rooms
        }
    }

    // delete room
    // TODO implement signal after room is removed from roster instead of just the roster changed signal!
    spyRosterChangedLhs.clear();
    interfaceLhs_->callDbusMethodWithArgument("removeRoom", QList<QVariant>{roomJid_});
    spyRosterChangedLhs.wait(timeOut_);
    qDebug() << "remove room count: " << spyRosterChangedLhs.count();
    QVERIFY(spyRosterChangedLhs.count() == 1);

#if 0
    spyRosterListReceivedLhs.clear();
    rosterListLhs_.clear();
    interfaceLhs_->callDbusMethodWithArgument("requestRosterList", QList<QVariant>());
    spyRosterListReceivedLhs.wait(timeOut_);

    QCOMPARE(rosterListLhs_.count(), 1); // only user3
    for (auto rosterItem: rosterListLhs_)
    {
        qDebug() << "process roster: " << rosterItem.jid << ", sub: " << rosterItem.subscription << ", avail: " << rosterItem.availability ;
    }
#endif

    // quit clients
    interfaceLhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
    interfaceRhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
    interfaceMhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
}

void RosterTest::removeContactCommon(DbusInterfaceWrapper* interface, const QString& jid)
{
    interface->callDbusMethodWithArgument("removeContact", QList<QVariant>{jid});

    QSignalSpy spyRosterChanged(interface->getInterface(), SIGNAL(signalNewRosterEntry()));
    spyRosterChanged.wait(timeOut_);

    QVERIFY(spyRosterChanged.count() > 0);
}

void RosterTest::removeRoomCommon(DbusInterfaceWrapper* interface, const QString& jid)
{
    interface->callDbusMethodWithArgument("removeRoom", QList<QVariant>{jid});

    QSignalSpy spyRoomRemoved(interface->getInterface(), SIGNAL(signalMucRoomRemoved(QString)));
    spyRoomRemoved.wait(timeOut_);

    QCOMPARE(spyRoomRemoved.count(), 1);
    // TODO check if room name matches
}


void RosterTest::collectRosterListLhs(QString jid, QString name, int subscription, int availability, QString status, QString imagePath, bool isGroup)
{
    //qDebug() << "collectRosterListLhs: " << jid << ", " << name << ", " << subscription << ", " << availability << ", " << status << ", " << imagePath << ", " << isGroup;
    RosterItem ri{jid, name, subscription, availability, status, imagePath, isGroup};
    rosterListLhs_.push_back(ri);

    dumpRosterList("collectRosterListLhs:");
}

void RosterTest::collectSubscriptionChangesLhs(int subs)
{
    qDebug() << "subs change lhs: " << subs;
}

void RosterTest::dumpRosterList(const QString& title)
{
    qDebug() << title;
    for (auto item: rosterListLhs_)
    {
        qDebug() << "  *jid: " << item.jid << ", name: " << item.name << ", sub: " << item.subscription << ", avail: " << item.availability
                 << ", status: " << item.status << ", imagePath:" << item.image << ", group: " << item.isGroup;
    }
}

QTEST_GUILESS_MAIN(RosterTest)

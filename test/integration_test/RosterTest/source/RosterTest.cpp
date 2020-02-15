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

    // -----------------------------------------------------------
    // for a clean test, check for contacts and delete them if any
    // -----------------------------------------------------------

    // request roster from server
    QSignalSpy spyNewRosterEntryLhs(interfaceLhs_->getInterface(), SIGNAL(signalNewRosterEntry()));
    QSignalSpy spyNewRosterEntryRhs(interfaceRhs_->getInterface(), SIGNAL(signalNewRosterEntry()));
    interfaceLhs_->callDbusMethodWithArgument("requestRoster", QList<QVariant>());

    spyNewRosterEntryLhs.wait(timeOut_);
    // on travis, the inital roster list is empty
    //QCOMPARE(spyNewRosterEntry.count(), 1);

    // request received rosterlist to be send from client
    interfaceLhs_->callDbusMethodWithArgument("requestRosterList", QList<QVariant>());

    QSignalSpy spyRosterListReceivedLhs(interfaceLhs_->getInterface(), SIGNAL(signalRosterListDone()));
    spyRosterListReceivedLhs.wait(timeOut_);

    for (auto rosterItem: rosterListLhs_)
    {
        qDebug() << "remove contacts: " << rosterItem.jid;

        if (rosterItem.isGroup == true)
        {
            removeRoomCommon(interfaceLhs_, rosterItem.jid);
        }
        else
        {
            removeContactCommon(interfaceLhs_, rosterItem.jid);
        }

    }

    spyRosterListReceivedLhs.wait(timeOut_);
    spyNewRosterEntryRhs.wait(timeOut_);

    // verfiy rosterlist is empty
    qDebug() << "verify rosterlist is empty";
    rosterListLhs_.clear();
    spyRosterListReceivedLhs.clear();
    interfaceLhs_->callDbusMethodWithArgument("requestRosterList", QList<QVariant>());
    spyRosterListReceivedLhs.wait(timeOut_);
    dumpRosterList("verfiy rosterlist is empty: ");
    QCOMPARE(rosterListLhs_.count(), 0);


#if 0
    // reconnect rhs
    qDebug() << "reconnect user2";
    interfaceRhs_->callDbusMethodWithArgument("disconnectFromServer", QList<QVariant>());
    QSignalSpy spySignalConnectionChangedRhs(interfaceRhs_->getInterface(), SIGNAL(signalConnectionStateChanged()));
    spySignalConnectionChangedRhs.wait(timeOutConnect_);
    QCOMPARE(spySignalConnectionChangedRhs.count(), 1);

    spySignalConnectionChangedRhs.clear();
    interfaceRhs_->callDbusMethodWithArgument("reConnect", QList<QVariant>());
    spySignalConnectionChangedRhs.wait(timeOutConnect_);
    spySignalConnectionChangedRhs.wait(timeOutConnect_);
    QCOMPARE(spySignalConnectionChangedRhs.count(), 1);
#endif




    qDebug() << "try to add a user2";
    spyNewRosterEntryLhs.wait(3000);

    // add user2 as contact to user1
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
    rosterListLhs_.clear();
    interfaceLhs_->callDbusMethodWithArgument("requestRosterList", QList<QVariant>());
    spyRosterListReceivedLhs.wait(timeOut_);

    for (auto rosterItem: rosterListLhs_)
    {
        qDebug() << "process roster: " << rosterItem.jid << ", sub: " << rosterItem.subscription << ", avail: " << rosterItem.availability ;
#if 0
        QCOMPARE(rosterItem.name, nameForUser2);
        QCOMPARE(rosterItem.availability, 1); // online
        QCOMPARE(rosterItem.subscription, 3); // both
#endif

    }

    // remove user2 as contact.

    // set user 2 offline

    // add user2 as contact

    // check _not_ accepted contact request, availability and status

    // reconnect user2

    // check accepted contact request, availability and status

    // test same with user3




    // quit clients
    interfaceLhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
    interfaceRhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
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

QTEST_MAIN(RosterTest)

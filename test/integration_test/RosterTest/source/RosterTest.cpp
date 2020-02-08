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
    QObject::connect(interfaceLhs_->getInterface(), SIGNAL(signalRosterEntry(QString, QString, int, int, QString, QString, bool)),
                     this, SLOT(collectRosterListLhs(QString, QString, int, int, QString, QString, bool)));

    // -----------------------------------------------------------
    // for a clean test, check for contacts and delete them if any
    // -----------------------------------------------------------

    // request roster from server
    QSignalSpy spyNewRosterEntry(interfaceLhs_->getInterface(), SIGNAL(signalNewRosterEntry()));
    interfaceLhs_->callDbusMethodWithArgument("requestRoster", QList<QVariant>());

    spyNewRosterEntry.wait(timeOut_);
    // on travis, the inital roster list is empty
    //QCOMPARE(spyNewRosterEntry.count(), 1);

    // request received rosterlist to be send from client
    interfaceLhs_->callDbusMethodWithArgument("requestRosterList", QList<QVariant>());

    QSignalSpy spyRosterListReceived(interfaceLhs_->getInterface(), SIGNAL(signalRosterListDone()));
    spyRosterListReceived.wait(timeOut_);

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

    spyRosterListReceived.wait(timeOut_);

    // verfiy rosterlist is empty
    rosterListLhs_.clear();
    spyRosterListReceived.clear();
    interfaceLhs_->callDbusMethodWithArgument("requestRosterList", QList<QVariant>());
    spyRosterListReceived.wait(timeOut_);
    dumpRosterList("verfiy rosterlist is empty: ");
    QCOMPARE(rosterListLhs_.count(), 0);

#if 0
    // add user2 as contact to user1
    QString nameForUser2 = "funny name for user2";
    addContactTestCommon(interfaceLhs_, "user2@localhost" , nameForUser2);

    // disconnect and reconnect user2
    interfaceRhs_->callDbusMethodWithArgument("disconnectFromServer", QList<QVariant>());

    QSignalSpy spySignalConnectionChanged(interfaceRhs_->getInterface(), SIGNAL(signalConnectionStateChanged()));
    spySignalConnectionChanged.wait(timeOutConnect_);
    QCOMPARE(spySignalConnectionChanged.count(), 1);

    spySignalConnectionChanged.clear();
    interfaceRhs_->callDbusMethodWithArgument("reConnect", QList<QVariant>());
    spySignalConnectionChanged.wait(timeOutConnect_);
    spySignalConnectionChanged.wait(timeOutConnect_);
    QCOMPARE(spySignalConnectionChanged.count(), 1);

    // check accepted contact add, availability and status.
    interfaceLhs_->callDbusMethodWithArgument("requestRosterList", QList<QVariant>());
    spyRosterListReceived.wait(timeOut_);

    spyRosterListReceived.wait(10000); // need some time to exchange subscription messages

    for (auto rosterItem: rosterListLhs_)
    {
        qDebug() << "process roster: " << rosterItem.jid << ", sub: " << rosterItem.subscription << ", avail: " << rosterItem.availability ;
        QCOMPARE(rosterItem.name, nameForUser2);
        QCOMPARE(rosterItem.availability, 2); // online


    }
#endif

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
    qDebug() << "collectRosterListLhs: " << jid << ", " << name << ", " << subscription << ", " << availability << ", " << status << ", " << imagePath << ", " << isGroup;
    RosterItem ri{jid, name, subscription, availability, status, imagePath, isGroup};
    rosterListLhs_.push_back(ri);
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

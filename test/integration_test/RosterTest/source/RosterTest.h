#ifndef ROSTERTEST_H
#define ROSTERTEST_H

#include "ClientComtestCommon.h"

#include <QObject>
#include <QtTest/QtTest>

class DbusInterfaceWrapper;

class RosterTest : public ClientComTestCommon
{
    Q_OBJECT

public:
    RosterTest();

private slots:
    void addDeleteRosterEntryTest();

    void collectRosterListLhs(QString jid, QString name, int subscription, int availability, QString status, QString imagePath, bool isGroup);
    void collectSubscriptionChangesLhs(int subs);

private:
    void removeRoomCommon(DbusInterfaceWrapper* interface, const QString& jid);
    void removeContactCommon(DbusInterfaceWrapper* interface, const QString& jid);
    void dumpRosterList(const QString& title);

    DbusInterfaceWrapper* interfaceMhs_;

    struct RosterItem
    {
        QString jid;
        QString name;
        int subscription;
        int availability;
        QString status;
        QString image;
        bool isGroup;
    };

    QList<RosterItem> rosterListLhs_;

};

#endif // RosterTest

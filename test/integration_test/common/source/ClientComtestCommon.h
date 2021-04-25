#ifndef CLIENTCOMTESTCOMMON_H
#define CLIENTCOMTESTCOMMON_H

#include <QObject>
#include <QtTest/QtTest>

class DbusInterfaceWrapper;

class ClientComTestCommon : public QObject
{
    Q_OBJECT

public:
    ClientComTestCommon();

public slots:
    void receiveConnectedSignal(QString str);

protected slots:
    void initTestCase();

    void collectLatestMsgRhs(QString msgId, QString jid, QString msg);

private slots:
    void cleanupTestCase();

    void connectionTest();
    void requestRosterTest();
    void addContactTest();

protected:
    void connectionTestCommon(DbusInterfaceWrapper* interface, const QString& jid, const QString& pass);
    void requestRosterTestCommon(DbusInterfaceWrapper *interface, bool doCompare = true);
    void addContactTestCommon(DbusInterfaceWrapper *interface, const QString& jid, const QString& name, bool doCompare = true);

    void generatePicture();

    DbusInterfaceWrapper* interfaceLhs_;
    DbusInterfaceWrapper* interfaceRhs_;

    const QString user1jid_;
    const QString user2jid_;
    const QString user3jid_;
    const QString roomJid_;

    const QString dbusServiceNameCommon_;
    const QString dbusObjectPath_;

    const QString imageFileName_;

    const int timeOutConnect_;
    int timeOut_;

    struct MsgContent
    {
        QString msgId;
        QString jid;
        QString msg;
    };

    QList<MsgContent> collectedMsgRhsList_;
};

#endif // CLIENTCOMTESTCOMMON_H

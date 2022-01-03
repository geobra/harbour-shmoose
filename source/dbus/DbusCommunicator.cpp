#include "DbusCommunicator.h"

#include "Shmoose.h"
#include "RosterItem.h"
#include "MessageController.h"
#include "MessageHandler.h"
#include "LurchAdapter.h"
#include "DownloadManager.h"
#include "MucManager.h"
#include "GcmController.h"
//#include "Settings.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusError>

DbusCommunicator::DbusCommunicator(const QString &path, const QString &name, QObject * const parent) : QObject(parent), dbusObjectPath_(path), dbusServiceName_(name)
{
    if (!QDBusConnection::sessionBus().isConnected())
    {
        qDebug() << "Cannot connect to the D-Bus session bus.";
        exit(1);
    }

    if (!QDBusConnection::sessionBus().registerService(dbusServiceName_))
    {
        fprintf(stderr, "%s\n",
                qPrintable(QDBusConnection::sessionBus().lastError().message()));
        exit(1);
    }

    if (!QDBusConnection::sessionBus().registerObject(dbusObjectPath_, dbusServiceName_, this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignal))
    {
        fprintf(stderr, "%s\n",
                qPrintable(QDBusConnection::sessionBus().lastError().message()));
        exit(1);
    }

    // globally disable omemo implementation for testing.
    //Settings settings;
    //settings.setSoftwareFeatureOmemoEnabled(false);

    qDebug() << "QDbusConnection name: " << QDBusConnection::sessionBus().name() << ", env: " << qgetenv("DBUS_SESSION_BUS_ADDRESS");
}

void DbusCommunicator::setXmpClient(Shmoose* shmoose)
{
    shmoose_ = shmoose;
}

void DbusCommunicator::setupConnections()
{
    Persistence *persistence = shmoose_->getPersistence();
    MessageController *msgCtrl = persistence->getMessageController();
    GcmController* gcmCtrl = persistence->getGcmController();
    DownloadManager* downloadMgr = shmoose_->messageHandler_->downloadManager_;
    MessageHandler* msgHandler = shmoose_->messageHandler_;
    LurchAdapter* lurchAdapter = shmoose_->lurchAdapter_;
    MucManager* muma = shmoose_->mucManager_;
    RosterController* rc = shmoose_->getRosterController();

    connect(shmoose_, SIGNAL(connectionStateChanged()), this, SLOT(slotConnectionStateChanged()));
    connect(msgCtrl, SIGNAL(signalMessageReceived(QString, QString, QString)), this, SLOT(slotForwaredReceivedMsgToDbus(QString, QString, QString)));
    connect(msgCtrl, SIGNAL(signalMessageStateChanged(QString, int)), this, SLOT(slotForwardMsgStateToDbus(QString, int)));
    connect(gcmCtrl, SIGNAL(signalRoomMessageStateChanged(QString,QString,int)), this, SLOT(slotForwardRoomMsgStateToDbus(QString, QString, int)));
    connect(downloadMgr, SIGNAL(httpDownloadFinished(QString)), this, SLOT(slotForwardDownloadMsgToDbus(QString)));
    connect(msgHandler, SIGNAL(messageSent(QString)), this, SLOT(slotForwardMsgSentToDbus(QString)));
    connect(muma, SIGNAL(newGroupForContactsList(QString,QString)), this, SLOT(slotNewRoomJoin(QString, QString)));
    connect(muma, SIGNAL(removeGroupFromContactsList(QString)), this, SLOT(slotForwardMucRoomRemoved(QString)));
    connect(rc, SIGNAL(rosterListChanged()), this, SLOT(slotGotRosterEntry()));
    connect(rc, SIGNAL(subscriptionUpdated(RosterItem::Subscription)), this, SLOT(slotForwardSubscriptionUpdate(RosterItem::Subscription)));
    connect(lurchAdapter, SIGNAL(signalReceivedDeviceListOfJid(QString)), this, SLOT(slotForwardReceivedDeviceListOfJid(QString)));
}

bool DbusCommunicator::tryToConnect(const QString& jid, const QString& pass)
{
    qDebug() << "try to connect as " << jid << " with pass " << pass;

    shmoose_->mainConnect(jid, pass);

    return true;
}

void DbusCommunicator::slotConnectionStateChanged()
{
    qDebug() << "slotConnectionStateChanged!";

    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalConnectionStateChanged");
    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

bool DbusCommunicator::requestRoster()
{
    qDebug() << "requestRoster";
    RosterController* rc = shmoose_->getRosterController();
    rc->requestRoster();

    return true;
}

bool DbusCommunicator::addContact(const QString& jid, const QString& name)
{
    qDebug() << "addContact: jid: " << jid << ", name: " << name;
    RosterController* rc = shmoose_->getRosterController();
    rc->addContact(jid, name);

    return true;
}

void DbusCommunicator::slotGotRosterEntry()
{
    qDebug() << "newRosterEntry!";

    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalNewRosterEntry");
    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

bool DbusCommunicator::joinRoom(const QString& jid, const QString& name)
{
    qDebug() << "joinRoom: jid: " << jid << ", name: " << name;
    shmoose_->joinRoom(jid, name);

    return true;
}

bool DbusCommunicator::slotNewRoomJoin(QString jid, QString name)
{
    qDebug() << "slotNewRoomJoin!";

    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalRoomJoined");
    msg << jid;
    msg << name;

    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }

    return true;
}

bool DbusCommunicator::removeRoom(const QString& jid)
{
    qDebug() << "removeRoom: jid: " << jid;
    shmoose_->removeRoom(jid);

    return true;
}

bool DbusCommunicator::removeContact(const QString& jid)
{
    qDebug() << "removeContact: jid: " << jid;
    RosterController* rc = shmoose_->getRosterController();

    rc->removeContact(jid);

    return true;
}

bool DbusCommunicator::sendMsg(const QString& jid, const QString& msg)
{
    qDebug() << "sendMsg";
    shmoose_->sendMessage(jid, msg, "txt");

    return true;
}

bool DbusCommunicator::sendFile(const QString& jid, const QString& path)
{
    qDebug() << "sendFile";
    shmoose_->sendFile(jid, path);

    return true;
}


void DbusCommunicator::slotForwaredReceivedMsgToDbus(QString id, QString jid, QString message)
{
    //qDebug() << "id: " << id << ", jid: " << jid << ", msg: " << message;

    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalLatestMsg");
    msg << id;
    msg << jid;
    msg << message;

    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

void DbusCommunicator::slotForwardMsgStateToDbus(QString msgId, int msgState)
{
    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalMsgState");
    msg << msgId;
    msg << msgState;

    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

void DbusCommunicator::slotForwardRoomMsgStateToDbus(QString msgId, QString jid, int msgState)
{
    qDebug() << "slotForwardRoomMsgStateToDbus: msg state changed for msgId: " << msgId << ", jid: " << jid << ", state: " << msgState;

    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalRoomMsgState");
    msg << msgId;
    msg << jid;
    msg << msgState;

    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

void DbusCommunicator::slotForwardDownloadMsgToDbus(QString localPath)
{
    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalDownloadFinished");
    msg << localPath;

    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

void DbusCommunicator::slotForwardReceivedDeviceListOfJid(QString jid)
{
    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalReceivedDeviceListOfJid");
    msg << jid;

    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

void DbusCommunicator::slotForwardMsgSentToDbus(QString msgId)
{
    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalMsgSent");
    msg << msgId;

    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

bool DbusCommunicator::setCurrentChatPartner(QString jid)
{
    qDebug() << "setCurrentChatPartner: " << jid;

    shmoose_->setCurrentChatPartner(jid);

    return true;
}

bool DbusCommunicator::quitClient()
{
    qDebug() << "goodbye";

    // FIXME this produces a segfault in ~Shmoose!
    //QCoreApplication::quit();

    exit(0);

    return true;
}

bool DbusCommunicator::disconnectFromServer()
{
    qDebug() << "disconnectFromServer";

    shmoose_->mainDisconnect();

    return true;
}

bool DbusCommunicator::reConnect()
{
    qDebug() << "reConnect";

    shmoose_->reConnect();

    return true;
}

bool DbusCommunicator::requestRosterList()
{
    RosterController* rc = shmoose_->getRosterController();

    QList<RosterItem*> rosterList = rc->fetchRosterList();
    for (auto item: rosterList)
    {
        qDebug() << ", jid: " << item->getJid() << "name: " << item->getName() <<
                    ", subscription: " << item->getSubscription() << ", availability: " << item->getAvailability() <<
                    ", status" << item->getStatus() << ", image: " << item->getImagePath() << ", isGroup: " << item->isGroup();

        QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalRosterEntry");
        msg << item->getJid();
        msg << item->getName();
        msg << item->getSubscription();
        msg << item->getAvailability();
        msg << item->getStatus();
        msg << item->getImagePath();
        msg << item->isGroup();

        if(QDBusConnection::sessionBus().send(msg) == false)
        {
            qDebug() << "cant send message via dbus";
        }
    }

    QDBusMessage msg2 = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalRosterListDone");
    if(QDBusConnection::sessionBus().send(msg2) == false)
    {
        qDebug() << "cant send message via dbus";
    }


    return true;
}

bool DbusCommunicator::addForcePlainMsgForJid(const QString& jid)
{
    shmoose_->settings_->addForcePlainTextSending(jid);

    return true;
}

bool DbusCommunicator::rmForcePlainMsgForJid(const QString& jid)
{
    shmoose_->settings_->removeForcePlainTextSending(jid);

    return true;
}

void DbusCommunicator::slotForwardMucRoomRemoved(QString jid)
{
    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalMucRoomRemoved");
    msg << jid;

    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

void DbusCommunicator::slotForwardSubscriptionUpdate(RosterItem::Subscription sub)
{
    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalSubscriptionUpdated");
    msg << (int) sub;

    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

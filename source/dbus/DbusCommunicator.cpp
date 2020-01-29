#include "DbusCommunicator.h"

#include "Shmoose.h"
#include "RosterController.h"
#include "MessageController.h"
#include "MessageHandler.h"
#include "DownloadManager.h"
#include "MucManager.h"
#include "GcmController.h"

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

    connect(msgCtrl, SIGNAL(signalMessageReceived(QString, QString, QString)), this, SLOT(slotForwaredReceivedMsgToDbus(QString, QString, QString)));
    connect(msgCtrl, SIGNAL(signalMessageStateChanged(QString, int)), this, SLOT(slotForwardMsgStateToDbus(QString, int)));
    connect(gcmCtrl, SIGNAL(signalRoomMessageStateChanged(QString,QString,int)), this, SLOT(slotForwardRoomMsgStateToDbus(QString, QString, int)));
    connect(downloadMgr, SIGNAL(httpDownloadFinished(QString)), this, SLOT(slotForwardDownloadMsgToDbus(QString)));
    connect(msgHandler, SIGNAL(messageSent(QString)), this, SLOT(slotForwardMsgSentToDbus(QString)));
}

bool DbusCommunicator::tryToConnect(const QString& jid, const QString& pass)
{
    qDebug() << "try to connect as " << jid << " with pass " << pass;

    connect(shmoose_, SIGNAL(connectionStateChanged()), this, SLOT(slotClientConnected()));
    shmoose_->mainConnect(jid, pass);

    return true;
}

void DbusCommunicator::slotClientConnected()
{
    qDebug() << "slotClientConnected!";

    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalConnected");
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

    connect(rc, SIGNAL(rosterListChanged()), this, SLOT(slotGotRosterEntry()));

    return true;
}

bool DbusCommunicator::addContact(const QString& jid, const QString& name)
{
    qDebug() << "addContact: jid: " << jid << ", name: " << name;
    RosterController* rc = shmoose_->getRosterController();
    rc->addContact(jid, name);

    connect(rc, SIGNAL(rosterListChanged()), this, SLOT(slotGotRosterEntry()));

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

    MucManager* mm = shmoose_->mucManager_;
    connect(mm, SIGNAL(newGroupForContactsList(QString,QString)), this, SLOT(slotNewRoomJoin(QString, QString)));

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


bool DbusCommunicator::sendMsg(const QString& jid, const QString& msg)
{
    qDebug() << "sendMsg";
    shmoose_->sendMessage(jid, msg, "msg");

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

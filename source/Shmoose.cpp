#include "Shmoose.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include <QtConcurrent>
#include <QDateTime>
#include <QUrl>

#include <QTimer>

#include <QDebug>

#include <Swiften/Elements/DiscoInfo.h>
#include <Swiften/Elements/DiscoItems.h>

#include <Swiften/Base/IDGenerator.h>

#include "RosterContoller.h"
#include "Persistence.h"
#include "MessageController.h"

#include "ChatMarkers.h"
#include "ConnectionHandler.h"
#include "MessageHandler.h"
#include "HttpFileUploadManager.h"
#include "MucManager.h"
#include "DiscoInfoHandler.h"


#include "System.h"

Shmoose::Shmoose(Swift::NetworkFactories* networkFactories, QObject *parent) :
    QObject(parent),
    netFactories_(networkFactories),
    rosterController_(new RosterController(this)),
    persistence_(new Persistence(this)),
    settings_(new Settings(this)),
    connectionHandler_(new ConnectionHandler(this)),
    messageHandler_(new MessageHandler(persistence_, settings_, this)),
    httpFileUploadManager_(new HttpFileUploadManager(this)),
    mucManager_(new MucManager(this)),
    discoInfoHandler_(new DiscoInfoHandler(httpFileUploadManager_, this)),
    jid_(""), password_(""),
    version_("0.5.0")
{
    qApp->setApplicationVersion(version_);

    connect(connectionHandler_, SIGNAL(signalInitialConnectionEstablished()), this, SLOT(intialSetupOnFirstConnection()));

    connect(httpFileUploadManager_, SIGNAL(fileUploadedForJidToUrl(QString,QString,QString)),
            this, SLOT(sendMessage(QString,QString,QString)));

    connect(mucManager_, SIGNAL(newGroupForContactsList(QString,QString)), rosterController_, SLOT(addGroupAsContact(QString,QString)));
    connect(mucManager_, SIGNAL(removeGroupFromContactsList(QString)), rosterController_, SLOT(removeGroupFromContacts(QString)) );

    // send read notification if app gets active
    connect(this, SIGNAL(signalAppGetsActive(bool)), this, SLOT(sendReadNotification(bool)));

    // inform connectionHandler and messageHandler about app status
    connect(this, SIGNAL(signalAppGetsActive(bool)), connectionHandler_, SLOT(slotAppGetsActive(bool)));
    connect(this, SIGNAL(signalAppGetsActive(bool)), messageHandler_, SLOT(slotAppGetsActive(bool)));

    // proxy signal to qml ui
    connect(connectionHandler_, SIGNAL(signalHasInetConnection(bool)), this, SIGNAL(signalHasInetConnection(bool)));
    connect(connectionHandler_, SIGNAL(connectionStateChanged()), this, SIGNAL(connectionStateChanged()));

    // show errors to user
    connect(mucManager_, SIGNAL(signalShowMessage(QString,QString)), this, SIGNAL(signalShowMessage(QString,QString)));
    connect(rosterController_, SIGNAL(signalShowMessage(QString,QString)), this, SIGNAL(signalShowMessage(QString,QString)));

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotAboutToQuit()));
}

Shmoose::~Shmoose()
{
    qDebug() << "Shmoose::~Shmoose";

    if (connectionHandler_->isConnected())
    {
        softwareVersionResponder_->stop();

        delete tracer_;
        delete softwareVersionResponder_;
        delete client_;
    }
}

void Shmoose::slotAboutToQuit()
{
    if (connectionHandler_->isConnected())
    {
        client_->disconnect();
    }
}

void Shmoose::mainConnect(const QString &jid, const QString &pass)
{
    persistence_->openDatabaseForJid(jid);

    QString completeJid = jid + "/shmoose";

    // setup the xmpp client
    client_ = new Swift::Client(Swift::JID(completeJid.toStdString()), pass.toStdString(), netFactories_);
    client_->setAlwaysTrustCertificates();

    connectionHandler_->setupWithClient(client_);
    messageHandler_->setupWithClient(client_);

    tracer_ = new Swift::ClientXMLTracer(client_);

    // configure the xmpp client
    softwareVersionResponder_ = new Swift::SoftwareVersionResponder(client_->getIQRouter());
    softwareVersionResponder_->setVersion("Shmoose", version_.toStdString());
    softwareVersionResponder_->start();
    client_->setSoftwareVersion("Shmoose", version_.toStdString());

    // register capabilities
    Swift::DiscoInfo discoInfo;
    discoInfo.addIdentity(Swift::DiscoInfo::Identity("shmoose", "client", "phone"));

    // http://xmpp.org/extensions/xep-0184.html, MessageDeliveryReceiptsFeature
    discoInfo.addFeature(Swift::DiscoInfo::MessageDeliveryReceiptsFeature);

    // https://xmpp.org/extensions/xep-0333.html
    discoInfo.addFeature(ChatMarkers::chatMarkersIdentifier.toStdString());

    client_->getDiscoManager()->setDiscoInfo(discoInfo);

    // finaly try to connect
    client_->connect();

    // for saving on connection success
    if (settings_->getSaveCredentials() == true)
    {
        jid_ = jid;
        password_ = pass;
    }
}

void Shmoose::mainDisconnect()
{
    if (connectionState())
    {
        client_->disconnect();
    }
}

void Shmoose::intialSetupOnFirstConnection()
{
    // Request the roster
    rosterController_->setupWithClient(client_);
    rosterController_->requestRosterFromClient(client_);

    // pass the client pointer to the httpFileUploadManager
    httpFileUploadManager_->setupWithClient(client_);

    // init and setup discoInfoHandler
    discoInfoHandler_->setupWithClient(client_);

    // init and setup mucManager
    mucManager_->setupWithClient(client_);

    // Save account data
    settings_->setJid(jid_);
    settings_->setPassword(password_);
}

void Shmoose::setCurrentChatPartner(QString const &jid)
{
    persistence_->setCurrentChatPartner(jid);

    sendReadNotification(true);
}

QString Shmoose::getCurrentChatPartner()
{
    return persistence_->getCurrentChatPartner();
}

void Shmoose::sendMessage(QString const &toJid, QString const &message, QString const &type)
{
    bool isGroup = rosterController_->isGroup(toJid);
    messageHandler_->sendMessage(toJid, message, type, isGroup);
}

void Shmoose::sendFile(QString const &toJid, QString const &file)
{
    if (httpFileUploadManager_->requestToUploadFileForJid(file, toJid) == false)
    {
        qDebug() << "Shmoose::sendFile failed";
    }
}

void Shmoose::sendReadNotification(bool active)
{
    QString currentChatPartner = persistence_->getCurrentChatPartner();

    if (active == true && (! currentChatPartner.isEmpty()))
    {
        messageHandler_->sendDisplayedForJid(currentChatPartner);
    }
}

RosterController* Shmoose::getRosterController()
{
    return rosterController_;
}

Persistence* Shmoose::getPersistence()
{
    return persistence_;
}

Settings* Shmoose::getSettings()
{
    return settings_;
}

bool Shmoose::connectionState() const
{
    return connectionHandler_->isConnected();
}

QString Shmoose::getAttachmentPath()
{
    return System::getAttachmentPath();
}

void Shmoose::setHasInetConnection(bool connected)
{
    connectionHandler_->setHasInetConnection(connected);
}

void Shmoose::setAppIsActive(bool active)
{
    emit signalAppGetsActive(active);
}

QString Shmoose::getVersion()
{
    return version_;
}

void Shmoose::joinRoom(QString const &roomJid, QString const &roomName)
{
    Swift::JID jid(roomJid.toStdString());

    if (jid.isValid())
    {
        setCurrentChatPartner(roomJid); // this prevents notifications for each initial history message
        mucManager_->addRoom(jid, roomName);
    }
    else
    {
        emit signalShowMessage("Join room", "JID not valid!");
    }
}

void Shmoose::removeRoom(QString const &roomJid)
{
    mucManager_->removeRoom(roomJid);
}

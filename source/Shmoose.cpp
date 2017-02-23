#include "Shmoose.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include <QtConcurrent>
#include <QDateTime>
#include <QSettings>
#include <QUrl>

#include <QTimer>

#include <QDebug>

#include <Swiften/Elements/DiscoInfo.h>
#include <Swiften/Elements/DiscoItems.h>

#include <Swiften/Base/IDGenerator.h>

#include "EchoPayload.h"
#include "RosterContoller.h"
#include "Persistence.h"
#include "MessageController.h"

#include "HttpFileUploadManager.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"
#include "XmppPingController.h"
#include "ReConnectionHandler.h"
#include "IpHeartBeatWatcher.h"
#include "MucManager.h"

#include "System.h"

Shmoose::Shmoose(NetworkFactories* networkFactories, QObject *parent) :
    QObject(parent), connected_(false), initialConnectionSuccessfull_(false),
    hasInetConnection_(false), netFactories_(networkFactories),
    rosterController_(new RosterController(this)),
    persistence_(new Persistence(this)),
    httpFileUploadManager_(new HttpFileUploadManager(this)),
    downloadManager_(new DownloadManager()),
    xmppPingController_(new XmppPingController()),
    reConnectionHandler_(new ReConnectionHandler(30000, this)),
    ipHeartBeatWatcher_(new IpHeartBeatWatcher(this)),
    mucManager_(new MucManager(this)),
    jid_(""), password_(""),
    version_("0.3.0")
{
    connect(ipHeartBeatWatcher_, SIGNAL(triggered()), this, SLOT(tryStablishServerConnection()));
    connect(ipHeartBeatWatcher_, SIGNAL(finished()), ipHeartBeatWatcher_, SLOT(deleteLater()));
    ipHeartBeatWatcher_->start();

    connect(httpFileUploadManager_, SIGNAL(fileUploadedForJidToUrl(QString,QString,QString)),
            this, SLOT(sendMessage(QString,QString,QString)));

    connect(reConnectionHandler_, SIGNAL(canTryToReconnect()), this, SLOT(tryReconnect()));

    connect(mucManager_, SIGNAL(newGroupForContactsList(QString,QString)), rosterController_, SLOT(addGroupAsContact(QString,QString)));
    connect(mucManager_, SIGNAL(removeGroupFromContactsList(QString)), rosterController_, SLOT(removeGroupFromContacts(QString)) );

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotAboutToQuit()));
}

Shmoose::~Shmoose()
{
    qDebug() << "Shmoose::~Shmoose";

    ipHeartBeatWatcher_->stopWatching();
    ipHeartBeatWatcher_->terminate();

    if (connected_)
    {
        client_->removePayloadSerializer(&echoPayloadSerializer_);
        client_->removePayloadParserFactory(&echoPayloadParserFactory_);
        softwareVersionResponder_->stop();

        delete tracer_;
        delete softwareVersionResponder_;
        delete client_;
    }

    delete downloadManager_;
    delete xmppPingController_;
}

void Shmoose::slotAboutToQuit()
{
    if (connected_)
    {
        client_->disconnect();
    }
}

void Shmoose::mainConnect(const QString &jid, const QString &pass)
{
    persistence_->openDatabaseForJid(jid);

    QString completeJid = jid + "/shmoose";
    client_ = new Swift::Client(Swift::JID(completeJid.toStdString()), pass.toStdString(), netFactories_);
    client_->setAlwaysTrustCertificates();

    client_->onConnected.connect(boost::bind(&Shmoose::handleConnected, this));
    client_->onDisconnected.connect(boost::bind(&Shmoose::handleDisconnected, this, _1));

    client_->onMessageReceived.connect(boost::bind(&Shmoose::handleMessageReceived, this, _1));
    client_->onPresenceReceived.connect(boost::bind(&Shmoose::handlePresenceReceived, this, _1));
    client_->onPresenceChange.connect(boost::bind(&Shmoose::handlePresenceChanged, this, _1));

    // xep 198 stream management and roster operations
    client_->onStanzaAcked.connect(boost::bind(&Shmoose::handleStanzaAcked, this, _1));

    tracer_ = new Swift::ClientXMLTracer(client_);

    softwareVersionResponder_ = new Swift::SoftwareVersionResponder(client_->getIQRouter());
    softwareVersionResponder_->setVersion("Shmoose", version_.toStdString());
    softwareVersionResponder_->start();
    client_->setSoftwareVersion("Shmoose", version_.toStdString());

    client_->addPayloadParserFactory(&echoPayloadParserFactory_);
    client_->addPayloadSerializer(&echoPayloadSerializer_);

    client_->connect();

    // for saving on connection success
    if (checkSaveCredentials() == true)
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

void Shmoose::handleConnected()
{
    qDebug() << QTime::currentTime().toString() << "Shmoose::handleConnected";

    connected_ = true;
    emit connectionStateConnected();

    // register capabilities
    // http://xmpp.org/extensions/xep-0184.html, MessageDeliveryReceiptsFeature
    DiscoInfo discoInfo;
    discoInfo.addIdentity(DiscoInfo::Identity("shmoose", "client", "phone"));
    discoInfo.addFeature(DiscoInfo::MessageDeliveryReceiptsFeature);
    //client_->getDiscoManager()->setCapsNode("https://github.com/geobra/harbour-shmoose");
    client_->getDiscoManager()->setDiscoInfo(discoInfo);

    // only on a first connection. skip this on a reconnect event.
    if (initialConnectionSuccessfull_ == false)
    {
        reConnectionHandler_->setActivated();

        // Request the roster
        rosterController_->setClient(client_);
        rosterController_->requestRosterFromClient(client_);

        // request the discoInfo from server
        GetDiscoInfoRequest::ref discoInfoRequest = GetDiscoInfoRequest::create(JID(client_->getJID().getDomain()), client_->getIQRouter());
        discoInfoRequest->onResponse.connect(boost::bind(&Shmoose::handleServerDiscoInfoResponse, this, _1, _2));
        discoInfoRequest->send();

        // pass the client pointer to the httpFileUploadManager
        httpFileUploadManager_->setClient(client_);
        xmppPingController_->setClient(client_);
        mucManager_->setClient(client_);
        mucManager_->initialize();

        // Save account data
        QSettings settings;
        settings.setValue("authentication/jid", jid_);
        settings.setValue("authentication/password", password_);
    }

    client_->sendPresence(Presence::create("Send me a message"));

    initialConnectionSuccessfull_ = true;
}

void Shmoose::handleDisconnected(const boost::optional<ClientError>& error)
{
    connected_ = false;
    emit connectionStateDisconnected();

    if (error)
    {
        ClientError clientError = *error;
        Swift::ClientError::Type type = clientError.getType();
        qDebug() << "disconnet error: " << type;

        // trigger the reConnectionHandler to get back online if inet is available
        if (initialConnectionSuccessfull_)
        {
            reConnectionHandler_->isConnected(hasInetConnection_);
        }
    }
    else
    {
        qDebug() << "disconnect without error";
    }
}

void Shmoose::setCurrentChatPartner(QString const &jid)
{
    persistence_->setCurrentChatPartner(jid);
}

void Shmoose::sendMessage(QString const &toJid, QString const &message, QString const &type)
{
    Swift::Message::ref msg(new Swift::Message);
    Swift::JID receiverJid(toJid.toStdString());

    Swift::IDGenerator idGenerator;
    std::string msgId = idGenerator.generateID();

    msg->setFrom(JID(client_->getJID()));
    msg->setTo(receiverJid);
    msg->setID(msgId);
    msg->setBody(message.toStdString());

    Swift::Message::Type messagesTyp = Swift::Message::Chat;
    if (rosterController_->isGroup(toJid))
    {
        messagesTyp = Swift::Message::Groupchat;
    }
    msg->setType(messagesTyp);

    msg->addPayload(boost::make_shared<DeliveryReceiptRequest>());

    client_->sendMessage(msg);
    persistence_->addMessage( (Swift::Message::Groupchat == messagesTyp) ? true : false,
                             QString::fromStdString(msgId),
                             QString::fromStdString(receiverJid.toBare().toString()),
                             QString::fromStdString(receiverJid.getResource()),
                             message, type, 0);
    unAckedMessageIds_.push_back(QString::fromStdString(msgId));
}

void Shmoose::sendFile(QString const &toJid, QString const &file)
{
    if (httpFileUploadManager_->requestToUploadFileForJid(file, toJid) == false)
    {
        qDebug() << "Shmoose::sendFile failed";
    }
}

void Shmoose::handlePresenceReceived(Presence::ref presence)
{
    // Automatically approve subscription requests
    // FIXME show to user and let user decide
    if (presence->getType() == Swift::Presence::Subscribe)
    {
        Swift::Presence::ref response = Swift::Presence::create();
        response->setTo(presence->getFrom());
        response->setType(Swift::Presence::Subscribed);
        client_->sendPresence(response);
    }
}

void Shmoose::handlePresenceChanged(Presence::ref presence)
{
    qDebug() << "handlePresenceChanged: type: " << QString::fromStdString(presence->getFrom());

    Swift::JID jid = presence->getFrom();
    QString status = ""; //QString::fromStdString(presence->getStatus());

    if (jid.isValid())
    {
        RosterItem::Availability availability = RosterItem::AVAILABILITY_ONLINE;

        if (presence->getType() == Swift::Presence::Unavailable
                || presence->getType() == Swift::Presence::Error
                || presence->getType() == Swift::Presence::Probe
                )
        {
            availability = RosterItem::AVAILABILITY_OFFLINE;
        }

        rosterController_->handleUpdateFromPresence(jid, status, availability);
    }
}

void Shmoose::handleStanzaAcked(Stanza::ref stanza)
{
    qDebug() << "Shmoose::handleStanzaAcked " << QString::fromStdString(stanza->getID());

    QMutableStringListIterator i(unAckedMessageIds_);
    while (i.hasNext())
    {
        QString value = i.next();
        if (value.compare(QString::fromStdString(stanza->getID())) == 0)
        {
            i.remove();
            persistence_->markMessageAsSentById(value);
        }
    }
}

void Shmoose::handleMessageReceived(Message::ref message)
{
    //std::cout << "handleMessageReceived: jid: " << message->getFrom() << ", bare: " << message->getFrom().toBare().toString() << ", resource: " << message->getFrom().getResource() << std::endl;

    std::string fromJid = message->getFrom().toBare().toString();
    boost::optional<std::string> fromBody = message->getBody();

    if (fromBody)
    {
        std::string body = *fromBody;
        QString theBody = QString::fromStdString(body);

        QString type = "txt";

        if (QUrl(theBody).isValid()) // it's an url
        {
            QStringList knownImageTypes = ImageProcessing::getKnownImageTypes();
            QString bodyEnd = theBody.trimmed().right(3); // url ends with an image type
            if (knownImageTypes.contains(bodyEnd))
            {
                type = "image";
                downloadManager_->doDownload(QUrl(theBody));
            }
        }

        bool isGroupMessage = false;
        if (message->getType() == Swift::Message::Groupchat)
        {
            isGroupMessage = true;
        }

        persistence_->addMessage(isGroupMessage,
                                 QString::fromStdString(message->getID()),
                                 QString::fromStdString(fromJid),
                                 QString::fromStdString(message->getFrom().getResource()),
                                 theBody, type, 1 );
    }

    // XEP 0184
    if (message->getPayload<DeliveryReceiptRequest>())
    {
        // send message receipt
        Message::ref receiptReply = boost::make_shared<Message>();
        receiptReply->setFrom(message->getTo());
        receiptReply->setTo(message->getFrom());

        boost::shared_ptr<DeliveryReceipt> receipt = boost::make_shared<DeliveryReceipt>();
        receipt->setReceivedID(message->getID());
        receiptReply->addPayload(receipt);
        client_->sendMessage(receiptReply);
    }

    // MUC
    // Examples/MUCListAndJoin/MUCListAndJoin.cpp
    if (message->getPayload<MUCInvitationPayload>())
    {
        qDebug() << "its a muc inventation!!!";
        MUCInvitationPayload::ref mucInventation = message->getPayload<MUCInvitationPayload>();

        Swift::JID roomJid = mucInventation->getJID();
        mucManager_->addRoom(roomJid, "fooBarRoom");
    }

    // mark sent msg as received
    DeliveryReceipt::ref rcpt = message->getPayload<DeliveryReceipt>();
    if (rcpt)
    {
        std::string recevideId = rcpt->getReceivedID();
        if (recevideId.length() > 0)
        {
            persistence_->markMessageAsReceivedById(QString::fromStdString(recevideId));
        }
    }
}

void Shmoose::handleServerDiscoInfoResponse(boost::shared_ptr<DiscoInfo> info, ErrorPayload::ref error)
{
    //qDebug() << "Shmoose::handleServerDiscoInfoResponse";
    const std::string httpUpload = "urn:xmpp:http:upload";

    if (!error)
    {
        if (info->hasFeature(httpUpload))
        {
            qDebug() << "has feature urn:xmpp:http:upload";
            //severHasFeatureHttpUpload = true;
            httpFileUploadManager_->setSeverHasFeatureHttpUpload(true);

            foreach (Swift::Form::ref form, info->getExtensions())
            {
                if (form)
                {
                    //qDebug() << "form: " << QString::fromStdString((*form).getFormType());
                    if ((*form).getFormType() == httpUpload)
                    {
                        Swift::FormField::ref formField = (*form).getField("max-file-size");
                        if (formField)
                        {
                            unsigned int maxFileSize = std::stoi((*formField).getTextSingleValue());
                            qDebug() << QString::fromStdString((*formField).getName()) << " val: " << maxFileSize;
                            httpFileUploadManager_->setMaxFileSize(maxFileSize);
                        }
                    }
                }
            }
        }
    }
}

void Shmoose::tryStablishServerConnection()
{
    qDebug() << QTime::currentTime().toString() << " Shmoose::tryStablishServerConnection. clientActive: " << client_->isActive() ;

    if (hasInetConnection_ == true
            && client_->isActive() == true
            && appIsActive_ == false /* connection wont be droped if app is in use */
            )
    {
        xmppPingController_->doPing();
    }
    else
    {
        // test to trigger a reconnect if not connected
        reConnectionHandler_->isConnected(hasInetConnection_);
    }
}

void Shmoose::tryReconnect()
{
    qDebug() << QTime::currentTime().toString() << "Shmoose::tryReconnect";

    if (initialConnectionSuccessfull_ == true && hasInetConnection_ == true)
    {
        // try to disconnect the old session fromn before network disturbtion
        client_->disconnect();

        // try new connect
        client_->connect();
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

bool Shmoose::connectionState() const
{
    return connected_;
}

bool Shmoose::checkSaveCredentials()
{
    bool save = false;

    QSettings settings;
    save = settings.value("authentication/saveCredentials", false).toBool();

    return save;
}

void Shmoose::saveCredentials(bool save)
{
    QSettings settings;
    settings.setValue("authentication/saveCredentials", save);
}


QString Shmoose::getJid()
{
    QString returnValue = "";

    QSettings settings;
    if(settings.value("authentication/jid").toString() != "NOT_SET")
    {
        returnValue = settings.value("authentication/jid").toString();
    }

    return returnValue;
}

QString Shmoose::getPassword()
{
    QString returnValue = "";

    QSettings settings;
    if(settings.value("authentication/password").toString() != "NOT_SET")
    {
        returnValue = settings.value("authentication/password").toString();
    }

    return returnValue;
}

QString Shmoose::getAttachmentPath()
{
    return System::getAttachmentPath();
}

void Shmoose::setHasInetConnection(bool connected)
{
    hasInetConnection_ = connected;
    reConnectionHandler_->isConnected(connected);
}

void Shmoose::setAppIsActive(bool active)
{
    appIsActive_ = active;
}

QString Shmoose::getVersion()
{
    return version_;
}

void Shmoose::joinRoom(QString const &roomJid, QString const &roomName)
{
    Swift::JID jid(roomJid.toStdString());
    mucManager_->addRoom(jid, roomName);
}

void Shmoose::removeRoom(QString const &roomJid)
{
    mucManager_->removeRoom(roomJid);
}

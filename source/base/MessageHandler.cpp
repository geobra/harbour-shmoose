#include "MessageHandler.h"
#include "Persistence.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"
#include "ChatMarkers.h"
#include "StanzaIdPayload.h"
#include "XmlProcessor.h"
#include "RosterController.h"
#include <Swiften/Elements/CarbonsReceived.h>
#include <Swiften/Elements/CarbonsSent.h>
#include <Swiften/Elements/Forwarded.h>

#include <QUrl>

#include <QDebug>

MessageHandler::MessageHandler(Persistence *persistence, Settings * settings, RosterController* rosterController, QObject *parent) : QObject(parent),
    client_(nullptr), persistence_(persistence), settings_(settings),
    downloadManager_(new DownloadManager(this)),
    chatMarkers_(new ChatMarkers(persistence_, rosterController, this)),
    appIsActive_(true), unAckedMessageIds_()
{

}

void MessageHandler::setupWithClient(Swift::Client* client)
{  
    if (client != NULL)
    {
        client_ = client;

        client_->onMessageReceived.connect(boost::bind(&MessageHandler::handleMessageReceived, this, _1));

        // xep 198 stream management and roster operations
        client_->onStanzaAcked.connect(boost::bind(&MessageHandler::handleStanzaAcked, this, _1));

        chatMarkers_->setupWithClient(client_);
    }
}

void MessageHandler::handleStanzaAcked(Swift::Stanza::ref stanza)
{
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

void MessageHandler::handleMessageReceived(Swift::Message::ref message)
{
    //std::cout << "handleMessageReceived: jid: " << message->getFrom() << ", bare: " << message->getFrom().toBare().toString() << ", resource: " << message->getFrom().getResource() << std::endl;

    std::string fromJid = message->getFrom().toBare().toString();

    // XEP 280
    Swift::JID toJID;
    bool sentCarbon = false;
    // If this is a carbon message, we need to retrieve the actual content
    if (settings_->getJid().compare(QString::fromStdString(fromJid)) == 0) {
        Swift::CarbonsReceived::ref carbonsReceived;
        Swift::CarbonsSent::ref carbonsSent;
        Swift::Forwarded::ref forwarded;
        Swift::Message::ref forwardedMessage;
        if ((carbonsReceived = message->getPayload<Swift::CarbonsReceived>()) &&
                (forwarded = carbonsReceived->getForwarded()) &&
                (forwardedMessage = std::dynamic_pointer_cast<Swift::Message>(forwarded->getStanza()))) {
            // It is the carbon of a message that we received
            message = forwardedMessage;
            fromJid = message->getFrom().toBare().toString();
        }
        else if ((carbonsSent = message->getPayload<Swift::CarbonsSent>()) &&
                 (forwarded = carbonsSent->getForwarded()) &&
                 (forwardedMessage = std::dynamic_pointer_cast<Swift::Message>(forwarded->getStanza()))) {
            // It is the carbon of a message that we sent
            message = forwardedMessage;
            toJID = forwardedMessage->getTo();
            sentCarbon = true;
        }
    }

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

        QString messageId = QString::fromStdString(message->getID());
        if (messageId.length() == 0)
        {
            // No message id, try xep 0359
            std::shared_ptr<StanzaIdPayload> stanzaId = message->getPayload<StanzaIdPayload>();
            if (stanzaId != nullptr)
            {
                messageId = QString::fromStdString(stanzaId->getId());
            }
        }

        if (!sentCarbon)
        {
            persistence_->addMessage(messageId,
                                     QString::fromStdString(fromJid),
                                     QString::fromStdString(message->getFrom().getResource()),
                                     theBody, type, 1 );
        } else
        {
            persistence_->addMessage(messageId,
                                     QString::fromStdString(toJID.toBare().toString()),
                                     QString::fromStdString(toJID.getResource()),
                                     theBody, type, 0 );
        }

        // xep 0333
        QString currentChatPartner = persistence_->getCurrentChatPartner();
        qDebug() << "fromJid: " << QString::fromStdString(fromJid) << "current: " << currentChatPartner << ", isGroup: " << isGroupMessage << ", appActive? " << appIsActive_;
        if ( (currentChatPartner.compare(QString::fromStdString(fromJid)) == 0) &&     // immediatelly send read notification if sender is current chat partner
             (appIsActive_ == true)                                                     // but only if app is active
             )
        {
            this->sendDisplayedForJid(currentChatPartner);
        }
    }

    // XEP 0184
    if (message->getPayload<Swift::DeliveryReceiptRequest>())
    {
        // send message receipt
        Swift::Message::ref receiptReply = std::make_shared<Swift::Message>();
        receiptReply->setFrom(message->getTo());
        receiptReply->setTo(message->getFrom());

        std::shared_ptr<Swift::DeliveryReceipt> receipt = std::make_shared<Swift::DeliveryReceipt>();
        receipt->setReceivedID(message->getID());
        receiptReply->addPayload(receipt);
        client_->sendMessage(receiptReply);
    }
}

void MessageHandler::sendMessage(QString const &toJid, QString const &message, QString const &type, bool isGroup)
{
    Swift::Message::ref msg(new Swift::Message);
    Swift::JID receiverJid(toJid.toStdString());

    Swift::IDGenerator idGenerator;
    std::string msgId = idGenerator.generateID();

    msg->setFrom(Swift::JID(client_->getJID()));
    msg->setTo(receiverJid);
    msg->setID(msgId);
    msg->setBody(message.toStdString());

    if(type == "image")
    {
        QString outOfBandElement("");
        outOfBandElement.append("<x xmlns=\"jabber:x:oob\">");
        outOfBandElement.append("<url>");
        outOfBandElement.append(message);
        outOfBandElement.append("</url>");
        outOfBandElement.append("</x>");

        std::shared_ptr<Swift::RawXMLPayload> outOfBand =
                std::make_shared<Swift::RawXMLPayload>(outOfBandElement.toStdString());
        msg->addPayload(outOfBand);
    }

    Swift::Message::Type messagesTyp = Swift::Message::Chat;
    if (isGroup == true)
    {
        messagesTyp = Swift::Message::Groupchat;
    }
    msg->setType(messagesTyp);

    msg->addPayload(std::make_shared<Swift::DeliveryReceiptRequest>());

    // add chatMarkers stanza
    msg->addPayload(std::make_shared<Swift::RawXMLPayload>(ChatMarkers::getMarkableString().toStdString()));

    client_->sendMessage(msg);
    persistence_->addMessage( QString::fromStdString(msgId),
                              QString::fromStdString(receiverJid.toBare().toString()),
                              QString::fromStdString(receiverJid.getResource()),
                              message, type, 0);
    unAckedMessageIds_.push_back(QString::fromStdString(msgId));

    emit messageSent(QString::fromStdString(msgId));
}

void MessageHandler::sendDisplayedForJid(const QString &jid)
{
    if(settings_->getSendReadNotifications())
    {
        chatMarkers_->sendDisplayedForJid(jid);
    }
}

void MessageHandler::slotAppGetsActive(bool active)
{
    appIsActive_ = active;
}

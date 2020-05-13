#include "MessageHandler.h"
#include "Persistence.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"
#include "ChatMarkers.h"
#include "XmlProcessor.h"
#include "RosterController.h"
#include <Swiften/Elements/CarbonsReceived.h>
#include <Swiften/Elements/CarbonsSent.h>
#include <Swiften/Elements/Forwarded.h>

#include <QUrl>

#include <QDebug>

MessageHandler::MessageHandler(Persistence *persistence, Settings * settings, RosterController* rosterController, QObject *parent) : QObject(parent),
    client_(NULL), persistence_(persistence), settings_(settings),
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

        // simple 'message archive management' for fetched muc history
        client_->onDataRead.connect(boost::bind(&MessageHandler::handleDataReceived, this, _1));

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

void MessageHandler::handleDataReceived(Swift::SafeByteArray data)
{
    std::string nodeData = Swift::safeByteArrayToString(data);
    QString qData = QString::fromStdString(nodeData);

    QString xmlnsTag = XmlProcessor::getContentInTag("result", "xmlns", qData);

    if ( xmlnsTag.contains("urn:xmpp:mam:1", Qt::CaseInsensitive) == true )
    {
        QString archivedMsg = XmlProcessor::getChildFromNode("message", qData);
        if (! archivedMsg.isEmpty() )
        {
            QString body = XmlProcessor::getContentInTag("message", "body", archivedMsg);
            if (! body.isEmpty()) // only messages with a body text will be processed
            {
                QString id = XmlProcessor::getContentInTag("message", "id", archivedMsg);
                QString archiveJid = XmlProcessor::getContentInTag("message", "from", archivedMsg);

                QString msgOwnerJid = XmlProcessor::getContentInTag("item", "jid", archivedMsg);
                QString msgOwnerBareJid = QString::fromStdString(Swift::JID(msgOwnerJid.toStdString()).toBare().toString());
                QString myBareJid = QString::fromStdString(client_->getJID().toBare().toString());

                Swift::JID jid = Swift::JID(archiveJid.toStdString());
                QString from = QString::fromStdString(jid.getResource());
                QString bareJid = QString::fromStdString(jid.toBare());

                qDebug() << "muc mam! body: " << body << ", id: " << id << " , jid: " << bareJid << ", from: " << from;

                if ( (! id.isEmpty()) && (! bareJid.isEmpty()) && (! from.isEmpty()) )
                {
                    // FIXME currently, this is only for group messages!
                    // FIXME has to also check content type. Currently we assume txt

                    bool isGroupMessage = true;
                    int direction = 1; // incoming

                    // if msg jid is same as my jid, then the msg was from me.
                    if (msgOwnerBareJid.compare(myBareJid, Qt::CaseInsensitive) == 0)
                    {
                        direction = 0; // outgoing
                    }

                    persistence_->addMessage(isGroupMessage, id, bareJid, from, body, "txt", direction );
                }
            }
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

	if (!sentCarbon) {
		persistence_->addMessage(isGroupMessage,
					 QString::fromStdString(message->getID()),
					 QString::fromStdString(fromJid),
					 QString::fromStdString(message->getFrom().getResource()),
					 theBody, type, 1 );
	} else {
		persistence_->addMessage(isGroupMessage,
					 QString::fromStdString(message->getID()),
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
    persistence_->addMessage( (Swift::Message::Groupchat == messagesTyp) ? true : false,
                              QString::fromStdString(msgId),
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

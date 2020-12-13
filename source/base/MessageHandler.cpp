#include "MessageHandler.h"
#include "Persistence.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"
#include "ChatMarkers.h"
#include "StanzaIdPayload.h"
#include "XmlProcessor.h"
#include "RosterController.h"
#include "Omemo.h"
#include "XmppMessageParserClient.h"

#include <Swiften/Elements/CarbonsReceived.h>
#include <Swiften/Elements/CarbonsSent.h>
#include <Swiften/Elements/Forwarded.h>

#include <QUrl>
#include <QDomDocument>
#include <QDebug>

MessageHandler::MessageHandler(Persistence *persistence, Settings * settings, RosterController* rosterController, Omemo* omemo, QObject *parent) : QObject(parent),
    client_(nullptr), persistence_(persistence), omemo_(omemo), settings_(settings),
    downloadManager_(new DownloadManager(this)),
    chatMarkers_(new ChatMarkers(persistence_, rosterController, this)),
    xmppMessageParserClient_(new XMPPMessageParserClient()),
    appIsActive_(true), unAckedMessageIds_()
{
    connect(omemo_, SIGNAL(rawMessageStanzaForSending(QString)), this, SLOT(sendRawMessageStanza(QString)));
}

void MessageHandler::setupWithClient(Swift::Client* client)
{  
    if (client != nullptr)
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

void MessageHandler::handleMessageReceived(Swift::Message::ref aMessage)
{
    //std::cout << "handleMessageReceived: jid: " << message->getFrom() << ", bare: " << message->getFrom().toBare().toString() << ", resource: " << message->getFrom().getResource() << std::endl;
    Swift::Message* message{nullptr};
    std::string fromJid{};

    //unsigned int msgEncrypted{0};

    // check if received aMessage is encrypted
    QString qMsg = getSerializedStringFromMessage(aMessage);
    if (isEncryptedMessage(qMsg))
    {
        //msgEncrypted = 1;

        std::string dmsg = omemo_->messageDecrypt(qMsg.toStdString());
        QString decryptedMessage = QString::fromStdString(dmsg);

        if (decryptedMessage.isEmpty())
        {
            // something went wrong on the decryption. use original encrypted aMessage and go one...
            message = &(*aMessage);
        }
        else
        {
            decryptedMessage = "<stream xmlns='http://etherx.jabber.org/streams'>" + decryptedMessage;

            // create a xmpp parser
            Swift::FullPayloadParserFactoryCollection factories;
            Swift::PlatformXMLParserFactory xmlParserFactory;
            Swift::XMPPParser parser(xmppMessageParserClient_, &factories, &xmlParserFactory);

            // parse the decrypted string
            if (parser.parse(decryptedMessage.toStdString()))
            {
                // catch pointer from parsed decrypted string as message pointer
                message = xmppMessageParserClient_->getMessagePtr();
                if (message == nullptr)
                {
                    message = &(*aMessage);
                }
            }
            else
            {
                // failure on xml parsing. use original message
                message = &(*aMessage);
            }
        }

    }
    else
    {
        message = &(*aMessage);
    }



    fromJid = message->getFrom().toBare().toString();

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
            message = &(*forwardedMessage);
            fromJid = message->getFrom().toBare().toString();
        }
        else if ((carbonsSent = message->getPayload<Swift::CarbonsSent>()) &&
                 (forwarded = carbonsSent->getForwarded()) &&
                 (forwardedMessage = std::dynamic_pointer_cast<Swift::Message>(forwarded->getStanza()))) {
            // It is the carbon of a message that we sent
            message = &(*forwardedMessage);
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
    // FIXME use smart pointer!
    Swift::Message::ref msg(new Swift::Message);
    Swift::JID receiverJid(toJid.toStdString());

    Swift::IDGenerator idGenerator;
    std::string msgId = idGenerator.generateID();

    msg->setFrom(Swift::JID(client_->getJID()));
    msg->setTo(receiverJid);
    msg->setID(msgId);

    Swift::Message::Type messagesTyp = Swift::Message::Chat;
    if (isGroup == true)
    {
        messagesTyp = Swift::Message::Groupchat;
    }

    msg->setType(messagesTyp);
    msg->setBody(message.toStdString());

// -----------------

    // FIXME for now, always try to encrypt the msg
    QString qMsg = getSerializedStringFromMessage(msg);
    std::string cryptMessage = omemo_->messageEncryptIm(qMsg.toStdString());
    QString encryptedPayload = XmlProcessor::getChildFromNode("encrypted", QString::fromStdString(cryptMessage));

    Swift::RawXMLPayload::ref encPayload(new Swift::RawXMLPayload(encryptedPayload.toStdString() + "<encryption xmlns=\"urn:xmpp:eme:0\" namespace=\"eu.siacs.conversations.axolotl\" name=\"OMEMO\" /><store xmlns=\"urn:xmpp:hints\" />"));
    msg->addPayload(encPayload);

    // TODO finally, remove the plain text body, if encryption was ok
    msg->removePayloadOfSameType(std::make_shared<Swift::Body>());



// -----------------

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

void MessageHandler::sendRawMessageStanza(QString str)
{
    // FIXME test this!
    QString msg = "<stream xmlns='http://etherx.jabber.org/streams'>" + str;

    // create a xmpp parser
    Swift::FullPayloadParserFactoryCollection factories;
    Swift::PlatformXMLParserFactory xmlParserFactory;
    Swift::XMPPParser parser(xmppMessageParserClient_, &factories, &xmlParserFactory);


    // parse the string
    if (parser.parse(msg.toStdString()))
    {
        // catch pointer from parsed decrypted string as message pointer
        Swift::Message* message = xmppMessageParserClient_->getMessagePtr();
        if (message != nullptr)
        {
            client_->sendMessage(std::make_shared<Swift::Message>(*message));
        }
    }
    else
    {
        // failure on xml parsing. use original message
        qDebug() << "failed to parse the msg!";
    }
}


QString MessageHandler::getSerializedStringFromMessage(Swift::Message::ref msg)
{
    Swift::FullPayloadSerializerCollection serializers_;
    Swift::XMPPSerializer xmppSerializer(&serializers_, Swift::ClientStreamType, true);
    Swift::SafeByteArray sba = xmppSerializer.serializeElement(msg);

    return QString::fromStdString(Swift::safeByteArrayToString(sba));
}

// FIXME move to omemo?
bool MessageHandler::isEncryptedMessage(const QString& xmlNode)
{
    bool returnValue = false;

    QDomDocument d;
    if (d.setContent(xmlNode) == true)
    {
        QDomNodeList nodeList = d.elementsByTagName("message");
        if (!nodeList.isEmpty())
        {
            //qDebug() << "found msg";
            QDomNodeList encList = d.elementsByTagName("encrypted");
            if (!encList.isEmpty())
            {
                //qDebug() << "found enc";
                returnValue = true;
            }
        }
    }

    return returnValue;
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

#include "MessageHandler.h"
#include "Persistence.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"
#include "ChatMarkers.h"
#include "StanzaIdPayload.h"
#include "XmlProcessor.h"
#include "RosterController.h"
#include "LurchAdapter.h"
#include "XmppMessageParserClient.h"

#include <Swiften/Elements/CarbonsReceived.h>
#include <Swiften/Elements/CarbonsSent.h>
#include <Swiften/Elements/Forwarded.h>

#include <QUrl>
#include <QDebug>
#include <QMimeDatabase>

MessageHandler::MessageHandler(Persistence *persistence, Settings * settings, RosterController* rosterController, LurchAdapter* lurchAdapter, QObject *parent) : QObject(parent),
    client_(nullptr), persistence_(persistence), lurchAdapter_(lurchAdapter), settings_(settings),
    downloadManager_(new DownloadManager(this)),
    chatMarkers_(new ChatMarkers(persistence_, rosterController, this)),
    xmppMessageParserClient_(new XMPPMessageParserClient()),
    appIsActive_(true), unAckedMessageIds_()
{
    connect(lurchAdapter_, SIGNAL(rawMessageStanzaForSending(QString)), this, SLOT(sendRawMessageStanza(QString)));
    connect(settings_, SIGNAL(askBeforeDownloadingChanged(bool)), this, SLOT(setAskBeforeDownloading(bool)));
    connect(downloadManager_, SIGNAL(httpDownloadFinished(QString)), this, SIGNAL(httpDownloadFinished(QString)));
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

        setAskBeforeDownloading(settings_->getAskBeforeDownloading());
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

    unsigned int security = 0;

    if (settings_->getSoftwareFeatureOmemoEnabled() == true)
    {
        auto success = lurchAdapter_->decryptMessageIfEncrypted(message);
        if (success == 0) // 0: success on decryption, 1: was not encrypted, 2: error during decryption.
        {
            security = 1;
        }
        else if (success == 2)
        {
            qDebug() << "handleMessageReceived: error during decryption).";
            QString cryptErrorMsg{tr("** Enrypted message could not be decrypted. Sorry. **")};
            message->setBody(cryptErrorMsg.toStdString());
        }
    }

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
        QUrl bodyUrl(theBody);
        bool isLink = false;

        QString type = "txt";
        QString messageId = QString::fromStdString(message->getID());

        if (bodyUrl.isValid() && bodyUrl.scheme().length()>0 ) // it's an url
        {
            std::vector< std::shared_ptr<Swift::RawXMLPayload> > xmlPayloads = message->getPayloads<Swift::RawXMLPayload>();
            for (std::vector<std::shared_ptr<Swift::RawXMLPayload>>::iterator it = xmlPayloads.begin() ; it != xmlPayloads.end(); ++it)
            {
                QString rawXml = QString::fromStdString((*it)->getRawXML());

                if(!XmlProcessor::getContentInElement("url", rawXml).isEmpty())
                {
                    isLink = true;
                    break;
                }
            }

            isLink = security == 1 ? theBody.startsWith("aesgcm://") : isLink;

            if(isLink)
            {
                type = QMimeDatabase().mimeTypeForFile(bodyUrl.fileName()).name();

                if(! askBeforeDownloading_)
                    downloadManager_->doDownload(bodyUrl, messageId); // keep the fragment in the sent message
            }
        }

        bool isGroupMessage = false;
        if (message->getType() == Swift::Message::Groupchat)
        {
            isGroupMessage = true;
        }

        if (messageId.length() == 0)
        {
            // No message id, try xep 0359
            std::shared_ptr<StanzaIdPayload> stanzaId = message->getPayload<StanzaIdPayload>();
            if (stanzaId != nullptr)
            {
                messageId = QString::fromStdString(stanzaId->getId());
            }

            // still empty?
            if (messageId.isEmpty() == true)
            {
                messageId = QString::number(QDateTime::currentMSecsSinceEpoch());
            }
        }

        if (!sentCarbon)
        {
            persistence_->addMessage(messageId,
                                     QString::fromStdString(fromJid),
                                     QString::fromStdString(message->getFrom().getResource()),
                                     theBody, type, 1, security);
        } else
        {
            persistence_->addMessage(messageId,
                                     QString::fromStdString(toJID.toBare().toString()),
                                     QString::fromStdString(toJID.getResource()),
                                     theBody, type, 0, security);
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
    unsigned int security = 0;

    if (settings_->getSoftwareFeatureOmemoEnabled() == true && message.startsWith("/lurch") == true)
    {
        std::vector<std::string> sl;
        for (auto string: message.split(" "))
        {
            if (string.compare("/lurch") != 0)
            {
                sl.push_back(string.toStdString());
            }
        }
        lurchAdapter_->callLurchCmd(sl);
    }
    else
    {
        Swift::Message::ref msg = std::make_shared<Swift::Message>();
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

        // add delivery request
        msg->addPayload(std::make_shared<Swift::DeliveryReceiptRequest>());

        // add chatMarkers stanza
        msg->addPayload(std::make_shared<Swift::RawXMLPayload>(ChatMarkers::getMarkableString().toStdString()));

        // exchange body by omemo stuff if applicable
        bool shouldSendMsgStanze{true};
        if ((settings_->getSoftwareFeatureOmemoEnabled() == true)
            && isGroup == false // for now, omemo is only supported on 1o1 messaging
            && (lurchAdapter_->isOmemoUser(toJid) == true) // the receipient client can handle omemo encryption
            && (! settings_->getSendPlainText().contains(toJid))) // no force for plain text msg in settings
        {
            bool success = lurchAdapter_->exchangePlainBodyByOmemoStanzas(msg);
            if (success == false)
            {
                // an exchange of the body stanza by an omemo stanza failed
                // either some de/encryption stuff failed, or the bundel is requested in background
                // don't send this msg, but put it in the database for chat markers to be set correct.
                // informs the user about real sending, receiving, reading.
                shouldSendMsgStanze = false;
            }
            else
            {
                security = 1;
            }
        }
        else // xep-0066. Only add the stanza on plain-text messages, as described in the xep-0454
        {  
            if(type.compare("txt", Qt::CaseInsensitive) != 0)   // XEP-0066
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
        }

        if (shouldSendMsgStanze == true)
        {
            client_->sendMessage(msg);
        }

        persistence_->addMessage( QString::fromStdString(msgId),
                                  QString::fromStdString(receiverJid.toBare().toString()),
                                  QString::fromStdString(receiverJid.getResource()),
                                  message, type, 0, security);
        unAckedMessageIds_.push_back(QString::fromStdString(msgId));

        emit messageSent(QString::fromStdString(msgId));
    }
}

void MessageHandler::sendRawMessageStanza(QString str)
{
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

            emit messageSent(QString::fromStdString(message->getID()));
        }
    }
    else
    {
        // failure on xml parsing. use original message
        qDebug() << "failed to parse the msg!";
    }
}

void MessageHandler::sendDisplayedForJid(const QString &jid)
{
    if(settings_->getSendReadNotifications())
    {
        chatMarkers_->sendDisplayedForJid(jid);
    }
}

void MessageHandler::downloadFile(const QString &str, const QString &msgId)
{
    downloadManager_->doDownload(QUrl(str), msgId);
}

void MessageHandler::slotAppGetsActive(bool active)
{
    appIsActive_ = active;
}

void MessageHandler::setAskBeforeDownloading(bool AskBeforeDownloading)
{
    askBeforeDownloading_ = AskBeforeDownloading;
}

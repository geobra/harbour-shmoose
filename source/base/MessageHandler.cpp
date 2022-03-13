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

void MessageHandler::handleMessageReceived(Swift::Message::ref message)
{
    //std::cout << "handleMessageReceived: jid: " << message->getFrom() << ", bare: " << message->getFrom().toBare().toString() << ", resource: " << message->getFrom().getResource() << std::endl;

    unsigned int security = 0;
    unsigned int direction = 1; // incoming
    Swift::Forwarded::ref forwarded = nullptr;
    Swift::Message::ref forwardedMessage = nullptr;
    qint64 timestamp = 0;
    QString partyJid = QString::fromStdString(message->getFrom().toBare().toString());
    auto clientBareJid = client_->getJID();
    qint64 start = Settings().getLatestMamSyncDate().toTime_t();

    auto delay = message->getPayload<Swift::Delay>();
    if(delay != nullptr)
    {
        using namespace boost::posix_time;
        static ptime epoch(boost::gregorian::date(1970, 1, 1));
        time_duration diff(delay->getStamp() - epoch);
        timestamp = diff.ticks() / diff.ticks_per_second();
    }

    // XEP 313 MAM
    bool isMamMsg = false;
    auto mamResult = message->getPayload<Swift::MAMResult>();
    if(mamResult != nullptr)
    {
        forwarded = mamResult->getPayload();
        if(forwarded != nullptr)
        {
            forwardedMessage = std::dynamic_pointer_cast<Swift::Message>(forwarded->getStanza());
            if(forwardedMessage != nullptr)
            {
                qDebug() << "Mam message" << endl;
                isMamMsg = true;
                message = forwardedMessage;
                partyJid = QString::fromStdString(message->getFrom().toBare().toString());

                if(forwarded->getDelay())
                {
                    using namespace boost::posix_time;
                    static ptime epoch(boost::gregorian::date(1970, 1, 1));
                    time_duration diff(forwarded->getDelay()->getStamp() - epoch);
                    timestamp = diff.ticks() / diff.ticks_per_second();
                }
            }
        }
    }

    // XEP 280 Carbon Messages
    if ((isMamMsg == false) && (settings_->getJid().compare(partyJid) == 0)) {
        Swift::CarbonsReceived::ref carbonsReceived;
        Swift::CarbonsSent::ref carbonsSent;
        if ((carbonsReceived = message->getPayload<Swift::CarbonsReceived>()) &&
                (forwarded = carbonsReceived->getForwarded()) &&
                (forwardedMessage = std::dynamic_pointer_cast<Swift::Message>(forwarded->getStanza()))) {
            // It is the carbon of a message that we received
            message = forwardedMessage;
            partyJid = QString::fromStdString(message->getFrom().toBare().toString());
            qDebug() << "Carbon received" << endl;
        }
        else if ((carbonsSent = message->getPayload<Swift::CarbonsSent>()) &&
                 (forwarded = carbonsSent->getForwarded()) &&
                 (forwardedMessage = std::dynamic_pointer_cast<Swift::Message>(forwarded->getStanza()))) {
            // It is the carbon of a message that we sent
            message = forwardedMessage;
            partyJid = QString::fromStdString(forwardedMessage->getTo().toString());
            qDebug() << "Carbon sent" << endl;
        }
        else
        {
            qDebug() << "Error Carbon" << endl;
        }
    }

    QString resource = QString::fromStdString(message->getFrom().getResource());

    // XEP 45 MUC messages
    bool isGroupMessage = false;
    if(message->getType() == Swift::Message::Groupchat)
    {
        qDebug() << "Group message" << endl;
        isGroupMessage = true;
        auto mucUser = message->getPayload<Swift::MUCUserPayload>();
        if(mucUser)
        {
            auto items = mucUser->getItems();
            if(items.size() > 0 && items[0].realJID)
            {
                auto msgOwnerJid = items[0].realJID;
                direction = msgOwnerJid->compare(clientBareJid, Swift::JID::WithoutResource) == 0 ? 0 : 1;
                partyJid = QString::fromStdString(msgOwnerJid->toBare().toString());
            }
        }
    }
    else
    {
        // 1o1 msg
        // if msg jid is same as my jid, then the msg was from me.
        qDebug() << "1o1 message" << endl;

        if(message->getFrom().compare(clientBareJid, Swift::JID::WithoutResource) == 0)
        {
            qDebug() << "message from me" << endl;
            direction = 0;
            partyJid = QString::fromStdString(message->getTo().toBare().toString());
            resource = QString::fromStdString(Swift::JID(message->getTo().toString()).getResource());
        }
    }

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

    boost::optional<std::string> fromBody = message->getBody();

    if (fromBody)
    {
        std::string body = *fromBody;
        QString theBody = QString::fromStdString(body);
        QUrl bodyUrl(theBody);
        bool isLink = false;

        QString type = "txt";

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
                downloadManager_->doDownload(bodyUrl); // keep the fragment in the sent message
            }
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

            // still empty?
            if (messageId.isEmpty() == true && (! isMamMsg))
            {
                messageId = QString::number(QDateTime::currentMSecsSinceEpoch());
            }
        }

        bool is1o1OrIsGroupWithResource = false;
        if (isGroupMessage == false)
        {
            is1o1OrIsGroupWithResource = true;
        }
        else if (! resource.isEmpty())
        {
            is1o1OrIsGroupWithResource = true;
        }

        bool isMsgToDiscard = false;
        if(timestamp > 0 && timestamp <= start)
        {
            qDebug() << "message to discard" << endl;
            isMsgToDiscard = true;
        }

        qDebug() << "Msg to process: " << messageId << ", " << partyJid << ", " << resource << ", " << theBody << ", " << type << ", "
                 << direction << ", " << security << ", " << timestamp << endl;

        if ( (! messageId.isEmpty()) && (! partyJid.isEmpty()) && is1o1OrIsGroupWithResource && (! isMsgToDiscard))
        {
            persistence_->addMessage(messageId, partyJid, resource, theBody, type, direction, security, timestamp);
        }

        // xep 0333
        QString currentChatPartner = persistence_->getCurrentChatPartner();
        qDebug() << "fromJid: " << partyJid << "current: " << currentChatPartner << ", isGroup: " << isGroupMessage << ", appActive? " << appIsActive_;
        if ( (currentChatPartner.compare(partyJid) == 0) &&     // immediatelly send read notification if sender is current chat partner
             (appIsActive_ == true)                             // but only if app is active
             )
        {
            this->sendDisplayedForJid(currentChatPartner);
        }
    }

    // process msg's with an 'received' tag
    auto deliveryReceipt = message->getPayload<Swift::DeliveryReceipt>();

    if (deliveryReceipt != nullptr)
    {
        QString msgId = QString::fromStdString(deliveryReceipt->getReceivedID());
        qDebug() << "Delivery Receipt received. msgId: " << msgId << endl;

        if (isGroupMessage == true)
        {
            persistence_->markGroupMessageReceivedByMember(msgId, resource);
        }
        else
        {
            persistence_->markMessageAsReceivedById(msgId);
        }
    }

    // XEP 0184
    if (message->getPayload<Swift::DeliveryReceiptRequest>())
    {
        Swift::Message::ref receiptReply = std::make_shared<Swift::Message>();
        receiptReply->setFrom(message->getTo());
        receiptReply->setTo(message->getFrom());

        std::shared_ptr<Swift::DeliveryReceipt> receipt = std::make_shared<Swift::DeliveryReceipt>();
        receipt->setReceivedID(message->getID());
        receiptReply->addPayload(receipt);
        client_->sendMessage(receiptReply);
    }


    // FIXME: create custom payload for chat markers
    // process msg's with an 'displayed' tag
    /*
    QString displayed = XmlProcessor::getChildFromNode("displayed", archivedMsg);
    if (! displayed.isEmpty())
    {
        QString msgId = XmlProcessor::getContentInTag("displayed", "id", displayed);
        //qDebug() << "msgId: " << msgId;
        if (isGroupMessage == true)
        {
            persistence_->markGroupMessageDisplayedByMember(msgId, resource);
        }
        else
        {
            persistence_->markMessageAsDisplayedId(msgId);
        }
    }*/
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


        // add delivery request
        msg->addPayload(std::make_shared<Swift::DeliveryReceiptRequest>());

        // add chatMarkers stanza
        msg->addPayload(std::make_shared<Swift::RawXMLPayload>(ChatMarkers::getMarkableString().toStdString()));

        // exchange body by omemo stuff if applicable
        bool shouldSendMsgStanze{true};
        if (settings_->getSoftwareFeatureOmemoEnabled() == true)
        {
            if ( isGroup == false // for now, omemo is only supported on 1o1 messaging
                 && (lurchAdapter_->isOmemoUser(toJid) == true) // the receipient client can handle omemo encryption
                 && (! settings_->getSendPlainText().contains(toJid)) // no force for plain text msg in settings
                 )
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

void MessageHandler::slotAppGetsActive(bool active)
{
    appIsActive_ = active;
}

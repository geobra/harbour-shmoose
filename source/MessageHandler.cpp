#include "MessageHandler.h"
#include "Persistence.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"
#include "ChatMarkers.h"
#include "Omemo.h"
#include "XmppMessageParserClient.h"

#include <QUrl>
#include <QDebug>
#include <QDomDocument>

MessageHandler::MessageHandler(Persistence *persistence, QObject *parent) : QObject(parent),
    client_(NULL), persistence_(persistence),
    downloadManager_(new DownloadManager(this)),
    chatMarkers_(new ChatMarkers(persistence_, this)),
    omemo_(new Omemo(this)),
    xmppMessageParserClient_(new XMPPMessageParserClient()),
    appIsActive_(true), unAckedMessageIds_()
{

}

MessageHandler::~MessageHandler()
{
    delete xmppMessageParserClient_;
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

        // omemo encryption
        omemo_->setupWithClient(client_);
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

    Swift::Message* plainMessage = nullptr;
    std::string fromJid = "";

    // check if received message is encrypted
    QString qMsg = getSerializedStringFromMessage(message);
    if (isEncryptedMessage(qMsg))
    {
        fromJid = Omemo::getValueForElementInNode("message", qMsg, "from").toStdString();
        std::string type = Omemo::getValueForElementInNode("message", qMsg, "type").toStdString();

        if ( (! fromJid.empty()) && (! type.empty()) )
        {
            QString decryptedMessage = omemo_->lurch_message_decrypt(fromJid.c_str(), type.c_str(), qMsg.toStdString());

            if (decryptedMessage.isEmpty())
            {
                // something went wrong on the decryption. use original encrypted message and go one...
                plainMessage = &(*message);
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
                    plainMessage = xmppMessageParserClient_->getMessagePtr();
                    if (plainMessage == nullptr)
                    {
                        plainMessage = &(*message);
                    }
                }
                else
                {
                    // failure on xml parsing. use original message
                    plainMessage = &(*message);
                }
            }
        }
    }
    else
    {
        plainMessage = &(*message);
    }

    fromJid = plainMessage->getFrom().toBare().toString();
    boost::optional<std::string> fromBody = plainMessage->getBody();

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
        if (plainMessage->getType() == Swift::Message::Groupchat)
        {
            isGroupMessage = true;
        }

        persistence_->addMessage(isGroupMessage,
                                 QString::fromStdString(plainMessage->getID()),
                                 QString::fromStdString(fromJid),
                                 QString::fromStdString(plainMessage->getFrom().getResource()),
                                 theBody, type, 1 );

        // xep 0333
        QString currentChatPartner = persistence_->getCurrentChatPartner();
        qDebug() << "fromJid: " << QString::fromStdString(fromJid) << "current: " << currentChatPartner << ", isGroup: " << isGroupMessage << ", appActive? " << appIsActive_;
        if ( (isGroupMessage == false) &&                                               // no read notification for group messages
             (currentChatPartner.compare(QString::fromStdString(fromJid)) == 0) &&     // immediatelly send read notification if sender is current chat partner
             (appIsActive_ == true)                                                     // but only if app is active
             )
        {
            chatMarkers_->sendDisplayedForJid(currentChatPartner);
        }
    }

    // XEP 0184
    if (plainMessage->getPayload<Swift::DeliveryReceiptRequest>())
    {
        // send message receipt
        Swift::Message::ref receiptReply = boost::make_shared<Swift::Message>();
        receiptReply->setFrom(plainMessage->getTo());
        receiptReply->setTo(plainMessage->getFrom());

        boost::shared_ptr<Swift::DeliveryReceipt> receipt = boost::make_shared<Swift::DeliveryReceipt>();
        receipt->setReceivedID(plainMessage->getID());
        receiptReply->addPayload(receipt);
        client_->sendMessage(receiptReply);
    }

    // mark sent msg as received
    Swift::DeliveryReceipt::ref rcpt = plainMessage->getPayload<Swift::DeliveryReceipt>();
    if (rcpt)
    {
        std::string recevideId = rcpt->getReceivedID();
        if (recevideId.length() > 0)
        {
            persistence_->markMessageAsReceivedById(QString::fromStdString(recevideId));
        }
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

        boost::shared_ptr<Swift::RawXMLPayload> outOfBand =
                boost::make_shared<Swift::RawXMLPayload>(outOfBandElement.toStdString());
        msg->addPayload(outOfBand);
    }

    Swift::Message::Type messagesTyp = Swift::Message::Chat;
    if (isGroup == true)
    {
        messagesTyp = Swift::Message::Groupchat;
    }
    msg->setType(messagesTyp);

    msg->addPayload(boost::make_shared<Swift::DeliveryReceiptRequest>());

    // add chatMarkers stanza
    msg->addPayload(boost::make_shared<Swift::RawXMLPayload>(ChatMarkers::getMarkableString().toStdString()));

    if ( true /* FIXME check if msg should be sent omemo enc */ )
    {
        QString qMsg = getSerializedStringFromMessage(msg);
        omemo_->lurch_message_encrypt_im(toJid, qMsg);
    }
    else
    {
        client_->sendMessage(msg);
    }
    persistence_->addMessage( (Swift::Message::Groupchat == messagesTyp) ? true : false,
                             QString::fromStdString(msgId),
                             QString::fromStdString(receiverJid.toBare().toString()),
                             QString::fromStdString(receiverJid.getResource()),
                             message, type, 0);
    unAckedMessageIds_.push_back(QString::fromStdString(msgId));
}

QString MessageHandler::getSerializedStringFromMessage(Swift::Message::ref msg)
{
    Swift::FullPayloadSerializerCollection serializers_;
    Swift::XMPPSerializer xmppSerializer(&serializers_, Swift::ClientStreamType, true);
    Swift::SafeByteArray sba = xmppSerializer.serializeElement(msg);

    return QString::fromStdString(Swift::safeByteArrayToString(sba));
}

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
    chatMarkers_->sendDisplayedForJid(jid);
}

void MessageHandler::slotAppGetsActive(bool active)
{
    appIsActive_ = active;
}

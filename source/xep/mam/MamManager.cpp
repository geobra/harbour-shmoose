#include "MamManager.h"
#include "Persistence.h"
#include "XmlWriter.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"
#include "LurchAdapter.h"

#include <QDateTime>
#include <QUrl>
#include "XmlProcessor.h"
#include <QDebug>
#include <QMimeDatabase>
#include <typeinfo>

#include <Swiften/Elements/MAMResult.h>

const QString MamManager::mamNs = "urn:xmpp:mam:2";

MamManager::MamManager(Persistence *persistence, LurchAdapter *lurchAdapter, QObject *parent) : QObject(parent),
    serverHasFeature_(false), queridJids_(), persistence_(persistence),
    downloadManager_(new DownloadManager(this)), client_(nullptr), lurchAdapter_(lurchAdapter)
{
    // https://xmpp.org/extensions/attic/xep-0313-0.5.html
}

void MamManager::setupWithClient(Swift::Client* client)
{
    client_ = client;

    client_->onConnected.connect(boost::bind(&MamManager::handleConnected, this));

    // watch the received messages and handle the Mam one's
    client_->onMessageReceived.connect(boost::bind(&MamManager::handleMessageReceived, this, _1));
}

void MamManager::handleConnected()
{
    // reset on each new connect
    queridJids_.clear();
}

void MamManager::receiveRoomWithName(QString jid, QString name)
{
    (void) name;
    addJidforArchiveQuery(jid);
}

void MamManager::addJidforArchiveQuery(QString jid)
{
    qDebug() << "MamManager::addJidforArchiveQuery " << jid;

    if (! queridJids_.contains(jid))
    {
        queridJids_.append(jid);
        requestArchiveForJid(jid);
    }
}

void MamManager::setServerHasFeatureMam(bool hasFeature)
{
    qDebug() << "MamManager::setServerHasFeatureMam: " << hasFeature << endl;
    serverHasFeature_ = hasFeature;

    requestArchiveForJid(QString::fromStdString(client_->getJID().toBare().toString()));
}

void MamManager::requestArchiveForJid(const QString& jid, const QString &last)
{
    if (serverHasFeature_)
    {
        qDebug() << "MamManager::requestArchiveForJid: " << jid << ", last: " << last << endl;

        // get the date of last week
        QDateTime lastWeek = QDateTime::currentDateTimeUtc().addDays(-45);
        lastWeek.setTimeSpec(Qt::UTC);

        // construct the mam query for messages from within last two week
        XmlWriter xw;
        xw.writeOpenTag( "query", AttrMap("xmlns", MamManager::mamNs) );

        AttrMap xmlnsMap;
        xmlnsMap.insert("xmlns", "jabber:x:data");
        xmlnsMap.insert("type", "submit");
        xw.writeOpenTag("x", xmlnsMap);

        AttrMap fieldMap;
        fieldMap.insert("var", "FORM_TYPE");
        fieldMap.insert("type", "hidden");
        xw.writeOpenTag("field", fieldMap);
        xw.writeTaggedString( "value", MamManager::mamNs );
        xw.writeCloseTag( "field" );

        xw.writeOpenTag( "field", AttrMap("var", "start") );
        xw.writeTaggedString( "value", lastWeek.toString(Qt::ISODate) );
        xw.writeCloseTag( "field" );

        xw.writeCloseTag( "x" );

        if (! last.isEmpty())
        {
            xw.writeOpenTag( "set", AttrMap("xmlns", "http://jabber.org/protocol/rsm") );
            xw.writeTaggedString( "after", last );
            xw.writeCloseTag( "set" );
        }

        xw.writeCloseTag( "query" );

        Swift::IDGenerator idGenerator;
        std::string msgId = idGenerator.generateID();

        Swift::JID sJid(jid.toStdString());

        // Calling getXmlResult flushes content and subsequent calls are empty! 
        // qDebug() << xw.getXmlResult(); 

        client_->getIQRouter()->sendIQ(Swift::IQ::createRequest(Swift::IQ::Set, sJid, msgId,
                                                                std::make_shared<Swift::RawXMLPayload>(xw.getXmlResult().toStdString())
                                                                ));

    }
}

void MamManager::handleMessageReceived(Swift::Message::ref message)
{
    auto mamResult = message->getPayload<Swift::MAMResult>();

    if(mamResult != nullptr)
    {
        processMamMessage(mamResult->getPayload());
    }
}

void MamManager::processFinIq(const QString& iq)
{
    if (! iq.isEmpty() )
    {
        QString from = XmlProcessor::getContentInTag("iq", "from", iq);

        QString fin = XmlProcessor::getChildFromNode("fin", iq);
        if (! fin.isEmpty() && ! from.isEmpty() )
        {
            QString complete = XmlProcessor::getContentInTag("fin", "complete", fin);

            if (complete.compare("false", Qt::CaseInsensitive) == 0)
            {
                QString last = XmlProcessor::getContentInTag("set", "last", fin);
                qDebug() << "####### mam not complete! last id: " << last;

                requestArchiveForJid(from, last);
            }
            else
            {
                qDebug() << "########## mam fin complete for jid: " << from;
            }
        }
    }
}

void MamManager::processMamMessage(std::shared_ptr<Swift::Forwarded> forwarded)
{
    unsigned int security = 0;
    bool isGroupMessage = false;
    QString senderBareJid = "";
    QString resource = "";
    unsigned int direction = 1; // incoming

    qDebug() << "MamManager::processMamMessage" << endl;

    if(forwarded)
    {        
        Swift::Message::ref message = std::dynamic_pointer_cast<Swift::Message>(forwarded->getStanza());
        auto clientBareJid = client_->getJID();
        QString id = QString::fromStdString(message->getID());
        resource = QString::fromStdString(Swift::JID(message->getFrom().toString()).getResource());

        if(message->getType() == Swift::Message::Groupchat)
        {
            isGroupMessage = true;

            auto mucUser = message->getPayload<Swift::MUCUserPayload>();

            if(mucUser)
            {
                auto items = mucUser->getItems();

                if(items.size() > 0 && items[0].realJID)
                {
                    auto msgOwnerJid = items[0].realJID;  
                    direction = msgOwnerJid->compare(clientBareJid, Swift::JID::WithoutResource) == 0 ? 0 : 1;
                    senderBareJid = QString::fromStdString(msgOwnerJid->toBare().toString());
                }  
            }
        }
        else
        {
            // 1o1 msg
            // if msg jid is same as my jid, then the msg was from me.

            if(message->getFrom().compare(clientBareJid, Swift::JID::WithoutResource) == 0)        
            {
                direction = 0; // outgoing
                senderBareJid = QString::fromStdString(message->getTo().toBare().toString());
                resource = QString::fromStdString(Swift::JID(message->getTo().toString()).getResource());
            }
        }

        // Check if this is an OMEMO message

        auto success = lurchAdapter_->decryptMessageIfEncrypted(message);
        if (success == 0) // 0: success on decryption, 1: was not encrypted, 2: error during decryption.
        {
            security = 1;
        }
        else if (success == 2)
        {
            qDebug() << "MamManager::processMamMessage: error during decryption).";
            QString cryptErrorMsg{tr("** Enrypted message could not be decrypted. Sorry. **")};
            message->setBody(cryptErrorMsg.toStdString());
        }

        QString body = "";

        // process msg's with a body
        if (message->getBody()) // process messages with a body text
        {
            // get timestamp of orginal sending
            qint64 timestamp = 0;

            if(forwarded->getDelay())
            {
                using namespace boost::posix_time;
                static ptime epoch(boost::gregorian::date(1970, 1, 1));
                time_duration diff(forwarded->getDelay()->getStamp() - epoch);
                timestamp = diff.ticks() / diff.ticks_per_second();
            }

            QString body = QString::fromStdString(*message->getBody());
            
            //                
            qDebug() << "mam group: " << isGroupMessage << " body: " << body
                     << ", id: " << id << " , jid: " << senderBareJid << ", resource: "
                     << resource << "direction: " << direction << ", ts: " << timestamp;
            //

            bool is1o1OrIsGroupWithResource = false;
            if (isGroupMessage == false)
            {
                is1o1OrIsGroupWithResource = true;
            }
            else if (! resource.isEmpty())
            {
                is1o1OrIsGroupWithResource = true;
            }

            QString type = "txt";

            bool isLink = false;
            if(success == 0)
            {
                isLink = body.startsWith("aesgcm://");

            } 
            else
            {
                auto xmlPayloads = message->getPayloads<Swift::RawXMLPayload>();
                for (std::vector<std::shared_ptr<Swift::RawXMLPayload>>::iterator it = xmlPayloads.begin() ; it != xmlPayloads.end(); ++it)
                {
                    QString rawXml = QString::fromStdString((*it)->getRawXML());

                    if(!XmlProcessor::getContentInElement("url", rawXml).isEmpty())
                    {
                        isLink = true;
                        break;
                    }
                }
            }

            if (isLink)
            {
                QUrl url = QUrl(body);
                if(url.scheme().length() > 0)
                {
                    type = QMimeDatabase().mimeTypeForFile(url.fileName()).name();
                    downloadManager_->doDownload(url);
                }
            }

            if ( (! id.isEmpty()) && (! senderBareJid.isEmpty()) && is1o1OrIsGroupWithResource )
            {
                persistence_->addMessage(id, senderBareJid, resource, body, type, direction, security, timestamp);
            }
        }

        // process msg's with an 'received' tag
        auto deliveryReceipt = message->getPayload<Swift::DeliveryReceipt>();

        if (deliveryReceipt != nullptr) 
        {
            QString msgId = QString::fromStdString(deliveryReceipt->getReceivedID());
            qDebug() << "msgId: " << msgId;

            if (isGroupMessage == true)
            {
                persistence_->markGroupMessageReceivedByMember(msgId, resource);
            }
            else
            {
                persistence_->markMessageAsReceivedById(msgId);
            }
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
}

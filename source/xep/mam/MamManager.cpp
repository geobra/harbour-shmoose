#include "MamManager.h"
#include "Persistence.h"
#include "XmlWriter.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"

#include <QDateTime>
#include <QUrl>
#include "XmlProcessor.h"
#include <QDebug>
#include <QMimeDatabase>


const QString MamManager::mamNs = "urn:xmpp:mam:2";

MamManager::MamManager(Persistence *persistence, QObject *parent) : QObject(parent),
    serverHasFeature_(false), queridJids_(), persistence_(persistence),
    downloadManager_(new DownloadManager(this)), client_(nullptr)
{
    // https://xmpp.org/extensions/attic/xep-0313-0.5.html
}

void MamManager::setupWithClient(Swift::Client* client)
{
    client_ = client;

    client_->onConnected.connect(boost::bind(&MamManager::handleConnected, this));

    // watch the received messages and handle the Mam one's
    client_->onDataRead.connect(boost::bind(&MamManager::handleDataReceived, this, _1));
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
    //qDebug() << "MamManager::addJidforArchiveQuery " << jid;

    if (! queridJids_.contains(jid))
    {
        queridJids_.append(jid);
        requestArchiveForJid(jid);
    }
}

void MamManager::setServerHasFeatureMam(bool hasFeature)
{
    //qDebug() << "MamManager::setServerHasFeatureMam: " << hasFeature;
    serverHasFeature_ = hasFeature;

    requestArchiveForJid(QString::fromStdString(client_->getJID().toBare().toString()));
}

void MamManager::requestArchiveForJid(const QString& jid, const QString &last)
{
    if (serverHasFeature_)
    {
        //qDebug() << "MamManager::requestArchiveForJid: " << jid << ", last: " << last;

        // get the date of last week
        QDateTime lastWeek = QDateTime::currentDateTimeUtc().addDays(-14);
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

        //qDebug() << xw.getXmlResult();

        client_->getIQRouter()->sendIQ(Swift::IQ::createRequest(Swift::IQ::Set, sJid, msgId,
                                                                std::make_shared<Swift::RawXMLPayload>(xw.getXmlResult().toStdString())
                                                                ));

    }
}

// FIXME rewrite me!
// this fails sometimes :-(.
// use custom payload parser to filter out mam messages!

void MamManager::handleDataReceived(Swift::SafeByteArray data)
{
    std::string nodeData = Swift::safeByteArrayToString(data);
    QString qData = QString::fromStdString(nodeData);

    // check for MamMessage
    QString xmlnsTagMsg = XmlProcessor::getContentInTag("result", "xmlns", qData);
    if ( xmlnsTagMsg.contains(MamManager::mamNs, Qt::CaseInsensitive) == true )
    {
        processMamMessage(qData);
    }

    // check iq if the answer contains the whole archive. Request more, if not.
    QString xmlnsTagFin = XmlProcessor::getContentInTag("fin", "xmlns", qData);
    if ( xmlnsTagFin.contains(MamManager::mamNs, Qt::CaseInsensitive) == true )
    {
        processFinIq(qData);
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
                //qDebug() << "####### mam not complete! last id: " << last;

                requestArchiveForJid(from, last);
            }
            else
            {
                //qDebug() << "########## mam fin complete for jid: " << from;
            }
        }
    }
}

void MamManager::processMamMessage(const QString& qData)
{
    unsigned int security = 0;
    bool isGroupMessage = false;
    QString senderBareJid = "";
    QString resource = "";
    unsigned int direction = 1; // incoming

    QString archivedMsg = XmlProcessor::getChildFromNode("message", qData);

    if (! archivedMsg.isEmpty() )
    {
        QString clientBareJid = QString::fromStdString(client_->getJID().toBare().toString());

        QString id = XmlProcessor::getContentInTag("message", "id", archivedMsg);

        QString fromJid = XmlProcessor::getContentInTag("message", "from", archivedMsg);
        senderBareJid = QString::fromStdString(Swift::JID(fromJid.toStdString()).toBare().toString());
        resource = QString::fromStdString(Swift::JID(fromJid.toStdString()).getResource());

        QString msgType = XmlProcessor::getContentInTag("message", "type", archivedMsg);
        if (msgType.compare("groupchat", Qt::CaseInsensitive) == 0)
        {
            isGroupMessage = true;

            QString msgOwnerJid = XmlProcessor::getContentInTag("item", "jid", archivedMsg);
            QString msgOwnerBareJid = QString::fromStdString(Swift::JID(msgOwnerJid.toStdString()).toBare().toString());

            // if msg jid is same as my jid, then the msg was from me.
            if (clientBareJid.compare(msgOwnerBareJid, Qt::CaseInsensitive) == 0)
            {
                direction = 0; // outgoing
            }
        }
        else
        {
            // 1o1 msg
            // if msg jid is same as my jid, then the msg was from me.
            if (clientBareJid.compare(senderBareJid, Qt::CaseInsensitive) == 0)
            {
                direction = 0; // outgoing
                QString toJid = XmlProcessor::getContentInTag("message", "to", archivedMsg);
                senderBareJid = QString::fromStdString(Swift::JID(toJid.toStdString()).toBare().toString());
                resource = QString::fromStdString(Swift::JID(toJid.toStdString()).getResource());
            }
        }

        // FIXME check if this is an omemo msg!

        // process msg's with a body
        QString body = XmlProcessor::getContentInTag("message", "body", archivedMsg);
        if (! body.isEmpty()) // process messages with a body text
        {
            // get timestamp of orginal sending
            qint64 timestamp = 0;
            QString delay = XmlProcessor::getContentInTag("delay", "stamp", qData);
            if (! delay.isEmpty())
            {
                QDateTime ts = QDateTime::fromString(delay, Qt::ISODate);
                if (ts.isValid())
                {
                    timestamp = ts.toTime_t();
                }
            }

            /*
            qDebug() << "mam group: " << isGroupMessage << " body: " << body
                     << ", id: " << id << " , jid: " << senderBareJid << ", resource: "
                     << resource << "direction: " << direction << ", ts: " << timestamp;
            */

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
            if(XmlProcessor::getContentInTag("encryption", "name", archivedMsg) == "OMEMO")
            {
                isLink = body.startsWith("aesgcm://");

            } else
            {
                isLink = !XmlProcessor::getContentInElement("url", archivedMsg).isEmpty();
            }

            if (isLink)
            {
                QUrl url = QUrl(body);
                if(url.scheme().length() > 0)
                {
                    type = QMimeDatabase().mimeTypeForFile(url.fileName()).name();
                    downloadManager_->doDownload(url, id);
                }
            }

            if ( (! id.isEmpty()) && (! senderBareJid.isEmpty()) && is1o1OrIsGroupWithResource )
            {
                persistence_->addMessage(id, senderBareJid, resource, body, type, direction, security, timestamp);
            }
        }

        // process msg's with an 'received' tag
        QString received = XmlProcessor::getChildFromNode("received", archivedMsg);
        if (! received.isEmpty())
        {
            QString msgId = XmlProcessor::getContentInTag("received", "id", received);
            //qDebug() << "msgId: " << msgId;

            if (isGroupMessage == true)
            {
                persistence_->markGroupMessageReceivedByMember(msgId, resource);
            }
            else
            {
                persistence_->markMessageAsReceivedById(msgId);
            }
        }

        // process msg's with an 'displayed' tag
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
        }

    }
}

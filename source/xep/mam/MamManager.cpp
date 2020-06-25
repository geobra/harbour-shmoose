#include "MamManager.h"
#include "Persistence.h"
#include "XmlWriter.h"

#include <QDateTime>
#include "XmlProcessor.h"
#include <QDebug>


const QString MamManager::mamNs = "urn:xmpp:mam:2";

MamManager::MamManager(Persistence *persistence, QObject *parent) : QObject(parent),
    serverHasFeature_(false), queridJids_(), persistence_(persistence), client_(nullptr)
{
}

void MamManager::setupWithClient(Swift::Client* client)
{
    client_ = client;

    client_->onConnected.connect(boost::bind(&MamManager::handleConnected, this));

    // simple 'message archive management' for fetched muc history
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

    // FIXME implement setting for storing ALL the msg in the archive

    requestArchiveForJid(QString::fromStdString(client_->getJID().toBare().toString()));
}

void MamManager::requestArchiveForJid(const QString& jid)
{
    // https://xmpp.org/extensions/attic/xep-0313-0.5.html

    if (serverHasFeature_)
    {
        qDebug() << "MamManager::requestArchiveForJid: " << jid;

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
        xw.writeCloseTag( "query" );

        Swift::IDGenerator idGenerator;
        std::string msgId = idGenerator.generateID();

        Swift::JID sJid(jid.toStdString());

        client_->getIQRouter()->sendIQ(Swift::IQ::createRequest(Swift::IQ::Set, sJid, msgId,
                                                                std::make_shared<Swift::RawXMLPayload>(xw.getXmlResult().toStdString())
                                                                ));

    }
}

void MamManager::handleDataReceived(Swift::SafeByteArray data)
{
    // FIXME received and displayed status for msgId's

    bool isGroupMessage = false;
    QString senderBareJid = "";
    QString resource = "";
    int direction = 1; // incoming

    std::string nodeData = Swift::safeByteArrayToString(data);
    QString qData = QString::fromStdString(nodeData);

    QString xmlnsTag = XmlProcessor::getContentInTag("result", "xmlns", qData);

    if ( xmlnsTag.contains(MamManager::mamNs, Qt::CaseInsensitive) == true )
    {
        QString archivedMsg = XmlProcessor::getChildFromNode("message", qData);
        if (! archivedMsg.isEmpty() )
        {
            QString body = XmlProcessor::getContentInTag("message", "body", archivedMsg);
            if (! body.isEmpty()) // only messages with a body text will be processed
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

                // FIXME check if the answer contains the whole archive or if we have to ask for more

                qDebug() << "mam group: " << isGroupMessage << " body: " << body << ", id: " << id << " , jid: " << senderBareJid << ", resource: " << resource << "direction: " << direction;

                bool is1o1OrIsGroupWithResource = false;
                if (isGroupMessage == false)
                {
                    is1o1OrIsGroupWithResource = true;
                }
                else if (! resource.isEmpty())
                {
                    is1o1OrIsGroupWithResource = true;
                }

                if ( (! id.isEmpty()) && (! senderBareJid.isEmpty()) && is1o1OrIsGroupWithResource )
                {
                    // FIXME has to also check content type. Currently we assume txt

                    // FIXME provide possibility to alter the date to the real timestamp from mam
                    persistence_->addMessage(id, senderBareJid, resource, body, "txt", direction );
                }
            }
        }
    }
}

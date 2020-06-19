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

    // FIXME implement setting for storing ALL your msg in the archive

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
                QString id = XmlProcessor::getContentInTag("message", "id", archivedMsg);
                QString archiveJid = XmlProcessor::getContentInTag("message", "from", archivedMsg);

                QString msgOwnerJid = XmlProcessor::getContentInTag("item", "jid", archivedMsg);
                QString msgOwnerBareJid = QString::fromStdString(Swift::JID(msgOwnerJid.toStdString()).toBare().toString());
                QString myBareJid = QString::fromStdString(client_->getJID().toBare().toString());

                // FIXME check if the answer contains the whole archive or if we have to ask for more

                Swift::JID jid = Swift::JID(archiveJid.toStdString());
                QString from = QString::fromStdString(jid.getResource());
                QString bareJid = QString::fromStdString(jid.toBare());

                qDebug() << "muc mam! body: " << body << ", id: " << id << " , jid: " << bareJid << ", from: " << from;

                if ( (! id.isEmpty()) && (! bareJid.isEmpty()) && (! from.isEmpty()) )
                {
                    // FIXME test for 1o1 msges
                    // FIXME has to also check content type. Currently we assume txt

                    bool isGroupMessage = queridJids_.contains(bareJid, Qt::CaseInsensitive);

                    // FIXME check direction
                    int direction = 1; // incoming

                    // if msg jid is same as my jid, then the msg was from me.
                    if (msgOwnerBareJid.compare(myBareJid, Qt::CaseInsensitive) == 0)
                    {
                        direction = 0; // outgoing
                    }

                    // FIXME provide possibility to alter the date to the real timestamp from mam
                    persistence_->addMessage(isGroupMessage, id, bareJid, from, body, "txt", direction );
                }
            }
        }
    }
}

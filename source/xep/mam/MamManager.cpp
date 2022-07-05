#include "MamManager.h"
#include "Persistence.h"
#include "XmlWriter.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"
#include "MamRequest.h"
#include "Settings.h"

#include <QDateTime>
#include <QUrl>
#include "XmlProcessor.h"
#include <QDebug>
#include <QMimeDatabase>
#include <typeinfo>

#include <Swiften/Elements/MAMResult.h>
#include <Swiften/Queries/IQHandler.h>

const QString MamManager::mamNs = "urn:xmpp:mam:2";

MamManager::MamManager(Persistence *persistence, QObject *parent) : QObject(parent),
    serverHasFeature_(true), queridJids_(), persistence_(persistence),
    downloadManager_(new DownloadManager(this)), client_(nullptr)
{
    // https://xmpp.org/extensions/attic/xep-0313-0.5.html
}

void MamManager::setupWithClient(Swift::Client* client)
{
    client_ = client;

    client_->onConnected.connect(boost::bind(&MamManager::handleConnected, this));
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

    qDebug() << "FIXME test when this will be set! It seems this gets called multiple times...";
    //serverHasFeature_ = hasFeature;

    requestArchiveForJid(QString::fromStdString(client_->getJID().toBare().toString()));
}

void MamManager::requestArchiveForJid(const QString& jid, const QString &last)
{
    qDebug() << "requestArchiveForJid";

    if (serverHasFeature_)
    {
        qDebug() << "requestArchiveForJid for " << jid;
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
        xw.writeTaggedString( "value", lastWeek.toString(Qt::ISODate));
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

        MamRequest::ref mamRequest = MamRequest::create(Swift::IQ::Set, sJid, xw.getXmlResult().toStdString(), client_->getIQRouter());
        mamRequest->onResponse.connect(boost::bind(&MamManager::processFinIq, this, _1, _2, _3));
        mamRequest->send();
    }
}

void MamManager::processFinIq(const std::string& jid, std::shared_ptr<Swift::MAMFin> mamFin, Swift::ErrorPayload::ref error)
{
    qDebug() << "MamManager::processFinIq" <<endl;

    if (mamFin != nullptr )
    {
        QString from = QString::fromStdString(jid);

        if (! from.isEmpty() )
        {
            auto resultSet = mamFin->getResultSet();

            if (! mamFin->isComplete() && resultSet != nullptr)
            {
                if(resultSet->getLastID() != nullptr)
                {
                    QString last = QString::fromStdString(*resultSet->getLastID());
                    qDebug() << "####### mam not complete! last id: " << last;
                    requestArchiveForJid(from, last);
                }
            }
            else
            {
                qDebug() << "########## mam fin complete for jid: " << from;
                QDateTime now = QDateTime::currentDateTimeUtc();
                now.setTimeSpec(Qt::UTC);
                Settings().setLatestMamSyncDate(now);
            }
        }
    }
}

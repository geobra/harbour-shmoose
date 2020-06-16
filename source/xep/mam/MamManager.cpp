#include "MamManager.h"
#include "Persistence.h"

#include <QDebug>

const QString MamManager::mamNs = "urn:xmpp:mam:2";

MamManager::MamManager(Persistence *persistence, QObject *parent) : QObject(parent), persistence_(persistence),
    serverHasFeature_(false), queridJids_()
{
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

void MamManager::addJidforArchiveQuery(QString jid)
{
    if (! queridJids_.contains(jid))
    {
        queridJids_.append(jid);
    }
    qDebug() << "### MAM from bookmarks: " << jid;
}

void MamManager::setServerHasFeatureMam(bool hasFeature)
{
    qDebug() << "MamManager::setServerHasFeatureMam: " << hasFeature;
    serverHasFeature_ = hasFeature;

    requestArchiveForJid(QString::fromStdString(client_->getJID().toBare().toString()));
}

void MamManager::requestArchives()
{
    for (auto item: queridJids_)
    {
        requestArchiveForJid(item);
    }
}

void MamManager::requestArchiveForJid(const QString& jid)
{
    if (serverHasFeature_)
    {
        qDebug() << "MamManager::requestArchiveForJid: " << jid;
    }
}

void MamManager::receiveRoomWithName(QString jid, QString name)
{
    void (name);
    requestArchiveForJid(jid);
}

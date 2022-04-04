#pragma once

#include <QObject>
#include <Swiften/Swiften.h>


class MucAffiliations : public QObject
{
    Q_OBJECT
public:
    MucAffiliations(QObject *parent, Swift::Client* client, const QString& roomJid);

signals:

private:
    void handleAffiliationListReceived(Swift::MUCOccupant::Affiliation af, const std::vector<Swift::JID>& jid);

    Swift::Client* client_;
    const QString roomJid_;
    Swift::MUC::ref muc_;

};

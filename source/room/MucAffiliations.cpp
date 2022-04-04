#include "MucAffiliations.h"

// https://swift.im/swiften/api/
// https://xmpp.org/extensions/xep-0045.html#modifymember
// https://stackoverflow.com/questions/27393540/discovering-members-of-muc-room-as-occupant

MucAffiliations::MucAffiliations(QObject *parent, Swift::Client* client, const QString& roomJid) : QObject(parent), client_(client), roomJid_(roomJid)
{
    if (client_ != nullptr)
    {
        muc_ = client_->getMUCManager()->createMUC(roomJid_.toStdString());
        if (muc_)
        {
            muc_->onAffiliationListReceived.connect(boost::bind(&MucAffiliations::handleAffiliationListReceived, this, _1, _2));
            muc_->requestAffiliationList(Swift::MUCOccupant::Member); // Owner, Admin, Member, Outcast
        }
    }
}

void MucAffiliations::handleAffiliationListReceived(Swift::MUCOccupant::Affiliation af, const std::vector<Swift::JID>& jid)
{
    std::cout << "handleAffiliationListReceived: " << af << std::endl;

    for (auto id: jid)
    {
        std::cout << "jid: " << id.toString() << std::endl;
    }
}

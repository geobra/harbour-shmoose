#include "ChatMarkers.h"
#include "Persistence.h"
#include "XmlProcessor.h"
#include "RosterContoller.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDebug>

#include <iostream>

const QString ChatMarkers::chatMarkersIdentifier = "urn:xmpp:chat-markers:0";

ChatMarkers::ChatMarkers(Persistence *persistence, RosterController *rosterController, QObject *parent) : QObject(parent),
    client_(NULL), persistence_(persistence), rosterController_(rosterController)
{
}

void ChatMarkers::setupWithClient(Swift::Client *client)
{
    if (client != NULL)
    {
        client_ = client;
        client_->onMessageReceived.connect(boost::bind(&ChatMarkers::handleMessageReceived, this, _1));
    }
}

#if 0
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
<message from="abc@jabber-germany.de/shmoose" id="71bb5cb2-22f2-47d6-a650-dfef7b1cfd50" to="xyz@jabber.ccc.de" type="chat">
 <body>aaa</body>
 <request xmlns="urn:xmpp:receipts"></request>
 <markable xmlns="urn:xmpp:chat-markers:0"></markable>
</message>
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
<r xmlns="urn:xmpp:sm:2"></r>
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
<a xmlns="urn:xmpp:sm:2" h="66"></a>
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
<message lang="en" type="chat" to="abc@jabber-germany.de/shmoose" from="xyz@jabber.ccc.de/Conversations.e4uP">
 <received xmlns="urn:xmpp:chat-markers:0" id="71bb5cb2-22f2-47d6-a650-dfef7b1cfd50"></received>
 <received xmlns="urn:xmpp:receipts" id="71bb5cb2-22f2-47d6-a650-dfef7b1cfd50"></received>
 <store xmlns="urn:xmpp:hints"></store>
 <stanza-id xmlns="urn:xmpp:sid:0" id="2019-07-20-437057ca3c572db1" by="abc@jabber-germany.de"></stanza-id>
</message>
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
<message lang="en" type="chat" to="abx@jabber-germany.de/shmoose" from="xyz@jabber.ccc.de/Conversations.e4uP">
 <displayed xmlns="urn:xmpp:chat-markers:0" id="71bb5cb2-22f2-47d6-a650-dfef7b1cfd50"></displayed>
 <store xmlns="urn:xmpp:hints"></store>
 <stanza-id xmlns="urn:xmpp:sid:0" id="2019-07-20-c21c739c9a3ff9ff" by="abc@jabber-germany.de"></stanza-id>
</message>
#endif

void ChatMarkers::handleMessageReceived(Swift::Message::ref message)
{
    //  First handle the received stanza
    Swift::DeliveryReceipt::ref rcpt = message->getPayload<Swift::DeliveryReceipt>();
    if (rcpt)
    {
        /*
        // from group
        <message type="chat" to="abc@jabber-germany.de/shmoose" from="3q1u5zjubr4rb@conference.jabber.ccc.de/xyz<at>jabber.de">
         <received xmlns="urn:xmpp:receipts" id="d7b90bae-33a4-40f2-ab42-4e9d7c7882cf"></received>
         <x xmlns="http://jabber.org/protocol/muc#user"></x>
         <stanza-id xmlns="urn:xmpp:sid:0" id="2019-07-21-171d70626c459590" by="abc@jabber-germany.de"></stanza-id>
        </message>

        // 1o1
        <message type="chat" to="abc@jabber-germany.de/shmoose" from="xyz@jabber.de/shmoose">
         <received xmlns="urn:xmpp:receipts" id="2566cb34-7a0c-41af-8896-926e7ef320b9"></received>
         <stanza-id xmlns="urn:xmpp:sid:0" id="2019-07-21-7fc43ad45670ed63" by="abc@jabber-germany.de"></stanza-id>
        </message>
*/
        std::string recevideId = rcpt->getReceivedID();
        if (recevideId.length() > 0)
        {
            Swift::JID msgJid = message->getFrom();
            if (rosterController_->isGroup(QString::fromStdString(msgJid.toBare().toString())))
            {
                // add to received list for that msg in the group
                QString groupChatMember = QString::fromStdString(msgJid.getResource());
                persistence_->markGroupMessageReceivedByMember(QString::fromStdString(recevideId), groupChatMember);
            }
            else
            {
                persistence_->markMessageAsReceivedById(QString::fromStdString(recevideId));
            }
        }
    }


    /*
     * Then handle the displayed stanza
     */

    std::vector< boost::shared_ptr<Swift::RawXMLPayload> > xmlPayloads = message->getPayloads<Swift::RawXMLPayload>();
    for (std::vector<boost::shared_ptr<Swift::RawXMLPayload>>::iterator it = xmlPayloads.begin() ; it != xmlPayloads.end(); ++it)
    {
        QString rawXml = QString::fromStdString((*it)->getRawXML());
        QString isMsgDisplayed = XmlProcessor::getContentInTag("displayed", "xmlns", rawXml);

        if (isMsgDisplayed.compare(chatMarkersIdentifier, Qt::CaseInsensitive) == 0)
        {
            QString msgId = XmlProcessor::getContentInTag("displayed", "id", rawXml);
            if (! msgId.isEmpty())
            {
                QString sender = XmlProcessor::getContentInTag("displayed", "sender", rawXml);
                Swift::JID jidSender(sender.toStdString());
                if ((! sender.isEmpty()) && rosterController_->isGroup(QString::fromStdString(jidSender.toBare().toString())))
                {
                    // group member displayed
                    // add to displayed list for that msg in the group
                    QString groupChatMember = QString::fromStdString(jidSender.getResource());
                    persistence_->markGroupMessageDisplayedByMember(msgId, groupChatMember);
                }
                else
                {
                    // 1o1 displayed
                    persistence_->markMessageAsDisplayedId(msgId);
                    break;
                }
            }
        }
    }
}

void ChatMarkers::sendDisplayedForJid(const QString& jid)
{
    // FIXME be sure to use complete jid with resource!

    QPair<QString, int> messageIdAndState = persistence_->getNewestReceivedMessageIdAndStateOfJid(jid);

    QString displayedMsgId = messageIdAndState.first;
    int msgState = messageIdAndState.second;

    //qDebug() << "id: " << displayedMsgId << ", state: " << msgState;

    if ( ( msgState != -1 ) && (! displayedMsgId.isEmpty()) && (! jid.isEmpty()) )
    {
        Swift::Message::ref msg(new Swift::Message);

        Swift::IDGenerator idGenerator;
        std::string msgId = idGenerator.generateID();

        msg->setFrom(Swift::JID(client_->getJID()));
        msg->setTo(jid.toStdString());
        msg->setID(msgId);
        msg->setType(Swift::Message::Normal);

        // add chatMarkers stanza
        msg->addPayload(boost::make_shared<Swift::RawXMLPayload>(getDisplayedStringForId(displayedMsgId).toStdString()));

        client_->sendMessage(msg);

        // mark msg as confirmed. no further confirms of that msg
        persistence_->markMessageDisplayedConfirmedId(displayedMsgId);
    }
}

QString ChatMarkers::getDisplayedStringForId(QString displayedId)
{
    return "<displayed xmlns='" +  chatMarkersIdentifier + "' id='" + displayedId + "' />";
}

QString ChatMarkers::getMarkableString()
{
    return "<markable xmlns='" +  chatMarkersIdentifier + "'/>";
}

#if 0
// group chat receives

>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
<message from="abc@jabber-germany.de/shmoose" id="a3a3316e-02c0-4dbf-b130-92609a27e2e2" to="3q1u5zjubr4rb@conference.jabber.ccc.de" type="groupchat">
 <body>abcd</body>
 <request xmlns="urn:xmpp:receipts"></request>
 <markable xmlns="urn:xmpp:chat-markers:0"></markable>
</message>
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
<message id="a3a3316e-02c0-4dbf-b130-92609a27e2e2" type="groupchat" to="abc@jabber-germany.de/shmoose" from="3q1u5zjubr4rb@conference.jabber.ccc.de/xyz">
 <archived xmlns="urn:xmpp:mam:tmp" id="1563386700596215" by="3q1u5zjubr4rb@conference.jabber.ccc.de"></archived>
 <stanza-id xmlns="urn:xmpp:sid:0" id="1563386700596215" by="3q1u5zjubr4rb@conference.jabber.ccc.de"></stanza-id>
 <request xmlns="urn:xmpp:receipts"></request>
 <markable xmlns="urn:xmpp:chat-markers:0"></markable>
 <body>abcd</body>
</message>
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
<message from="abc@jabber-germany.de/shmoose" to="3q1u5zjubr4rb@conference.jabber.ccc.de/xyz" type="chat">
 <received xmlns="urn:xmpp:receipts" id="a3a3316e-02c0-4dbf-b130-92609a27e2e2"></received>
</message>
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
<message type="chat" to="abc@jabber-germany.de/shmoose" from="3q1u5zjubr4rb@conference.jabber.ccc.de/xyz">
 <received xmlns="urn:xmpp:receipts" id="a3a3316e-02c0-4dbf-b130-92609a27e2e2"></received>
 <x xmlns="http://jabber.org/protocol/muc#user"></x>
 <stanza-id xmlns="urn:xmpp:sid:0" id="2019-07-17-7e5e36fc8fb67543" by="abc@jabber-germany.de"></stanza-id>
</message>
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
<message type="chat" to="abc@jabber-germany.de/shmoose" from="3q1u5zjubr4rb@conference.jabber.ccc.de/bla<at>jabber.de">
 <received xmlns="urn:xmpp:receipts" id="a3a3316e-02c0-4dbf-b130-92609a27e2e2"></received>
 <x xmlns="http://jabber.org/protocol/muc#user"></x>
 <stanza-id xmlns="urn:xmpp:sid:0" id="2019-07-17-9c6e9b464f6aac72" by="abc@jabber-germany.de"></stanza-id>
</message>
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
<message lang="en" type="groupchat" to="abc@jabber-germany.de/shmoose" from="3q1u5zjubr4rb@conference.jabber.ccc.de/blub">
 <archived xmlns="urn:xmpp:mam:tmp" id="1563386725459309" by="3q1u5zjubr4rb@conference.jabber.ccc.de"></archived>
 <stanza-id xmlns="urn:xmpp:sid:0" id="1563386725459309" by="3q1u5zjubr4rb@conference.jabber.ccc.de"></stanza-id>
 <displayed xmlns="urn:xmpp:chat-markers:0" id="a3a3316e-02c0-4dbf-b130-92609a27e2e2" sender="3q1u5zjubr4rb@conference.jabber.ccc.de/xyz"></displayed>
 <store xmlns="urn:xmpp:hints"></store>
</message>
#endif

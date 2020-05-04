#include "ChatMarkers.h"
#include "Persistence.h"
#include "XmlProcessor.h"
#include "RosterController.h"
#include "XmlWriter.h"

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

    // Then handle the displayed stanza
    std::vector< std::shared_ptr<Swift::RawXMLPayload> > xmlPayloads = message->getPayloads<Swift::RawXMLPayload>();
    for (std::vector<std::shared_ptr<Swift::RawXMLPayload>>::iterator it = xmlPayloads.begin() ; it != xmlPayloads.end(); ++it)
    {
        QString rawXml = QString::fromStdString((*it)->getRawXML());
        QString isMsgDisplayed = XmlProcessor::getContentInTag("displayed", "xmlns", rawXml);

        if (isMsgDisplayed.compare(chatMarkersIdentifier, Qt::CaseInsensitive) == 0)
        {
            QString msgId = XmlProcessor::getContentInTag("displayed", "id", rawXml);
            if (! msgId.isEmpty())
            {
                QString sender = QString::fromStdString(message->getFrom());
                Swift::JID jidSender(sender.toStdString());
                if ((! sender.isEmpty()) && rosterController_->isGroup(QString::fromStdString(jidSender.toBare().toString())))
                {
                    // group member displayed
                    // add to displayed list for that msg in the group
                    QString groupChatMember = QString::fromStdString(jidSender.getResource());
                    if (! groupChatMember.isEmpty())
                    {
                        persistence_->markGroupMessageDisplayedByMember(msgId, groupChatMember);
                    }
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
    bool itsAMuc = rosterController_->isGroup(jid);
    QPair<QString, int> messageIdAndState = persistence_->getNewestReceivedMessageIdAndStateOfJid(jid);

    QString displayedMsgId = messageIdAndState.first;
    int msgState = messageIdAndState.second;

    //qDebug() << "id: " << displayedMsgId << ", state: " << msgState;

    if ( ( msgState != -1 ) && (! displayedMsgId.isEmpty()) && (! jid.isEmpty()) )
    {
        Swift::Message::ref msg(new Swift::Message);

        Swift::IDGenerator idGenerator;
        std::string msgId = idGenerator.generateID();
        Swift::JID toJID = Swift::JID(jid.toStdString());

        msg->setFrom(Swift::JID(client_->getJID()));
        msg->setID(msgId);

        AttrMap displayedPayloadContent;
        displayedPayloadContent.insert("xmlns", chatMarkersIdentifier);
        displayedPayloadContent.insert("id", displayedMsgId);

        if (itsAMuc == true)
        {
            msg->setTo(toJID.toBare());
            msg->setType(Swift::Message::Groupchat);

            QString sender = QString::fromStdString(toJID.toString());

            if (toJID.isBare() == true)
            {
                sender += "/" + persistence_->getResourceForMsgId(displayedMsgId);
            }

            displayedPayloadContent.insert("sender", sender);
        }
        else
        {
            msg->setTo(toJID);
            msg->setType(Swift::Message::Normal);
        }

        // add chatMarkers stanza
        XmlWriter xw;
        xw.writeAtomTag("displayed", displayedPayloadContent);

        msg->addPayload(std::make_shared<Swift::RawXMLPayload>(xw.getXmlResult().toStdString()));

        client_->sendMessage(msg);

        // mark msg as confirmed. no further confirms of that msg
        persistence_->markMessageDisplayedConfirmedId(displayedMsgId);
    }
}

QString ChatMarkers::getMarkableString()
{
    XmlWriter xw;
    xw.writeAtomTag("markable", AttrMap("xmlns", chatMarkersIdentifier));

    return xw.getXmlResult();
}

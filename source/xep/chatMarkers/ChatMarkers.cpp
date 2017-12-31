#include "ChatMarkers.h"
#include "Persistence.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDebug>

#include <iostream>

const QString ChatMarkers::chatMarkersIdentifier = "urn:xmpp:chat-markers:0";

ChatMarkers::ChatMarkers(Persistence *persistence, QObject *parent) : QObject(parent),
    client_(NULL), persistence_(persistence)
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
    std::vector< boost::shared_ptr<Swift::RawXMLPayload> > xmlPayloads = message->getPayloads<Swift::RawXMLPayload>();
    for (std::vector<boost::shared_ptr<Swift::RawXMLPayload>>::iterator it = xmlPayloads.begin() ; it != xmlPayloads.end(); ++it)
    {
        QString rawXml = QString::fromStdString((*it)->getRawXML());

        if (rawXml.contains(chatMarkersIdentifier))
        {
            //qDebug() << "found:" << rawXml;

            QString id = getIdFromRawXml(rawXml);
            if (id.length() > 0)
            {
                if (rawXml.contains("displayed"))
                {
                    persistence_->markMessageAsDisplayedId(id);
                    break;
                }

                if (rawXml.contains("received"))
                {
                    persistence_->markMessageAsReceivedById(id);
                    break;
                }

                // acknowledged ?!
            }

            break;
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

QString ChatMarkers::getIdFromRawXml(QString rawXml)
{
    // just for this, we dont use a fat and complicated xml parser...
    QString returnValue = "";

    QRegularExpression re("id=(\"|')([^\\s]+)(\"|')");
    QRegularExpressionMatch match = re.match(rawXml);
    if (match.hasMatch()) {
        returnValue = match.captured(2);
    }

    return returnValue;
}

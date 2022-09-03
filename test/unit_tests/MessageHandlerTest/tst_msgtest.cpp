#include <QtTest>

#include "tst_msgtest.h"
#include "FreeStanza.h"
#include "Swiften/EventLoop/Qt/QtEventLoop.h"

MsgTest::MsgTest()
{

}

MsgTest::~MsgTest()
{

}

void MsgTest::initTestCase()
{
    persistence_ = new Persistence();
    settings_ = new Settings();
    rosterController_ = new RosterController();
    lurchAdapter_ = new LurchAdapter();
    messageHandler_ = new MessageHandler(persistence_, settings_, rosterController_, lurchAdapter_);
    chatMarkers_  = new ChatMarkers(persistence_, rosterController_);

    Swift::QtEventLoop eventLoop;
    Swift::BoostNetworkFactories networkFactories(&eventLoop);

    client_ = new Swift::Client(Swift::JID("sos@jabber.ccc.de"), "sos", &networkFactories);

    messageHandler_->setupWithClient(client_);
}

void MsgTest::cleanupTestCase()
{
}

void MsgTest::testPlain1to1Msg()
{
    persistence_->clear();

    std::shared_ptr<Swift::Message> message(new Swift::Message());
    message->setFrom(Swift::JID("from@me.org/fromRes"));
    message->setTo(Swift::JID("to@you.org/toRes"));
    const std::string id{"abcdef-ghijk-lmn"};
    message->setID(id);
    const std::string body{"the body"};
    message->setBody(body);

    qDebug() << getSerializedStringFromMessage(message);

    messageHandler_->handleMessageReceived(message);

    QCOMPARE(persistence_->id_, QString::fromStdString(id));
    QCOMPARE(persistence_->jid_, "from@me.org");
    QCOMPARE(persistence_->resource_, "fromRes");
    QCOMPARE(persistence_->message_, QString::fromStdString(body));
    QCOMPARE(persistence_->type_, "txt");
    QCOMPARE(persistence_->direction_, 1);
    QCOMPARE(persistence_->security_, 0);
    QCOMPARE(persistence_->timestamp_, 0);
}

/*
<message to="sos@jabber-germany.de/shmoose.BRjADesktop" type="chat" lang="en" from="mosos@jabber.de/shmoose.QSwh">
 <received xmlns="urn:xmpp:receipts" id="6907688f-3306-4910-8251-0854ce188f70"></received>
</message>
 */

// test 1o1 received stanza
void MsgTest::testPlain1to1ReceivedMsg()
{
    persistence_->clear();

    std::shared_ptr<Swift::Message> message(new Swift::Message());
    message->setFrom(Swift::JID("from@me.org/fromRes"));
    message->setTo(Swift::JID("to@you.org/toRes"));
    const std::string id{"abcdef-ghijk-lmn"};
    message->setID(id);

    // received stanza
    std::shared_ptr<Swift::DeliveryReceipt> rcpt(new Swift::DeliveryReceipt());
    rcpt->setReceivedID("id-received-id");

    message->addPayload(rcpt);

    qDebug() << getSerializedStringFromMessage(message);

    messageHandler_->handleMessageReceived(message);

    QCOMPARE(persistence_->message_, "");
    QCOMPARE(persistence_->security_, 0);
    QCOMPARE(persistence_->timestamp_, 0);
    QCOMPARE(persistence_->receivedId_, "id-received-id");
}


/*
<message id="5dcb8d6e-56c6-48f4-8627-6c6899b7b99e" to="sos@jabber-germany.de" lang="en" from="mosos@jabber.de/shmoose.QSwh">
 <displayed xmlns="urn:xmpp:chat-markers:0" id="a8c23e15-d71f-4029-abdc-638eb1b62683"></displayed>
</message>
 */
// test 1o1 displayed stanza
void MsgTest::testPlain1to1DisplayedMsg()
{
    persistence_->clear();

    std::shared_ptr<Swift::Message> message(new Swift::Message());
    message->setFrom(Swift::JID("from@me.org/fromRes"));
    message->setTo(Swift::JID("to@you.org/toRes"));
    const std::string id{"abcdef-ghijk-lmn"};
    message->setID(id);

    // generate a displayed stanza
    std::shared_ptr<Swift::RawXMLPayload> displayed(new Swift::RawXMLPayload());
    displayed->setRawXML("<displayed xmlns=\"urn:xmpp:chat-markers:0\" id=\"id-displayed-id\" sender=\"sos@jabber-germany.de/sosccc\"></displayed>");

    message->addPayload(displayed);

    qDebug() << getSerializedStringFromMessage(message);

    chatMarkers_->handleMessageReceived(message);

    QCOMPARE(persistence_->message_, "");
    QCOMPARE(persistence_->security_, 0);
    QCOMPARE(persistence_->timestamp_, 0);
    QCOMPARE(persistence_->idDisplayed_, "id-displayed-id");
}

void MsgTest::test1o1MsgInsideMam()
{
    persistence_->clear();

    // generate a Forwardd stanza with that delay
    std::shared_ptr<Swift::Forwarded> fwd(new Swift::Forwarded());

    // generate a stanza for the msg inside the mam and fwd container
    std::shared_ptr<Swift::Message> msgStz(new Swift::Message());
    msgStz->setFrom("sos@jabber.ccc.de/sosccc");
    msgStz->setID("aac5cd6e-23b7-4eda-900a-ff838a1b3ade");
    msgStz->setType(Swift::Message::Chat);
    msgStz->setBody("An other mam");
    fwd->setStanza(msgStz);

    // add the forwarded stanza to mam
    std::shared_ptr<Swift::MAMResult> mam(new Swift::MAMResult());
    mam->setID("2022-07-13-88497cbc5a054b5f");
    mam->setPayload(fwd);

    // finally, make the outer message
    std::shared_ptr<Swift::Message> message(new Swift::Message());
    message->setTo(Swift::JID("sos@jabber-germany.de/shmoose.BRjADesktop"));
    message->setFrom("server@jabber.ccc.de");

    message->addPayload(mam);

    qDebug() << getSerializedStringFromMessage(message);
    messageHandler_->handleMessageReceived(message);

    QCOMPARE(persistence_->id_, "aac5cd6e-23b7-4eda-900a-ff838a1b3ade");
    QCOMPARE(persistence_->jid_, "sos@jabber.ccc.de");
    QCOMPARE(persistence_->resource_, "sosccc");
    QCOMPARE(persistence_->message_, "An other mam");
    QCOMPARE(persistence_->type_, "txt");
    QCOMPARE(persistence_->direction_, 1);
    QCOMPARE(persistence_->security_, 0);
    QCOMPARE(persistence_->timestamp_, 0);
    QCOMPARE(messageHandler_->isGroupMessage_, false);
}

// FIXME test 1o1 received stanza inside mam
// FIXME test 1o1 displayed stanza inside mam


void MsgTest::testPlainRoomMsg()
{
    persistence_->clear();

    std::shared_ptr<Swift::Message> message(new Swift::Message());
    message->setFrom(Swift::JID("room@somewhere.org/fromRes"));
    message->setTo(Swift::JID("me@home.org/toRes"));
    const std::string id{"abcdef-ghijk-lmn"};
    message->setID(id);
    message->setType(Swift::Message::Groupchat);
    const std::string body{"the body"};
    message->setBody(body);

    qDebug() << getSerializedStringFromMessage(message);

    messageHandler_->handleMessageReceived(message);

    QCOMPARE(persistence_->id_, QString::fromStdString(id));
    QCOMPARE(persistence_->jid_, "room@somewhere.org");
    QCOMPARE(persistence_->resource_, "fromRes");
    QCOMPARE(persistence_->message_, QString::fromStdString(body));
    QCOMPARE(persistence_->type_, "txt");
    QCOMPARE(persistence_->direction_, 1);
    QCOMPARE(persistence_->security_, 0);
    QCOMPARE(persistence_->timestamp_, 0);
}

void MsgTest::testPlainRoomWithTimestampMsg()
{
    persistence_->clear();

    std::shared_ptr<Swift::Message> message(new Swift::Message());
    message->setFrom(Swift::JID("room@somewhere.org/fromRes"));
    message->setTo(Swift::JID("me@home.org/toRes"));
    const std::string id{"abcdef-ghijk-lmn"};
    message->setID(id);
    message->setType(Swift::Message::Groupchat);
    const std::string body{"the body"};
    message->setBody(body);

    // generate delay payload
    std::shared_ptr<Swift::Delay> delay(new Swift::Delay());
    boost::posix_time::ptime t1(boost::posix_time::time_from_string("1970-01-01 00:00:00.000"));
    delay->setStamp(t1);

    message->addPayload(delay);

    qDebug() << getSerializedStringFromMessage(message);
    messageHandler_->handleMessageReceived(message);

    QCOMPARE(persistence_->id_, QString::fromStdString(id));
    QCOMPARE(persistence_->jid_, "room@somewhere.org");
    QCOMPARE(persistence_->resource_, "fromRes");
    QCOMPARE(persistence_->message_, QString::fromStdString(body));
    QCOMPARE(persistence_->type_, "txt");
    QCOMPARE(persistence_->direction_, 1);
    QCOMPARE(persistence_->security_, 0);
    QCOMPARE(persistence_->timestamp_, 0);
}


/*
        <message xmlns="jabber:client" to="sos@jabber-germany.de/shmoose.BRjADesktop" from="room@conference.jabber-germany.de">
         <result xmlns="urn:xmpp:mam:2" id="XnFo0EDjYBHhDL5GqxOOtAsP">
          <forwarded xmlns="urn:xmpp:forward:0">
          <delay xmlns="urn:xmpp:delay" stamp="2022-07-15T18:20:07Z"></delay>
           <message xmlns="jabber:client" type="groupchat" lang="en" from="room@conference.jabber-germany.de/sosccc" id="aac5cd6e-23b7-4eda-900a-ff838a1b3ade">
            <markable xmlns="urn:xmpp:chat-markers:0"></markable>
            <active xmlns="http://jabber.org/protocol/chatstates"></active>
            <body>An other mam</body>
          </message>
          </forwarded>
         </result>
        </message>
*/
void MsgTest::testPlainRoomMsgInsideMam()
{
    persistence_->clear();

    // generate delay payload
    std::shared_ptr<Swift::Delay> delay(new Swift::Delay());
    boost::posix_time::ptime t1(boost::posix_time::time_from_string("1970-01-01 00:00:00.000"));
    delay->setStamp(t1);

    // generate a Forwardd stanza with that delay
    std::shared_ptr<Swift::Forwarded> fwd(new Swift::Forwarded());
    fwd->setDelay(delay);

    // generate a stanza for the msg inside the mam and fwd container
    std::shared_ptr<Swift::Message> msgStz(new Swift::Message());
    msgStz->setFrom("room@conference.jabber-germany.de/sosccc");
    msgStz->setID("aac5cd6e-23b7-4eda-900a-ff838a1b3ade");
    msgStz->setType(Swift::Message::Groupchat);
    msgStz->setBody("An other mam");
    fwd->setStanza(msgStz);


    // add the forwarded stanza to mam
    std::shared_ptr<Swift::MAMResult> mam(new Swift::MAMResult());
    mam->setID("2022-07-13-88497cbc5a054b5f");
    mam->setPayload(fwd);

    // finally, make the outer message
    std::shared_ptr<Swift::Message> message(new Swift::Message());
    message->setTo(Swift::JID("sos@jabber-germany.de/shmoose.BRjADesktop"));
    message->setFrom("room@conference.jabber-germany.de");

    message->addPayload(mam);

    qDebug() << getSerializedStringFromMessage(message);
    messageHandler_->handleMessageReceived(message);

    QCOMPARE(persistence_->id_, "aac5cd6e-23b7-4eda-900a-ff838a1b3ade");
    QCOMPARE(persistence_->jid_, "room@conference.jabber-germany.de");
    QCOMPARE(persistence_->resource_, "sosccc");
    QCOMPARE(persistence_->message_, "An other mam");
    QCOMPARE(persistence_->type_, "txt");
    QCOMPARE(persistence_->direction_, 1);
    QCOMPARE(persistence_->security_, 0);
    QCOMPARE(persistence_->timestamp_, 0);
    QCOMPARE(messageHandler_->isGroupMessage_, true);
}

// this mam msg must not be pushed into the db.
// there is _NO_ ID inside the mam msg. Shmoose needs it as a primary key.
// If this won't be skipped, we end up with endless messages on every mam request.
void MsgTest::testPlainRoomMsgWithoutIdInsideMam()
{
    persistence_->clear();

    // generate a Forwardd stanza with that delay
    std::shared_ptr<Swift::Forwarded> fwd(new Swift::Forwarded());

    // generate a stanza for the msg inside the mam and fwd container
    std::shared_ptr<Swift::Message> msgStz(new Swift::Message());
    msgStz->setFrom("room@conference.jabber-germany.de/sosccc");
    //msgStz->setID("foo-id"); // this must be missing for the test!! thats an other clients misstake.
    msgStz->setType(Swift::Message::Groupchat);
    msgStz->setBody("An other mam");
    fwd->setStanza(msgStz);

    // add the forwarded stanza to mam
    std::shared_ptr<Swift::MAMResult> mam(new Swift::MAMResult());
    mam->setID("2022-07-13-88497cbc5a054b5f");
    mam->setPayload(fwd);

    // finally, make the outer message
    std::shared_ptr<Swift::Message> message(new Swift::Message());
    message->setTo(Swift::JID("sos@jabber-germany.de/shmoose.BRjADesktop"));
    message->setFrom("room@conference.jabber-germany.de");

    message->addPayload(mam);

    qDebug() << getSerializedStringFromMessage(message);
    messageHandler_->handleMessageReceived(message);

    QCOMPARE(persistence_->id_, "");
    QCOMPARE(persistence_->jid_, "");
    QCOMPARE(persistence_->resource_, "");
    QCOMPARE(persistence_->message_, "");
    QCOMPARE(persistence_->type_, "");
    QCOMPARE(persistence_->direction_, 255);
    QCOMPARE(persistence_->security_, 0);
    QCOMPARE(persistence_->timestamp_, 0);
    QCOMPARE(messageHandler_->isGroupMessage_, true);
}

/*
 <message xmlns="jabber:client" to="sos@jabber-germany.de/shmoose.BRjADesktop" from="room@conference.jabber-germany.de">
  <result xmlns="urn:xmpp:mam:2" id="qVsnHaEJM9cia3H5JjXEYCn0">
   <forwarded xmlns="urn:xmpp:forward:0">
    <delay xmlns="urn:xmpp:delay" stamp="2022-07-15T19:15:58Z"></delay>
    <message xmlns="jabber:client" type="groupchat" lang="en" from="room@conference.jabber-germany.de/mosos(at)jabber.de" id="253a435d-2783-43c6-9cc9-61a01e635bfd">
     <displayed xmlns="urn:xmpp:chat-markers:0" id="aac5cd6e-23b7-4eda-900a-ff838a1b3ade" sender="room@conference.jabber-germany.de/sosccc"></displayed>
    </message>
   </forwarded>
  </result>
 </message>
*/

void MsgTest::testDisplayedRoomMsgInsideMam()
{
    persistence_->clear();

    // generate delay payload
    std::shared_ptr<Swift::Delay> delay(new Swift::Delay());
    boost::posix_time::ptime t1(boost::posix_time::max_date_time);
    delay->setStamp(t1);

    // generate a Forwardd stanza with that delay
    std::shared_ptr<Swift::Forwarded> fwd(new Swift::Forwarded());
    fwd->setDelay(delay);

    // generate a displayed stanza
    std::shared_ptr<Swift::RawXMLPayload> displayed(new Swift::RawXMLPayload());
    displayed->setRawXML("<displayed xmlns=\"urn:xmpp:chat-markers:0\" id=\"aac5cd6e-23b7-4eda-900a-ff838a1b3ade\" sender=\"room@conference.jabber-germany.de/sosccc\"></displayed>");

    // generate a stanza for the msg inside the mam and fwd container
    std::shared_ptr<Swift::Message> msgStz(new Swift::Message());
    msgStz->setFrom("room@conference.jabber-germany.de/mosos(at)jabber.de");
    msgStz->setID("253a435d-2783-43c6-9cc9-61a01e635bfd");
    msgStz->setType(Swift::Message::Groupchat);
    msgStz->addPayload(displayed);
    fwd->setStanza(msgStz);


    // add the forwarded stanza to mam
    std::shared_ptr<Swift::MAMResult> mam(new Swift::MAMResult());
    mam->setPayload(fwd);

    // finally, make the outer message
    std::shared_ptr<Swift::Message> message(new Swift::Message());
    message->setTo(Swift::JID("sos@jabber-germany.de/shmoose.BRjADesktop"));
    message->setFrom("room@conference.jabber-germany.de");

    message->addPayload(mam);

    qDebug() << getSerializedStringFromMessage(message);
    messageHandler_->handleMessageReceived(message);

    QCOMPARE(persistence_->idDisplayed_, "aac5cd6e-23b7-4eda-900a-ff838a1b3ade");
    QCOMPARE(persistence_->resourceDisplayed_, "mosos(at)jabber.de");
}

// FIXME test room received stanza inside mam


// FIXME test all of this again with omemo encryption

QString MsgTest::getSerializedStringFromMessage(Swift::Message::ref msg)
{
    Swift::FullPayloadSerializerCollection serializers_;
    Swift::XMPPSerializer xmppSerializer(&serializers_, Swift::ClientStreamType, true);
    Swift::SafeByteArray sba = xmppSerializer.serializeElement(msg);

    return QString::fromStdString(Swift::safeByteArrayToString(sba));
}

QTEST_APPLESS_MAIN(MsgTest)

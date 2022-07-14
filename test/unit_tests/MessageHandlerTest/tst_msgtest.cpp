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
#if 0
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
#endif
}

void MsgTest::testPlainRoomMsg()
{
#if 0
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
#endif
}

void MsgTest::testPlainRoomWithTimestampMsg()
{
#if 0
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
    boost::posix_time::ptime t1(boost::posix_time::max_date_time);
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
    QCOMPARE(persistence_->timestamp_, 253402297199);
#endif
}


#if 0
<message xmlns="jabber:client" to="user@jabber.foo/shmoose.A">
 <result xmlns="urn:xmpp:mam:2" id="2022-07-13-88497cbc5a054b5f">
  <forwarded xmlns="urn:xmpp:forward:0">
   <delay xmlns="urn:xmpp:delay" stamp="2022-07-13T04:11:09Z"></delay>
   <message xmlns="jabber:client" type="chat" to="someone@jabber.foo" from="user@jabber.foo/shmoose.B" lang="en">
    <body>a mam body</body>
    <subject></subject>
   </message>
  </forwarded>
 </result>
</message>
#endif

void MsgTest::testPlainRoomMsgInsideMam()
{
    persistence_->clear();

    // generate delay payload
    std::shared_ptr<Swift::Delay> delay(new Swift::Delay());
    boost::posix_time::ptime t1(boost::posix_time::max_date_time);
    delay->setStamp(t1);

    // generate a Forwardd stanza with that delay
    std::shared_ptr<Swift::Forwarded> fwd(new Swift::Forwarded());
    fwd->setDelay(delay);

    // generate a body payload
    std::shared_ptr<Swift::RawXMLPayload> bodyPayload(new Swift::RawXMLPayload());
    bodyPayload->setRawXML("<body>a mam msg!</body>");

    // generate a stanza for the msg inside the mam and fwd container
    std::shared_ptr<Swift::Message> msgStz(new Swift::Message());
    msgStz->setFrom("user@jabber.foo/shmoose.B");
    msgStz->setTo("someone@jabber.foo");
    msgStz->setType(Swift::Message::Chat);
    msgStz->setBody("a mam body!");
    fwd->setStanza(msgStz);


    // add the forwarded stanza to mam
    std::shared_ptr<Swift::MAMResult> mam(new Swift::MAMResult());
    mam->setID("2022-07-13-88497cbc5a054b5f");
    mam->setPayload(fwd);

    // finally, make the outer message
    std::shared_ptr<Swift::Message> message(new Swift::Message());
    message->setTo(Swift::JID("user@jabber.foo/shmoose.A"));
    //const std::string id{"abcdef-ghijk-lmn"};
    //message->setID(id);
    //message->setType(Swift::Message::Normal);

    message->addPayload(mam);

    qDebug() << getSerializedStringFromMessage(message);
    messageHandler_->handleMessageReceived(message);

    QCOMPARE(persistence_->id_, "2022-07-13-88497cbc5a054b5f");
    QCOMPARE(persistence_->jid_, "user@jabber.foo");
    QCOMPARE(persistence_->resource_, "shmoose.B");
    QCOMPARE(persistence_->message_, "a mam body!");
    QCOMPARE(persistence_->type_, "txt");
    QCOMPARE(persistence_->direction_, 1);
    QCOMPARE(persistence_->security_, 0);
    QCOMPARE(persistence_->timestamp_, 253402297199);
    QCOMPARE(messageHandler_->isGroupMessage_, false);
}


// FIXME write a test for a displayed mam!
#if 0
        <message xmlns="jabber:client" to="user@jabber.foo/shmoose.A">
         <result xmlns="urn:xmpp:mam:2" id="2022-07-13-cdccd86d5cd6422c">
          <forwarded xmlns="urn:xmpp:forward:0">
           <delay xmlns="urn:xmpp:delay" stamp="2022-07-13T04:13:16Z"></delay>
           <message xmlns="jabber:client" to="user@jabber.foo" from="someone@jabber.foo/shmoose.QSwh" id="ea3a7d56-0e58-45d5-ab63-e1c91a3e5817" lang="en">
            <displayed xmlns="urn:xmpp:chat-markers:0" id="1657685469988"></displayed>
           </message>
          </forwarded>
         </result>
        </message>
#endif


QString MsgTest::getSerializedStringFromMessage(Swift::Message::ref msg)
{
    Swift::FullPayloadSerializerCollection serializers_;
    Swift::XMPPSerializer xmppSerializer(&serializers_, Swift::ClientStreamType, true);
    Swift::SafeByteArray sba = xmppSerializer.serializeElement(msg);

    return QString::fromStdString(Swift::safeByteArrayToString(sba));
}

QTEST_APPLESS_MAIN(MsgTest)

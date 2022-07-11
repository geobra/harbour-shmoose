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
}

void MsgTest::testPlainRoomMsgInsideMam()
{
    persistence_->clear();

    // generate delay payload
    std::shared_ptr<Swift::Delay> delay(new Swift::Delay());
    boost::posix_time::ptime t1(boost::posix_time::max_date_time);

    // generate a Forwardd stanza with that delay
    std::shared_ptr<Swift::Forwarded> fwd(new Swift::Forwarded());
    fwd->setDelay(delay);

    // generate a body payload
    std::shared_ptr<Swift::RawXMLPayload> bodyPayload(new Swift::RawXMLPayload());
    bodyPayload->setRawXML("<body>a mam msg!</body>");

    // generate a stanza for the msg inside the mam and fwd container
    std::shared_ptr<FreeStanza> stz(new FreeStanza());
    stz->setFrom("from@mam.org");
    stz->setTo("to@mam.org");
    stz->addPayload(bodyPayload);
    fwd->setStanza(stz);

    // add thr forwarded stanza to mam
    std::shared_ptr<Swift::MAMResult> mam(new Swift::MAMResult());
    mam->setPayload(fwd);

    std::shared_ptr<Swift::Message> message(new Swift::Message());
    message->setFrom(Swift::JID("room@somewhere.org/fromRes"));
    message->setTo(Swift::JID("me@home.org/toRes"));
    const std::string id{"abcdef-ghijk-lmn"};
    message->setID(id);
    message->setType(Swift::Message::Normal);

    message->addPayload(mam);

    qDebug() << getSerializedStringFromMessage(message);
    messageHandler_->handleMessageReceived(message);

    /*
     * <message from=\"room@somewhere.org/fromRes\" id=\"abcdef-ghijk-lmn\" to=\"me@home.org/toRes\" type=\"groupchat\" xmlns=\"jabber:client\">
     * <body>the body</body>
     * <result id=\"\" xmlns=\"urn:xmpp:mam:0\">
     * <forwarded xmlns=\"urn:xmpp:forward:0\">
     * <delay stamp=\"not-a-date-timeZ\" xmlns=\"urn:xmpp:delay\"/>
     * </forwarded></result></message>
     */

#if 0
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

QString MsgTest::getSerializedStringFromMessage(Swift::Message::ref msg)
{
    Swift::FullPayloadSerializerCollection serializers_;
    Swift::XMPPSerializer xmppSerializer(&serializers_, Swift::ClientStreamType, true);
    Swift::SafeByteArray sba = xmppSerializer.serializeElement(msg);

    return QString::fromStdString(Swift::safeByteArrayToString(sba));
}

QTEST_APPLESS_MAIN(MsgTest)

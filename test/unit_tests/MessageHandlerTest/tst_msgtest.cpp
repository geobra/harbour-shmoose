#include <QtTest>

#include "tst_msgtest.h"

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

QTEST_APPLESS_MAIN(MsgTest)

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


void MsgTest::testSimple1to1Msg()
{
    std::shared_ptr<Swift::Message> message(new Swift::Message());
    message->setFrom(Swift::JID("from@me.org/fromRes"));
    message->setTo(Swift::JID("to@you.org/toRes"));
    const std::string id{"abcdef-ghijk-lmn"};
    message->setID(id);
    const std::string body{"the body"};
    message->setBody(body);


    persistence_->clear();
    messageHandler_->handleMessageReceived(message);

    QCOMPARE(persistence_->id_, QString::fromStdString(id));
    QCOMPARE(persistence_->jid_, "from@me.org");
    QCOMPARE(persistence_->resource_, "fromRes");
    QCOMPARE(persistence_->message_, QString::fromStdString(body));
    QCOMPARE(persistence_->type_, "txt");
    QCOMPARE(persistence_->direction_, 1);

}

QTEST_APPLESS_MAIN(MsgTest)

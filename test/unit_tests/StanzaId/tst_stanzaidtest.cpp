#include <QtTest>

#include "tst_stanzaidtest.h"

#include "StanzaIdPayloadParserTester.h"

StanzaIdTest::StanzaIdTest()
{

}

StanzaIdTest::~StanzaIdTest()
{

}

void StanzaIdTest::initTestCase()
{

}

void StanzaIdTest::cleanupTestCase()
{

}

void StanzaIdTest::testParseStanzaId()
{
    StanzaIdPayloadParserTester parser;
    QVERIFY(parser.parse("<stanza-id id=\"76F899B4-5F57-47E0-9180-F8EE2BDB0CBE\" by=\"sos@jabber.ccc.de\" xmlns=\"urn:xmpp:sid:0\"/>"));
    std::shared_ptr<StanzaIdPayload> payload = parser.getPayload();
    QCOMPARE(payload->getId(), "76F899B4-5F57-47E0-9180-F8EE2BDB0CBE");
    QCOMPARE(payload->getBy(), "sos@jabber.ccc.de");
}

void StanzaIdTest::testSerializeStanzaId()
{
    StanzaIdPayloadSerializer serializer;
    std::shared_ptr<StanzaIdPayload> payload(new StanzaIdPayload());
    payload->setId("76F899B4-5F57-47E0-9180-F8EE2BDB0CBE");
    payload->setBy("sos@jabber.ccc.de");
    std::string serialized = serializer.serializePayload(payload);
    QCOMPARE(serialized, "<stanza-id by=\"sos@jabber.ccc.de\" id=\"76F899B4-5F57-47E0-9180-F8EE2BDB0CBE\" xmlns=\"urn:xmpp:sid:0\"/>");
}

void StanzaIdTest::testSetupWithClient()
{
    Swift::DummyEventLoop eventLoop;
    Swift::BoostNetworkFactories networkFactories(&eventLoop);
    Swift::Client *client = new Swift::Client(Swift::JID("sos@jabber.ccc.de"), "sos", &networkFactories);
    StanzaId *stanzaId = new StanzaId(this);
    stanzaId->setupWithClient(client);
    delete client;
    delete stanzaId;
}

QTEST_APPLESS_MAIN(StanzaIdTest)

#include <QtTest>

#include "tst_mamtest.h"

#include "Swiften/EventLoop/Qt/QtEventLoop.h"

MamTest::MamTest()
{

}

MamTest::~MamTest()
{

}

void MamTest::initTestCase()
{
    persistence_ = new Persistence();
    mamManager_ = new MamManager(persistence_);

    Swift::QtEventLoop eventLoop;
    Swift::BoostNetworkFactories networkFactories(&eventLoop);

    client_ = new Swift::Client(Swift::JID("sos@jabber.ccc.de"), "sos", &networkFactories);

    mamManager_->setupWithClient(client_);
}

void MamTest::cleanupTestCase()
{
    delete  mamManager_;
    //delete persistence_;
}


void MamTest::test1o1MamMsgIncoming()
{
    Swift::SafeByteArray sba = Swift::createSafeByteArray("<message xmlns=\"jabber:client\" to=\"sos@jabber.ccc.de/shmooseDesktop\" from=\"sos@jabber.ccc.de\"> \
                                                          <result xmlns=\"urn:xmpp:mam:2\" id=\"1593026088927513\"> \
                                                           <forwarded xmlns=\"urn:xmpp:forward:0\"> \
                                                            <message xmlns=\"jabber:client\" to=\"sos@jabber.ccc.de\" from=\"ms@jabber.de/shmoose\" type=\"chat\" id=\"137e1f50-57bb-48f1-8188-0b13d02a503e\"> \
                                                             <archived xmlns=\"urn:xmpp:mam:tmp\" by=\"sos@jabber.ccc.de\" id=\"1593026088927513\"></archived> \
                                                             <stanza-id xmlns=\"urn:xmpp:sid:0\" by=\"sos@jabber.ccc.de\" id=\"1593026088927513\"></stanza-id> \
                                                             <request xmlns=\"urn:xmpp:receipts\"></request> \
                                                             <markable xmlns=\"urn:xmpp:chat-markers:0\"></markable> \
                                                             <body>1</body> \
                                                            </message> \
                                                            <delay xmlns=\"urn:xmpp:delay\" from=\"jabber.ccc.de\" stamp=\"2020-06-24T19:14:48.927513Z\"></delay> \
                                                           </forwarded> \
                                                          </result> \
                                                         </message>");

    persistence_->clear();
    mamManager_->handleDataReceived(sba);
    QCOMPARE(persistence_->id_, "137e1f50-57bb-48f1-8188-0b13d02a503e");
    QCOMPARE(persistence_->jid_, "ms@jabber.de");
    QCOMPARE(persistence_->resource_, "shmoose");
    QCOMPARE(persistence_->message_, "1");
    QCOMPARE(persistence_->type_, "txt");
    QCOMPARE(persistence_->direction_, 1);
}

void MamTest::test1o1MamMsgOutgoing()
{
    Swift::SafeByteArray sba = Swift::createSafeByteArray("<message xmlns=\"jabber:client\" to=\"sos@jabber.ccc.de/shmooseDesktop\" from=\"sos@jabber.ccc.de\"> \
                                                          <result xmlns=\"urn:xmpp:mam:2\" id=\"1593026094395928\"> \
                                                           <forwarded xmlns=\"urn:xmpp:forward:0\"> \
                                                            <message xmlns=\"jabber:client\" lang=\"en\" to=\"ms@jabber.de\" from=\"sos@jabber.ccc.de/shmooseDesktop\" type=\"chat\" id=\"bf92dd33-5f35-48f7-8b87-800337ceaaa7\"> \
                                                             <archived xmlns=\"urn:xmpp:mam:tmp\" by=\"sos@jabber.ccc.de\" id=\"1593026094395928\"></archived> \
                                                             <stanza-id xmlns=\"urn:xmpp:sid:0\" by=\"sos@jabber.ccc.de\" id=\"1593026094395928\"></stanza-id> \
                                                             <request xmlns=\"urn:xmpp:receipts\"></request> \
                                                             <markable xmlns=\"urn:xmpp:chat-markers:0\"></markable> \
                                                             <body>2</body> \
                                                            </message> \
                                                            <delay xmlns=\"urn:xmpp:delay\" from=\"jabber.ccc.de\" stamp=\"2020-06-24T19:14:54.395928Z\"></delay> \
                                                           </forwarded> \
                                                          </result> \
                                                         </message>");

    persistence_->clear();
    mamManager_->handleDataReceived(sba);
    QCOMPARE(persistence_->id_, "bf92dd33-5f35-48f7-8b87-800337ceaaa7");
    QCOMPARE(persistence_->jid_, "ms@jabber.de");
    QCOMPARE(persistence_->resource_, "");
    QCOMPARE(persistence_->message_, "2");
    QCOMPARE(persistence_->type_, "txt");
    QCOMPARE(persistence_->direction_, 0);
}

void MamTest::testRoomMamMsgIncoming()
{
    Swift::SafeByteArray sba = Swift::createSafeByteArray("<message xmlns=\"jabber:client\" to=\"sos@jabber.ccc.de/shmooseDesktop\" from=\"mg@conference.jabber.ccc.de\"> \
                                                          <result xmlns=\"urn:xmpp:mam:2\" id=\"1591439483304165\"> \
                                                           <forwarded xmlns=\"urn:xmpp:forward:0\"> \
                                                            <message xmlns=\"jabber:client\" from=\"mg@conference.jabber.ccc.de/ms(at)jabber.de\" type=\"groupchat\" id=\"4cb33155-2b57-496a-ad80-2a9c7f31639b\"> \
                                                             <x xmlns=\"http://jabber.org/protocol/muc#user\"> \
                                                              <item jid=\"ms@jabber.de/shmoose\"></item> \
                                                             </x> \
                                                             <archived xmlns=\"urn:xmpp:mam:tmp\" by=\"mg@conference.jabber.ccc.de\" id=\"1591439483304165\"></archived> \
                                                             <stanza-id xmlns=\"urn:xmpp:sid:0\" by=\"mg@conference.jabber.ccc.de\" id=\"1591439483304165\"></stanza-id> \
                                                             <request xmlns=\"urn:xmpp:receipts\"></request> \
                                                             <markable xmlns=\"urn:xmpp:chat-markers:0\"></markable> \
                                                             <body>foo msg</body> \
                                                            </message> \
                                                            <delay xmlns=\"urn:xmpp:delay\" from=\"conference.jabber.ccc.de\" stamp=\"2020-06-06T10:31:23.304165Z\"></delay> \
                                                           </forwarded> \
                                                          </result> \
                                                         </message>");

    persistence_->clear();
    mamManager_->handleDataReceived(sba);
    QCOMPARE(persistence_->id_, "4cb33155-2b57-496a-ad80-2a9c7f31639b");
    QCOMPARE(persistence_->jid_, "mg@conference.jabber.ccc.de");
    QCOMPARE(persistence_->resource_, "ms(at)jabber.de");
    QCOMPARE(persistence_->message_, "foo msg");
    QCOMPARE(persistence_->type_, "txt");
    QCOMPARE(persistence_->direction_, 1);
}

void MamTest::testRoomMamMsgOutgoing()
{
    Swift::SafeByteArray sba = Swift::createSafeByteArray("<message xmlns=\"jabber:client\" to=\"sos@jabber.ccc.de/shmooseDesktop\" from=\"mg@conference.jabber.ccc.de\"> \
                                                          <result xmlns=\"urn:xmpp:mam:2\" id=\"1593636669754332\"> \
                                                           <forwarded xmlns=\"urn:xmpp:forward:0\"> \
                                                            <message xmlns=\"jabber:client\" lang=\"en\" from=\"mg@conference.jabber.ccc.de/sos-ccc\" type=\"groupchat\" id=\"f3ec3d8d-a3e0-4b37-89f6-6e661fe6bee3\"> \
                                                             <x xmlns=\"http://jabber.org/protocol/muc#user\"> \
                                                              <item jid=\"sos@jabber.ccc.de/Conversations.97pX\"></item> \
                                                             </x> \
                                                             <archived xmlns=\"urn:xmpp:mam:tmp\" by=\"mg@conference.jabber.ccc.de\" id=\"1593636669754332\"></archived> \
                                                             <stanza-id xmlns=\"urn:xmpp:sid:0\" by=\"mg@conference.jabber.ccc.de\" id=\"1593636669754332\"></stanza-id> \
                                                             <markable xmlns=\"urn:xmpp:chat-markers:0\"></markable> \
                                                             <origin-id xmlns=\"urn:xmpp:sid:0\" id=\"f3ec3d8d-a3e0-4b37-89f6-6e661fe6bee3\"></origin-id> \
                                                             <active xmlns=\"http://jabber.org/protocol/chatstates\"></active> \
                                                             <body>bar msg</body> \
                                                            </message> \
                                                            <delay xmlns=\"urn:xmpp:delay\" from=\"conference.jabber.ccc.de\" stamp=\"2020-07-01T20:51:09.754332Z\"></delay> \
                                                           </forwarded> \
                                                          </result> \
                                                         </message>");

    persistence_->clear();
    mamManager_->handleDataReceived(sba);
    QCOMPARE(persistence_->id_, "f3ec3d8d-a3e0-4b37-89f6-6e661fe6bee3");
    QCOMPARE(persistence_->jid_, "mg@conference.jabber.ccc.de");
    QCOMPARE(persistence_->resource_, "sos-ccc");
    QCOMPARE(persistence_->message_, "bar msg");
    QCOMPARE(persistence_->type_, "txt");
    QCOMPARE(persistence_->direction_, 0);
}


QTEST_APPLESS_MAIN(MamTest)

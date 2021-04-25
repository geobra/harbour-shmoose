#include <QtTest>

#include "tst_lurchadapter.h"
#include "LurchAdapter.h"
#include "StringPayload.h"

//#include "Swiften/EventLoop/Qt/QtEventLoop.h"

LurchAdapterTest::LurchAdapterTest()
{

}

LurchAdapterTest::~LurchAdapterTest()
{

}

void LurchAdapterTest::initTestCase()
{

}

void LurchAdapterTest::cleanupTestCase()
{

}


void LurchAdapterTest::test1o1MamMsgIncoming()
{
#if 0
    <message xmlns="jabber:client" lang="en" to="xxx@jabber.ccc.de/shmooseDesktop" from="bbb@conference.blabber.im/libersum" type="groupchat" id="12bb22c1-9978-4e8e-91ca-82a5302cfa80">
     <archived xmlns="urn:xmpp:mam:tmp" by="bbb@conference.blabber.im" id="1612520430972962"></archived>
     <stanza-id xmlns="urn:xmpp:sid:0" by="bbb@conference.blabber.im" id="1612520430972962"></stanza-id>
     <origin-id xmlns="urn:xmpp:sid:0" id="12bb22c1-9978-4e8e-91ca-82a5302cfa80"></origin-id>
     <delay xmlns="urn:xmpp:delay" from="bbb@conference.blabber.im" stamp="2021-02-05T10:20:30.990752Z"></delay>
     <body>> sasd wrote:

       stuff.</body>
    </message>
   [warning] Swiften/Serializer/StanzaSerializer.cpp:56 serialize: Could not find serializer for 15StanzaIdPayload
   decryptMessageIfEncrypted (1: was not encrypted, 2: error during decryption). code:  1
   fromJid:  "bbb@conference.blabber.im" current:  "" , isGroup:  true , appActive?  true
   [warning] Swiften/Serializer/StanzaSerializer.cpp:56 serialize: Could not find serializer for 15StanzaIdPayload
#endif

    LurchAdapter la;

    StringPayload sp("bla");
#if 0
[warning] Swiften/Serializer/StanzaSerializer.cpp:56 serialize: Could not find serializer for 13StringPayload
#endif

    std::shared_ptr<Swift::Message> m1 = std::make_shared<Swift::Message>();
    m1->addPayload(std::make_shared<StringPayload>("bla"));

    qDebug() << la.getSerializedStringFromMessage(m1);

    std::shared_ptr<Swift::Message> m2 = std::make_shared<Swift::Message>();
    m2->setTo(Swift::JID("foo@bar.de"));
    auto enc = std::make_shared<Swift::RawXMLPayload>("<encrpted foo='bar'>asdfasfafasdfsfy</encrypted>");
    m2->addPayload(enc);
    qDebug() << la.getSerializedStringFromMessage(m2);
}


QTEST_APPLESS_MAIN(LurchAdapterTest)

#include <QtTest>

#include "tst_XmlProcessortest.h"

#include "XmlProcessor.h"

XmlProcessortest::XmlProcessortest()
{

}

XmlProcessortest::~XmlProcessortest()
{

}

void XmlProcessortest::test_case1()
{
    QString str{"<foo><bar><body>the body</body></bar></foo>"};

    QString match = XmlProcessor::getChildFromNode("bar", str);
    //qDebug() << match;
    QCOMPARE(match, "<bar><body>the body</body></bar>");


    match = XmlProcessor::getContentInElement("body", str);
    QCOMPARE(match, "the body");

    const char *m = R"(<message xmlns="jabber:client" to="user2@localhost/shmooseDesktop" from="user1@localhost" type="headline">
                       <event xmlns="http://jabber.org/protocol/pubsub#event">
                        <items node="eu.siacs.conversations.axolotl.devicelist">
                         <item id="64BBE01EA5421">
                          <list xmlns="eu.siacs.conversations.axolotl">
                           <device id="226687003"></device>
                          </list>
                         </item>
                        </items>
                       </event>
                       <addresses xmlns="http://jabber.org/protocol/address">
                        <address jid="user1@localhost/8832136518768075312" type="replyto"></address>
                       </addresses>
                      </message>)";

    QString mStr = QString::fromLatin1(m);
    match = XmlProcessor::getChildFromNode("items", mStr);
    QVERIFY(match.startsWith("<items"));

    match = XmlProcessor::getChildFromNode("address", mStr);
    QVERIFY(match.startsWith("<address "));

    str = "<x><a><a1></a1></a><b><b1></b1></b></x>";
    match = XmlProcessor::getChildFromNode("a", str);
    QCOMPARE(match, "<a><a1/></a>");

    match = XmlProcessor::getChildFromNode("b", str);
    QCOMPARE(match, "<b><b1/></b>");

    str = "<x><a><a1></a1><a2><c></c></a2></a><b><b1></b1></b></x>";
    match = XmlProcessor::getChildFromNode("a2", str);
    QCOMPARE(match, "<a2><c/></a2>");


}

QTEST_APPLESS_MAIN(XmlProcessortest)

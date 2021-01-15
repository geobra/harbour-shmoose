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
    QCOMPARE(match, "<bar><body>the body</body></bar>");


    match = XmlProcessor::getContentInElement("body", str);
    QCOMPARE(match, "the body");
}

QTEST_APPLESS_MAIN(XmlProcessortest)

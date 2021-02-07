#include <QtTest>
#include <string>

#include "tst_XmlNodeTest.h"

extern "C"
{
#include "xmlnode.h"
}

XmlNodeTest::XmlNodeTest()
{

}

XmlNodeTest::~XmlNodeTest()
{

}

void XmlNodeTest::test_case1()
{
    std::string str{"<foo><bar><body>the body</body></bar></foo>"};

    xmlnode* node = xmlnode_from_str(str.c_str(), -1);

    int len{0};
    char* cStr = xmlnode_to_str(node, &len);

    xmlnode_free(node);

    QVERIFY(str.compare(cStr) == 0);
    free(cStr);
}

QTEST_APPLESS_MAIN(XmlNodeTest)

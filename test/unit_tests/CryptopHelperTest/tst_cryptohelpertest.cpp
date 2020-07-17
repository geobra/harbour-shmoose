#include <QtTest>

#include "tst_cryptohelpertest.h"

#include "CryptoHelper.h"

cryptohelpertest::cryptohelpertest()
{

}

cryptohelpertest::~cryptohelpertest()
{

}

void cryptohelpertest::test_case1()
{
    QString str{"http://foo.bar/img.jpg"};

    QString hash = CryptoHelper::getHashOfString(str, true);
    QCOMPARE(hash, "aad5ec6c18cb20fd9c1a7548ecd2d7b9.jpg");

    hash = CryptoHelper::getHashOfString(str);
    QCOMPARE(hash, "aad5ec6c18cb20fd9c1a7548ecd2d7b9");

    str = "http://foo.bar/img.jpeg";
    hash = CryptoHelper::getHashOfString(str, true);
    QCOMPARE(hash, "19eaba83804e68bc1cd41ee859e12a12.jpeg");

    str = "http://foo.bar/img.mjpeg";
    hash = CryptoHelper::getHashOfString(str, true);
    QCOMPARE(hash, "d92fde787c9a81f9a5dfc2afb394bc97.mjpeg");

    str = "http://foo.bar/img.mjpegs";
    hash = CryptoHelper::getHashOfString(str, true);
    QCOMPARE(hash, "89968578f7de68eef96b0091f6c478a5");

    hash = CryptoHelper::getHashOfString(str);
    QCOMPARE(hash, "89968578f7de68eef96b0091f6c478a5");

}

QTEST_APPLESS_MAIN(cryptohelpertest)

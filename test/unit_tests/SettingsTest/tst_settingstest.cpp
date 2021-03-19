#include <QtTest>

#include "tst_settingstest.h"

#include "Settings.h"

SettingsTest::SettingsTest()
{

}

SettingsTest::~SettingsTest()
{

}

void SettingsTest::test_case1()
{
    QString str{"http://foo.bar/img.jpg"};

    Settings settings;
    settings.setJid("foo@bar.de");
    QCOMPARE(settings.getJid(), "foo@bar.de");

    settings.setJid("bar@foo.de");
    QCOMPARE(settings.getJid(), "bar@foo.de");

    settings.setPassword("swordfish");
    QCOMPARE(settings.getPassword(), "swordfish");

    settings.setSaveCredentials(true);
    QVERIFY(settings.getSaveCredentials() == true);

    settings.setSaveCredentials(false);
    QVERIFY(settings.getSaveCredentials() == false);

    settings.setDisplayGroupchatNotifications(true);
    QVERIFY(settings.getDisplayGroupchatNotifications() == true);

    settings.setDisplayGroupchatNotifications(false);
    QVERIFY(settings.getDisplayGroupchatNotifications() == false);

    settings.setDisplayChatNotifications(true);
    QVERIFY(settings.getDisplayChatNotifications() == true);

    settings.setDisplayChatNotifications(false);
    QVERIFY(settings.getDisplayChatNotifications() == false);

    // fill and clear force ON list
    QVERIFY(settings.getForceOnNotifications().size() == 0);
    settings.addForceOnNotifications("foo@bar.de");
    QVERIFY(settings.getForceOnNotifications().size() == 1);
    QCOMPARE(settings.getForceOnNotifications().at(0), "foo@bar.de");
    settings.addForceOnNotifications("bar@foo.de");
    QVERIFY(settings.getForceOnNotifications().size() == 2);
    QCOMPARE(settings.getForceOnNotifications().at(1), "bar@foo.de");
    settings.removeForceOnNotifications("foo");
    QVERIFY(settings.getForceOnNotifications().size() == 2);
    settings.removeForceOnNotifications("foo@bar.de");
    QVERIFY(settings.getForceOnNotifications().size() == 1);
    QCOMPARE(settings.getForceOnNotifications().at(0), "bar@foo.de");
    settings.removeForceOnNotifications("bar@foo.de");
    QVERIFY(settings.getForceOnNotifications().size() == 0);

    // fill and clear force OFF list
    QVERIFY(settings.getForceOffNotifications().size() == 0);
    settings.addForceOffNotifications("foo@bar.de");
    QVERIFY(settings.getForceOffNotifications().size() == 1);
    QCOMPARE(settings.getForceOffNotifications().at(0), "foo@bar.de");
    settings.addForceOffNotifications("bar@foo.de");
    QVERIFY(settings.getForceOffNotifications().size() == 2);
    QCOMPARE(settings.getForceOffNotifications().at(1), "bar@foo.de");
    settings.removeForceOffNotifications("foo");
    QVERIFY(settings.getForceOffNotifications().size() == 2);
    settings.removeForceOffNotifications("foo@bar.de");
    QVERIFY(settings.getForceOffNotifications().size() == 1);
    QCOMPARE(settings.getForceOffNotifications().at(0), "bar@foo.de");
    settings.removeForceOffNotifications("bar@foo.de");
    QVERIFY(settings.getForceOffNotifications().size() == 0);

    settings.setSendReadNotifications(true);
    QVERIFY(settings.getSendReadNotifications() == true);

    settings.setSendReadNotifications(false);
    QVERIFY(settings.getSendReadNotifications() == false);

    settings.setImagePaths(QStringList("/foo/bar"));
    QCOMPARE(settings.getImagePaths(), QStringList("/foo/bar"));

    // plain text send list
    QVERIFY(settings.getSendPlainText().size() == 0);
    settings.addForcePlainTextSending("foo@bar.de");
    QVERIFY(settings.getSendPlainText().size() == 1);
    QCOMPARE(settings.getSendPlainText().at(0), "foo@bar.de");
    settings.addForcePlainTextSending("bar@foo.de");
    QVERIFY(settings.getSendPlainText().size() == 2);
    QCOMPARE(settings.getSendPlainText().at(1), "bar@foo.de");
    settings.removeForcePlainTextSending("foo@bar.de");
    QVERIFY(settings.getSendPlainText().size() == 1);
    QCOMPARE(settings.getSendPlainText().at(0), "bar@foo.de");
    settings.removeForcePlainTextSending("bar@foo.de");
    QVERIFY(settings.getSendPlainText().size() == 0);

}

QTEST_APPLESS_MAIN(SettingsTest)

#ifndef MAMTEST_H
#define MAMTEST_H

#include <QObject>

#include "Persistence.h"
#include "Settings.h"
#include "RosterController.h"
#include "LurchAdapter.h"
#include "MessageHandler.h"
#include "ChatMarkers.h"

#include <Swiften/Swiften.h>

class MsgTest : public QObject
{
    Q_OBJECT

public:
    MsgTest();
    ~MsgTest();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testPlain1to1Msg();
    void testPlainRoomMsg();
    void testPlainRoomWithTimestampMsg();
    void testPlainRoomMsgInsideMam();
    void testPlainRoomMsgWithoutIdInsideMam();
    void testDsiplayedMsgInsideMam();

private:
    QString getSerializedStringFromMessage(Swift::Message::ref msg);

    Persistence* persistence_;
    Settings* settings_;
    RosterController* rosterController_;
    LurchAdapter* lurchAdapter_;
    MessageHandler* messageHandler_;
    ChatMarkers* chatMarkers_;

    Swift::Client* client_;
};

#endif

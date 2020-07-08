#ifndef MAMTEST_H
#define MAMTEST_H

#include <QObject>

#include "Persistence.h"
#include "MamManager.h"
#include <Swiften/Swiften.h>

class MamTest : public QObject
{
    Q_OBJECT

public:
    MamTest();
    ~MamTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test1o1MamMsgIncoming();
    void test1o1MamMsgOutgoing();
    void testRoomMamMsgIncoming();
    void testRoomMamMsgOutgoing();

    void testMamIqNotComplete();

    void testMam1o1MsgReceived();
    void testMamGroupMsgReceived();
    void testMamGroupMsgDisplayed();
    void testMam1o1MsgDisplayed();

private:
    Persistence* persistence_;
    MamManager* mamManager_;

    Swift::Client* client_;

};

#endif

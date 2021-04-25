#ifndef MAMTEST_H
#define MAMTEST_H

#include <QObject>

#include <Swiften/Swiften.h>

class LurchAdapterTest : public QObject
{
    Q_OBJECT

public:
    LurchAdapterTest();
    ~LurchAdapterTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test1o1MamMsgIncoming();

private:
    //Swift::Client* client_;


};

#endif

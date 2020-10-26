#pragma once

#include <QObject>

#include "StanzaId.h"
#include <Swiften/Swiften.h>

class StanzaIdTest : public QObject
{
    Q_OBJECT

public:
    StanzaIdTest();
    ~StanzaIdTest();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testParseStanzaId();
    void testSerializeStanzaId();
    void testSetupWithClient();
};

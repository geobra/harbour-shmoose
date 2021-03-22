QT += testlib xml
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

DEFINES += UNIT_TEST

# path to local compiled swift lib
SWIFTPATH = $$_PRO_FILE_PWD_/../../../../swift-4.0.2
contains(DEFINES, TRAVIS) {
    SWIFTPATH = $$_PRO_FILE_PWD_/../../../swift-4.0.2
}
include($$_PRO_FILE_PWD_/../../../swift.pri)

QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-placement-new -Wno-parentheses -Wno-unused-but-set-parameter

INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/xep/stanzaId
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/base

SWIFTPATH = $$_PRO_FILE_PWD_/../../../../swift-4.0.2
include($$_PRO_FILE_PWD_/../../../swift.pri)

TEMPLATE = app

SOURCES +=  tst_stanzaidtest.cpp StanzaIdPayloadParserTester.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/stanzaId/StanzaId.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/stanzaId/StanzaIdPayload.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/stanzaId/StanzaIdPayloadParser.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/stanzaId/StanzaIdPayloadParserFactory.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/stanzaId/StanzaIdPayloadSerializer.cpp

HEADERS += tst_stanzaidtest.h StanzaIdPayloadParserTester.h \
    $$_PRO_FILE_PWD_/../../../source/xep/stanzaId/StanzaId.h \
    $$_PRO_FILE_PWD_/../../../source/xep/stanzaId/StanzaIdPayload.h \
    $$_PRO_FILE_PWD_/../../../source/xep/stanzaId/StanzaIdPayloadParser.h \
    $$_PRO_FILE_PWD_/../../../source/xep/stanzaId/StanzaIdPayloadParserFactory.h \
    $$_PRO_FILE_PWD_/../../../source/xep/stanzaId/StanzaIdPayloadSerializer.h

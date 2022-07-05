QT += testlib xml network

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

QT += qml quick

LIBS += -lgcrypt -lglib-2.0

DEFINES += UNIT_TEST

# path to local compiled swift lib
SWIFTPATH = $$_PRO_FILE_PWD_/../../../../swift-4.0.2
contains(DEFINES, TRAVIS) {
    SWIFTPATH = $$_PRO_FILE_PWD_/../../../swift-4.0.2
}
include($$_PRO_FILE_PWD_/../../../swift.pri)

QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-placement-new -Wno-parentheses -Wno-unused-but-set-parameter

INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/xep/mam
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/xep/httpFileUpload
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/xep/chatMarkers
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/xep/stanzaId
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/xep/omemo
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/base
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/contacts

SWIFTPATH = $$_PRO_FILE_PWD_/../../../../swift-4.0.2
include($$_PRO_FILE_PWD_/../../../swift.pri)

TEMPLATE = app

SOURCES +=  tst_msgtest.cpp \
    $$_PRO_FILE_PWD_/../../../source/base/MessageHandler.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/chatMarkers/ChatMarkers.cpp \
    $$_PRO_FILE_PWD_/../../../source/contacts/RosterController.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/mam/MamManager.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/httpFileUpload/ImageProcessing.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/httpFileUpload/DownloadManager.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/httpFileUpload/FileWithCypher.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/XmppMessageParserClient.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/stanzaId/StanzaIdPayload.cpp \
    $$_PRO_FILE_PWD_/../../../source/base/XmlWriter.cpp \
    $$_PRO_FILE_PWD_/../../../source/base/CryptoHelper.cpp \
    $$_PRO_FILE_PWD_/../../../source/base/System.cpp \
    $$_PRO_FILE_PWD_/../../../source/base/Settings.cpp \
    $$_PRO_FILE_PWD_/../../../source/base/XmlProcessor.cpp \
    $$_PRO_FILE_PWD_/../../../source/contacts/PresenceHandler.cpp \
    $$_PRO_FILE_PWD_/../../../source/contacts/RosterItem.cpp \
    Persistence.cpp

HEADERS += tst_msgtest.h \
    $$_PRO_FILE_PWD_/../../../source/base/MessageHandler.h \
    $$_PRO_FILE_PWD_/../../../source/xep/chatMarkers/ChatMarkers.h \
    $$_PRO_FILE_PWD_/../../../source/contacts/RosterController.h \
    $$_PRO_FILE_PWD_/../../../source/xep/mam/MamManager.h \
    $$_PRO_FILE_PWD_/../../../source/xep/httpFileUpload/ImageProcessing.h \
    $$_PRO_FILE_PWD_/../../../source/xep/httpFileUpload/DownloadManager.h \
    $$_PRO_FILE_PWD_/../../../source/xep/httpFileUpload/FileWithCypher.h \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/XmppMessageParserClient.h \
    $$_PRO_FILE_PWD_/../../../source/xep/stanzaId/StanzaIdPayload.h \
    $$_PRO_FILE_PWD_/../../../source/base/XmlWriter.h \
    $$_PRO_FILE_PWD_/../../../source/base/CryptoHelper.h \
    $$_PRO_FILE_PWD_/../../../source/base/System.h \
    $$_PRO_FILE_PWD_/../../../source/base/Settings.h \
    $$_PRO_FILE_PWD_/../../../source/base/XmlProcessor.h \
    $$_PRO_FILE_PWD_/../../../source/contacts/PresenceHandler.h \
    $$_PRO_FILE_PWD_/../../../source/contacts/RosterItem.h \
    LurchAdapter.h \
    Persistence.h

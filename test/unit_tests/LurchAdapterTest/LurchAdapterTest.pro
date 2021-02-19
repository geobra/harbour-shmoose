QT += testlib xml
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

DEFINES += UNIT_TEST

QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-placement-new -Wno-parentheses -Wno-unused-but-set-parameter

INCLUDEPATH +=  $$_PRO_FILE_PWD_/../../../libomemo/src
INCLUDEPATH +=  $$_PRO_FILE_PWD_/../../../axc/src
INCLUDEPATH +=  $$_PRO_FILE_PWD_/../../../axc/lib/libsignal-protocol-c/src
LIBS +=  $$_PRO_FILE_PWD_/../../../axc/build/libaxc.a
LIBS +=  $$_PRO_FILE_PWD_/../../../axc/lib/libsignal-protocol-c/build/src/libsignal-protocol-c.a
LIBS +=  $$_PRO_FILE_PWD_/../../..//libomemo/build/libomemo-conversations.a
LIBS += -lmxml -lgcrypt -lglib-2.0 -lsqlite3
QMAKE_CXXFLAGS += $$system("pkg-config --cflags glib-2.0")
QMAKE_CFLAGS += $$system("pkg-config --cflags glib-2.0 libxml-2.0")


INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/xep/omemo
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/xep/omemo/lurch
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/xep/omemo/mock
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/base

SWIFTPATH = $$_PRO_FILE_PWD_/../../../../swift-4.0.2
include($$_PRO_FILE_PWD_/../../../swift.pri)

TEMPLATE = app

SOURCES +=  tst_lurchadapter.cpp lurch_wrapper.c \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/LurchAdapter.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/CToCxxProxy.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/xmlnode.c \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/lurch/lurch_util.c \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/mock/purple.c \
    $$_PRO_FILE_PWD_/../../../source/base/System.cpp \
    $$_PRO_FILE_PWD_/../../../source/base/XmlProcessor.cpp

HEADERS += tst_lurchadapter.h StringPayload.h \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/LurchAdapter.h \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/CToCxxProxy.h \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/xmlnode.h \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/lurch_wrapper.h \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/lurch/lurch_util.h \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/mock/purple.h \
    $$_PRO_FILE_PWD_/../../../source/base/System.h \
    $$_PRO_FILE_PWD_/../../../source/base/XmlProcessor.h


QT += testlib xml
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

DEFINES += UNIT_TEST

QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-placement-new -Wno-parentheses -Wno-unused-but-set-parameter

INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/xep/mam
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/base

SWIFTPATH = $$_PRO_FILE_PWD_/../../../../swift-4.0.2
include($$_PRO_FILE_PWD_/../../../swift.pri)

TEMPLATE = app

SOURCES +=  tst_mamtest.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/mam/MamManager.cpp \
    $$_PRO_FILE_PWD_/../../../source/base/XmlWriter.cpp \
    $$_PRO_FILE_PWD_/../../../source/base/XmlProcessor.cpp \
    Persistence.cpp

HEADERS += tst_mamtest.h \
    $$_PRO_FILE_PWD_/../../../source/xep/mam/MamManager.h \
    $$_PRO_FILE_PWD_/../../../source/base/XmlWriter.h \
    $$_PRO_FILE_PWD_/../../../source/base/XmlProcessor.h \
    Persistence.h

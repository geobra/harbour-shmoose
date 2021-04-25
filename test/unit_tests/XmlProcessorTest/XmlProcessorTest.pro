QT += testlib xml
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

DEFINES += UNIT_TEST

QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-placement-new -Wno-parentheses -Wno-unused-but-set-parameter

INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/base

TEMPLATE = app

SOURCES +=  tst_XmlProcessortest.cpp \
    $$_PRO_FILE_PWD_/../../../source/base/XmlProcessor.cpp

HEADERS += tst_XmlProcessortest.h \
    $$_PRO_FILE_PWD_/../../../source/base/XmlProcessor.h

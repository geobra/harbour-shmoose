QT += testlib xml
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

DEFINES += UNIT_TEST

QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-placement-new -Wno-parentheses -Wno-unused-but-set-parameter

QMAKE_CXXFLAGS += $$system("pkg-config --cflags glib-2.0")
QMAKE_CFLAGS += $$system("pkg-config --cflags glib-2.0 libxml-2.0")

INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/xep/omemo
INCLUDEPATH += $$_PRO_FILE_PWD_/../../../source/xep/omemo/mock

LIBS += -lxml2 -lglib-2.0

TEMPLATE = app

SOURCES +=  tst_XmlNodeTest.cpp \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/xmlnode.c \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/mock/purple.c

HEADERS += tst_XmlNodeTest.h \
    $$_PRO_FILE_PWD_/../../../source/xep/omemo/xmlnode.h

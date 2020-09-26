QT += testlib dbus

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += ../common/source

SOURCES += \
        source/RosterTest.cpp \
        ../common/source/ClientComtestCommon.cpp \
        ../common/source/DbusInterfaceWrapper.cpp

HEADERS += \
    source/RosterTest.h \
    ../common/source/ClientComtestCommon.h \
    ../common/source/DbusInterfaceWrapper.h

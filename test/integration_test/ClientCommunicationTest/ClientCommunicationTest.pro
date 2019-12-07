QT -= gui
QT += testlib dbus

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        source/ClientComtest.cpp \
        source/DbusInterfaceWrapper.cpp

HEADERS += \
    source/ClientComtest.h \
    source/DbusInterfaceWrapper.h

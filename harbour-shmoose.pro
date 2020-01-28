TARGET = harbour-shmoose

# path to local compiled swift 3 lib
SWIFT3PATH = ../swift-3.0-host
contains(DEFINES, SFOS) {
    SWIFT3PATH = ../swift-3.0-arm
}

include($$PWD/swift.pri)

TEMPLATE = app
QT += qml quick core sql xml concurrent

contains(DEFINES, DBUS) {
    CONFIG += console
    QT += dbus
}

INCLUDEPATH += source
INCLUDEPATH += source/persistence
INCLUDEPATH += source/xep/httpFileUpload
INCLUDEPATH += source/xep/xmppPing
INCLUDEPATH += source/xep/chatMarkers

! contains(DEFINES, SFOS) {
    QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-placement-new -Wno-parentheses
}

contains(DEFINES, SFOS) {
    LIBS += -liphb
}

QMAKE_CXXFLAGS += -std=c++11

DEFINES += BOOST_SIGNALS_NO_DEPRECATION_WARNING

# testing or production files
contains(DEFINES, DBUS) {
    SOURCES += source/dbus/main.cpp

    SOURCES += source/dbus/DbusCommunicator.cpp
    HEADERS += source/dbus/DbusCommunicator.h
}
else {
    SOURCES += source/main.cpp
}

# on testing, add flags to produce coverage
contains(DEFINES, DBUS) {
	QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
	LIBS += -lgcov
}


SOURCES += \
	source/Shmoose.cpp \
	source/RosterItem.cpp \
        source/ReConnectionHandler.cpp \
	source/persistence/Database.cpp \
	source/persistence/MessageController.cpp \
	source/persistence/SessionController.cpp \
        source/persistence/GcmController.cpp \
	source/persistence/Persistence.cpp \
	source/xep/httpFileUpload/XmlHttpUploadContentHandler.cpp \
	source/xep/httpFileUpload/HttpFileuploader.cpp \
	source/xep/httpFileUpload/HttpFileUploadManager.cpp \
	source/xep/httpFileUpload/DownloadManager.cpp \
	source/FileModel.cpp \
	source/ImageProcessing.cpp \
	source/System.cpp \
        source/xep/xmppPing/XmppPingController.cpp \
        source/IpHeartBeatWatcher.cpp \
        source/MucManager.cpp \
        source/MucCollection.cpp \
        source/xep/chatMarkers/ChatMarkers.cpp \
        source/ConnectionHandler.cpp \
        source/MessageHandler.cpp \
        source/PresenceHandler.cpp \
        source/DiscoInfoHandler.cpp \
        source/Settings.cpp \
        source/XmlProcessor.cpp \
        source/XmlWriter.cpp \
        source/RosterController.cpp

HEADERS += source/Shmoose.h \
	source/RosterItem.h \
        source/ReConnectionHandler.h \
	source/persistence/Database.h \
	source/persistence/MessageController.h \
	source/persistence/SessionController.h \
        source/persistence/GcmController.h \
	source/persistence/Persistence.h \
	source/xep/httpFileUpload/XmlHttpUploadContentHandler.h \
	source/xep/httpFileUpload/HttpFileuploader.h \
	source/xep/httpFileUpload/HttpFileUploadManager.h \
	source/xep/httpFileUpload/DownloadManager.h \
	source/xep/xmppPing/PingRequest.h \
	source/FileModel.h \
	source/ImageProcessing.h \
	source/System.h \
        source/xep/xmppPing/XmppPingController.h \
        source/IpHeartBeatWatcher.h \
        source/MucManager.h \
        source/MucCollection.h \
        source/xep/chatMarkers/ChatMarkers.h \
        source/ConnectionHandler.h \
        source/MessageHandler.h \
        source/PresenceHandler.h \
        source/DiscoInfoHandler.h \
        source/Settings.h \
        source/XmlProcessor.h \
        source/XmlWriter.h \
        source/RosterController.h

lupdate_only {
        SOURCES += resources/qml/*.qml \
           resources/qml/cover/*.qml \
           resources/qml/pages/*.qml
}

TRANSLATIONS = resources/translations/de_DE.ts \
               resources/translations/en_GB.ts \
               resources/translations/es_BO.ts \
               resources/translations/nl.ts \
               resources/translations/sv_SE.ts \
               resources/translations/zh_CN.ts \
               resources/translations/nb_NO.ts \
               resources/translations/hr_HR.ts

RESOURCES += shmoose.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =



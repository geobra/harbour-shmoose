TARGET = harbour-shmoose

# path to local compiled swift lib
SWIFTPATH = $$_PRO_FILE_PWD_/../swift-4.0.2
contains(DEFINES, SFOS) {
    SWIFTPATH = $$_PRO_FILE_PWD_/../swift-4.0.2-arm
}

contains(DEFINES, TRAVIS) {
    SWIFTPATH = $$_PRO_FILE_PWD_/swift-4.0.2
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
INCLUDEPATH += source/xep/mam
INCLUDEPATH += source/xep/xmppPing
INCLUDEPATH += source/xep/chatMarkers
INCLUDEPATH += source/xep/stanzaId
INCLUDEPATH += source/xep/omemo
INCLUDEPATH += source/xep/omemo/payload
INCLUDEPATH += source/xep/omemo/lurch
INCLUDEPATH += source/xep/omemo/mock
INCLUDEPATH += source/room
INCLUDEPATH += source/networkconnection
INCLUDEPATH += source/contacts
INCLUDEPATH += source/base

! contains(DEFINES, SFOS) {
    QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-placement-new -Wno-parentheses -Wno-unused-but-set-parameter
}

contains(DEFINES, QMLLIVE_SOURCE) {
    QMLLIVEPROJECT = $$_PRO_FILE_PWD_/../qmllive
    include($${QMLLIVEPROJECT}/src/src.pri)
}

contains(DEFINES, SFOS) {
    LIBS += -liphb
}

# omemo
INCLUDEPATH += libomemo/src
INCLUDEPATH += axc/src
INCLUDEPATH += axc/lib/libsignal-protocol-c/src
LIBS += $$_PRO_FILE_PWD_/axc/build/libaxc.a
LIBS += $$_PRO_FILE_PWD_/axc/lib/libsignal-protocol-c/build/src/libsignal-protocol-c.a
LIBS += $$_PRO_FILE_PWD_/libomemo/build/libomemo-conversations.a
LIBS += -lmxml -lgcrypt -lglib-2.0 -lsqlite3
QMAKE_CXXFLAGS += $$system("pkg-config --cflags glib-2.0")
QMAKE_CFLAGS += $$system("pkg-config --cflags glib-2.0 libxml-2.0")

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
    QMAKE_CFLAGS += -fprofile-arcs -ftest-coverage
    LIBS += -lgcov
}

SOURCES += \
    source/base/Shmoose.cpp \
    source/base/Settings.cpp \
    source/base/MessageHandler.cpp \
    source/base/DiscoInfoHandler.cpp \
    source/base/FileModel.cpp \
    source/base/XmlWriter.cpp \
    source/base/System.cpp \
    source/base/XmlProcessor.cpp \
    source/persistence/Database.cpp \
    source/persistence/MessageController.cpp \
    source/persistence/SessionController.cpp \
    source/persistence/GcmController.cpp \
    source/persistence/Persistence.cpp \
    source/xep/httpFileUpload/XmlHttpUploadContentHandler.cpp \
    source/xep/httpFileUpload/HttpFileuploader.cpp \
    source/xep/httpFileUpload/HttpFileUploadManager.cpp \
    source/xep/httpFileUpload/DownloadManager.cpp \
    source/xep/httpFileUpload/ImageProcessing.cpp \
    source/xep/httpFileUpload/FileWithCypher.cpp \
    source/xep/mam/MamManager.cpp \
    source/xep/xmppPing/XmppPingController.cpp \
    source/xep/chatMarkers/ChatMarkers.cpp \
    source/xep/stanzaId/StanzaId.cpp \
    source/xep/stanzaId/StanzaIdPayload.cpp \
    source/xep/stanzaId/StanzaIdPayloadParser.cpp \
    source/xep/stanzaId/StanzaIdPayloadParserFactory.cpp \
    source/xep/stanzaId/StanzaIdPayloadSerializer.cpp \
    source/xep/omemo/LurchAdapter.cpp \
    source/xep/omemo/payload/ItemsPayload.cpp \
    source/xep/omemo/mock/purple.c \
    source/xep/omemo/mock/jabber.c \
    source/xep/omemo/lurch/lurch_util.c \
    source/xep/omemo/lurch_wrapper.c \
    source/xep/omemo/xmlnode.c \
    source/xep/omemo/XmppMessageParserClient.cpp \
    source/room/MucManager.cpp \
    source/room/MucCollection.cpp \
    source/networkconnection/ConnectionHandler.cpp \
    source/networkconnection/IpHeartBeatWatcher.cpp \
    source/networkconnection/ReConnectionHandler.cpp \
    source/contacts/PresenceHandler.cpp \
    source/contacts/RosterItem.cpp \
    source/contacts/RosterController.cpp \
    source/base/CryptoHelper.cpp \
    source/xep/omemo/CToCxxProxy.cpp \
    source/xep/omemo/payload/ItemsPayloadParser.cpp \
    source/xep/omemo/payload/ItemsPayloadParserFactory.cpp \
    source/xep/omemo/payload/ItemsPayloadSerializer.cpp \
    source/xep/omemo/payload/EncryptionPayload.cpp \
    source/xep/omemo/payload/EncryptionPayloadParser.cpp \
    source/xep/omemo/payload/EncryptionPayloadParserFactory.cpp \
    source/xep/omemo/payload/EncryptionPayloadSerializer.cpp

HEADERS += source/base/Shmoose.h \
    source/base/Settings.h \
    source/base/MessageHandler.h \
    source/base/DiscoInfoHandler.h \
    source/base/FileModel.h \
    source/base/XmlWriter.h \
    source/base/System.h \
    source/base/XmlProcessor.h \
    source/persistence/Database.h \
    source/persistence/MessageController.h \
    source/persistence/SessionController.h \
    source/persistence/GcmController.h \
    source/persistence/Persistence.h \
    source/xep/httpFileUpload/XmlHttpUploadContentHandler.h \
    source/xep/httpFileUpload/HttpFileuploader.h \
    source/xep/httpFileUpload/HttpFileUploadManager.h \
    source/xep/httpFileUpload/DownloadManager.h \
    source/xep/httpFileUpload/ImageProcessing.h \
    source/xep/httpFileUpload/FileWithCypher.h \
    source/xep/mam/MamManager.h \
    source/xep/omemo/BundleDeviceListRequest.h \
    source/xep/xmppPing/PingRequest.h \
    source/xep/xmppPing/XmppPingController.h \
    source/xep/chatMarkers/ChatMarkers.h \
    source/xep/stanzaId/StanzaId.h \
    source/xep/stanzaId/StanzaIdPayload.h \
    source/xep/stanzaId/StanzaIdPayloadParser.h \
    source/xep/stanzaId/StanzaIdPayloadParserFactory.h \
    source/xep/stanzaId/StanzaIdPayloadSerializer.h \
    source/xep/omemo/LurchAdapter.h \
    source/xep/omemo/payload/ItemsPayload.h \
    source/xep/omemo/mock/purple.h \
    source/xep/omemo/mock/jabber.h \
    source/xep/omemo/mock/chat.h \
    source/xep/omemo/mock/jutil.h \
    source/xep/omemo/mock/pep.h \
    source/xep/omemo/lurch/lurch_util.h \
    source/xep/omemo/lurch_wrapper.h \
    source/xep/omemo/XmppMessageParserClient.h \
    source/xep/omemo/RawRequestWithFromJid.h \
    source/xep/omemo/RawRequestBundle.h \
    source/xep/omemo/xmlnode.h \
    source/room/MucManager.h \
    source/room/MucCollection.h \
    source/networkconnection/ConnectionHandler.h \
    source/networkconnection/IpHeartBeatWatcher.h \
    source/networkconnection/ReConnectionHandler.h \
    source/contacts/PresenceHandler.h \
    source/contacts/RosterItem.h \
    source/contacts/RosterController.h \
    source/base/CryptoHelper.h \
    source/xep/omemo/CToCxxProxy.h \
    source/xep/omemo/payload/ItemsPayloadParser.h \
    source/xep/omemo/payload/ItemsPayloadParserFactory.h \
    source/xep/omemo/payload/ItemsPayloadSerializer.h \
    source/xep/omemo/payload/EncryptionPayload.h \
    source/xep/omemo/payload/EncryptionPayloadParser.h \
    source/xep/omemo/payload/EncryptionPayloadParserFactory.h \
    source/xep/omemo/payload/EncryptionPayloadSerializer.h

lupdate_only {
        SOURCES += resources/qml/*.qml \
           resources/qml/cover/*.qml \
           resources/qml/pages/*.qml
}

TRANSLATIONS = resources/translations/de_DE.ts \
               resources/translations/en_GB.ts \
               resources/translations/es_BO.ts \
               resources/translations/fr_FR.ts \
               resources/translations/hr-HR.ts \
               resources/translations/nb_NO.ts \
               resources/translations/nl.ts \
               resources/translations/sv_SE.ts \
               resources/translations/zh_CN.ts

RESOURCES += shmoose.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =



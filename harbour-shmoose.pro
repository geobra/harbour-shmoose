TARGET = harbour-shmoose

# path to local compiled swift 3 lib
SWIFT3PATH = ../swift-3.0-host
contains(DEFINES, SFOS) {
    SWIFT3PATH = ../swift-3.0-arm
}

# from swift-config
SWIFTCXX = -DSWIFTEN_STATIC -DBOOST_ALL_NO_LIB -DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_SIGNALS_NO_DEPRECATION_WARNING -DSWIFT_EXPERIMENTAL_FT
SWIFTLIB = -lSwiften -lSwiften_Boost -lrt -lz -lssl -lcrypto -lxml2 -lresolv -lpthread -ldl -lm -lc -lstdc++

TEMPLATE = app
QT += qml quick core sql xml concurrent

INCLUDEPATH += $${SWIFT3PATH}/3rdParty/Boost/src
INCLUDEPATH += $${SWIFT3PATH}/
INCLUDEPATH += source
INCLUDEPATH += source/persistence
INCLUDEPATH += source/xep/httpFileUpload
INCLUDEPATH += source/xep/xmppPing
INCLUDEPATH += source/xep/chatMarkers

QMAKE_CXXFLAGS += $${SWIFTCXX} -std=c++11
linux-g++ {
    QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-placement-new
}
LIBS += -L$${SWIFT3PATH}/Swiften -L$${SWIFT3PATH}/3rdParty/Boost $${SWIFTLIB}

contains(DEFINES, SFOS) {
    LIBS += -liphb
}

DEFINES += BOOST_SIGNALS_NO_DEPRECATION_WARNING

SOURCES += source/main.cpp \
	source/Shmoose.cpp \
	source/RosterContoller.cpp \
	source/RosterItem.cpp \
        source/ReConnectionHandler.cpp \
	source/persistence/Database.cpp \
	source/persistence/MessageController.cpp \
	source/persistence/SessionController.cpp \
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
    source/Settings.cpp

HEADERS += source/Shmoose.h \
	source/RosterContoller.h \
	source/RosterItem.h \
        source/ReConnectionHandler.h \
	source/persistence/Database.h \
	source/persistence/MessageController.h \
	source/persistence/SessionController.h \
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
    source/Settings.h

lupdate_only {
        SOURCES += resources/qml/*.qml \
           resources/qml/cover/*.qml \
           resources/qml/pages/*.qml
}

TRANSLATIONS = resources/translations/de_DE.ts \
               resources/translations/en_GB.ts \
               resources/translations/es_BO.ts

RESOURCES += shmoose.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =



# path to local compiled swift 3 lib
SWIFT3PATH = ../swift-3.0
# from swift-config
SWIFTCXX = -DSWIFTEN_STATIC -DBOOST_ALL_NO_LIB -DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_SIGNALS_NO_DEPRECATION_WARNING -DSWIFT_EXPERIMENTAL_FT
SWIFTLIB = -lSwiften -lSwiften_Boost -lrt -lz -lssl -lcrypto -lxml2 -lresolv -lpthread -ldl -lm -lc -lstdc++


TEMPLATE = app
QT += qml quick core sql xml


INCLUDEPATH += $${SWIFT3PATH}/3rdParty/Boost/src
INCLUDEPATH += $${SWIFT3PATH}/
INCLUDEPATH += source
INCLUDEPATH += source/persistence
INCLUDEPATH += source/xep/httpFileUpload

QMAKE_CXXFLAGS += $${SWIFTCXX} -std=c++11
linux-g++ {
    QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-placement-new
}
LIBS += -L$${SWIFT3PATH}/Swiften -L$${SWIFT3PATH}/3rdParty/Boost $${SWIFTLIB}

DEFINES += BOOST_SIGNALS_NO_DEPRECATION_WARNING

SOURCES += source/main.cpp \
	source/Shmoose.cpp \
	source/RosterContoller.cpp \
	source/RosterItem.cpp \
	source/persistence/Database.cpp \
	source/persistence/MessageController.cpp \
	source/persistence/SessionController.cpp \
	source/persistence/Persistence.cpp \
        source/xep/httpFileUpload/XmlHttpUploadContentHandler.cpp \
        source/xep/httpFileUpload/HttpFileuploader.cpp \
        source/xep/httpFileUpload/HttpFileUploadManager.cpp \
        source/FileModel.cpp \
        source/ImageProcessing.cpp \
    source/System.cpp

HEADERS += source/Shmoose.h \
	source/EchoPayload.h \
	source/EchoPayloadParserFactory.h \
	source/EchoPayloadSerializer.h \
	source/RosterContoller.h \
	source/RosterItem.h \
	source/persistence/Database.h \
	source/persistence/MessageController.h \
	source/persistence/SessionController.h \
	source/persistence/Persistence.h \
        source/xep/httpFileUpload/XmlHttpUploadContentHandler.h \
        source/xep/httpFileUpload/HttpFileuploader.h \
        source/xep/httpFileUpload/HttpFileUploadManager.h \
        source/FileModel.h \
        source/ImageProcessing.h \
    source/System.h

RESOURCES += shmoose.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =



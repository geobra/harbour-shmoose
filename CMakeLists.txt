cmake_minimum_required (VERSION 3.13)

project (harbour-shmoose)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable (harbour-shmoose shmoose.qrc)

set(CMAKE_BUILD_TYPE Debug)
set_property(TARGET harbour-shmoose PROPERTY CXX_STANDARD 14)

target_sources (
    harbour-shmoose PRIVATE
    source/main.cpp
    source/base/Shmoose.cpp
    source/base/Settings.cpp
    source/base/MessageHandler.cpp
    source/base/DiscoInfoHandler.cpp
    source/base/FileModel.cpp
    source/base/XmlWriter.cpp
    source/base/System.cpp
    source/base/XmlProcessor.cpp
    source/persistence/Database.cpp
    source/persistence/MessageController.cpp
    source/persistence/SessionController.cpp
    source/persistence/GcmController.cpp
    source/persistence/Persistence.cpp
    source/xep/httpFileUpload/XmlHttpUploadContentHandler.cpp
    source/xep/httpFileUpload/HttpFileuploader.cpp
    source/xep/httpFileUpload/HttpFileUploadManager.cpp
    source/xep/httpFileUpload/DownloadManager.cpp
    source/xep/httpFileUpload/ImageProcessing.cpp
    source/xep/httpFileUpload/FileWithCypher.cpp
    source/xep/mam/MamManager.cpp
    source/xep/xmppPing/XmppPingController.cpp
    source/xep/chatMarkers/ChatMarkers.cpp
    source/xep/stanzaId/StanzaId.cpp
    source/xep/stanzaId/StanzaIdPayload.cpp
    source/xep/stanzaId/StanzaIdPayloadParser.cpp
    source/xep/stanzaId/StanzaIdPayloadParserFactory.cpp
    source/xep/stanzaId/StanzaIdPayloadSerializer.cpp
    source/xep/omemo/LurchAdapter.cpp
    source/xep/omemo/payload/ItemsPayload.cpp
    source/xep/omemo/mock/purple.c
    source/xep/omemo/mock/jabber.c
    source/xep/omemo/lurch/lurch_util.c
    source/xep/omemo/lurch_wrapper.c
    source/xep/omemo/xmlnode.c
    source/xep/omemo/XmppMessageParserClient.cpp
    source/room/MucManager.cpp
    source/room/MucCollection.cpp
    source/networkconnection/ConnectionHandler.cpp
    source/networkconnection/IpHeartBeatWatcher.cpp
    source/networkconnection/ReConnectionHandler.cpp
    source/contacts/PresenceHandler.cpp
    source/contacts/RosterItem.cpp
    source/contacts/RosterController.cpp
    source/base/CryptoHelper.cpp
    source/xep/omemo/CToCxxProxy.cpp
    source/xep/omemo/payload/ItemsPayloadParser.cpp
    source/xep/omemo/payload/ItemsPayloadParserFactory.cpp
    source/xep/omemo/payload/ItemsPayloadSerializer.cpp
    source/xep/omemo/payload/EncryptionPayload.cpp
    source/xep/omemo/payload/EncryptionPayloadParser.cpp
    source/xep/omemo/payload/EncryptionPayloadParserFactory.cpp
    source/xep/omemo/payload/EncryptionPayloadSerializer.cpp
)

set(AXCPATH ${CMAKE_CURRENT_SOURCE_DIR}/axc_x86)
set(LIBOMEMOPATH ${CMAKE_CURRENT_SOURCE_DIR}/libomemo_x86)


# Qt and all needed modules
find_package(Qt5Core REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Xml REQUIRED)
target_link_libraries(harbour-shmoose Qt5::Core Qt5::Widgets Qt5::Sql Qt5::Quick Qt5::Multimedia Qt5::Qml Qt5::Concurrent Qt5::Xml)

# libs which needs pkgconfig
find_package (PkgConfig REQUIRED)

# glib-2
pkg_check_modules (GLIB2 REQUIRED glib-2.0)
include_directories (${GLIB2_INCLUDE_DIRS})
link_directories (${GLIB2_LIBRARY_DIRS})

# libxml-2
pkg_check_modules (LIBXML2 REQUIRED libxml-2.0)
include_directories (${LIBXML2_INCLUDE_DIRS})
link_directories (${LIBXML2_LIBRARY_DIRS})

# mxml
pkg_check_modules (MXML REQUIRED mxml)
include_directories (${MXML_INCLUDE_DIRS})
link_directories (${MXML_LIBRARY_DIRS})

# sqlite3
pkg_check_modules (SQLITE3 REQUIRED sqlite3)
include_directories (${SQLITE3_INCLUDE_DIRS})
link_directories (${SQLITE3_LIBRARY_DIRS})


# swiften cflags and libs
set(SWIFTPATH ${CMAKE_CURRENT_SOURCE_DIR}/../swift-4.0.2)
set(SWIFTCFG "${SWIFTPATH}/Swiften/Config/swiften-config")

execute_process(COMMAND ${SWIFTCFG} --cflags WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} RESULT_VARIABLE result OUTPUT_VARIABLE output)
string(REGEX REPLACE "\n$" "" output "${output}")
#message(STATUS "swifen config: ${result}, ${output}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${output} ")

execute_process(COMMAND ${SWIFTCFG} --libs WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} RESULT_VARIABLE result OUTPUT_VARIABLE output)
string(REGEX REPLACE "[ ]*\n$" "" output "${output}")
target_link_libraries(harbour-shmoose ${output})

# mark swiften include dir as external lib to prevent compiler warnings from there
target_include_directories(harbour-shmoose SYSTEM PUBLIC ${SWIFTPATH})
target_include_directories(harbour-shmoose SYSTEM PUBLIC ${SWIFTPATH}/3rdParty/Boost/src)


include_directories (
	source/persistence
	source/xep/httpFileUpload
	source/xep/mam
	source/xep/xmppPing
	source/xep/chatMarkers
	source/xep/stanzaId
	source/xep/omemo
	source/xep/omemo/payload
	source/xep/omemo/lurch
	source/xep/omemo/mock
	source/room
	source/networkconnection
	source/contacts
	source/base
	${LIBOMEMOPATH}/src
	${AXCPATH}/src
	${AXCPATH}/lib/libsignal-protocol-c/src/
)

target_link_libraries(
	harbour-shmoose
	${AXCPATH}/build/libaxc.a
	${AXCPATH}/lib/libsignal-protocol-c/build/src/libsignal-protocol-c.a
	${LIBOMEMOPATH}/build/libomemo-conversations.a
	${GLIB2_LIBRARIES}
	${LIBXML2_LIBRARIES}
	${MXML_LIBRARIES}
	${GCRYPT_LIBRARIES}
	${SQLITE3_LIBRARIES}
	-lgcrypt # no pkg-config
)


# from swift-config
SWIFTCXX = -DSWIFTEN_STATIC -DBOOST_ALL_NO_LIB -DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_SIGNALS_NO_DEPRECATION_WARNING -DSWIFT_EXPERIMENTAL_FT
SWIFTLIB = -lSwiften -lrt -lz -lssl -lcrypto -lxml2 -lresolv -lpthread -ldl -lm -lc -lstdc++

contains(DEFINES, TRAVIS) {
SWIFTLIB += -lboost_system -lboost_signals
}

! contains(DEFINES, TRAVIS) {
SWIFTLIB += -lSwiften_Boost
}

INCLUDEPATH += $${SWIFT3PATH}/3rdParty/Boost/src
INCLUDEPATH += $${SWIFT3PATH}/

LIBS += -L$${SWIFT3PATH}/Swiften -L$${SWIFT3PATH}/3rdParty/Boost $${SWIFTLIB}

QMAKE_CXXFLAGS += $${SWIFTCXX}

# from swift-config
SWIFTCXX = -DSWIFTEN_STATIC -DBOOST_ALL_NO_LIB -DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_SIGNALS_NO_DEPRECATION_WARNING -DSWIFT_EXPERIMENTAL_FT
SWIFTLIB = -lSwiften -lrt -lz -lssl -lcrypto -lxml2 -lresolv -lpthread -ldl -lm -lc -lstdc++ -lSwiften_Boost

INCLUDEPATH += $${SWIFTPATH}/3rdParty/Boost/src
INCLUDEPATH += $${SWIFTPATH}/

LIBS += -L$${SWIFTPATH}/Swiften -L$${SWIFTPATH}/3rdParty/Boost $${SWIFTLIB}

QMAKE_CXXFLAGS += $${SWIFTCXX}

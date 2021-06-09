SWIFTCXX = $$system("$${SWIFTPATH}/Swiften/Config/swiften-config --cflags")
SWIFTLIB = $$system("$${SWIFTPATH}/Swiften/Config/swiften-config --libs")

LIBS += $${SWIFTLIB}
QMAKE_CXXFLAGS += $${SWIFTCXX}

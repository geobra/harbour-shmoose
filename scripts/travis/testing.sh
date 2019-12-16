#! /bin/bash

# a dbus session is needed
DBA=$(dbus-daemon --print-address --session --fork)
export DBUS_SESSION_BUS_ADDRESS=$DBA
echo "use $DBUS_SESSION_BUS_ADDRESS as dbus address"

# build for testing
mkdir testing
cd testing
qmake .. DEFINES+=TRAVIS DEFINES+=dbus
make

# run the test clients
xvfb-run -a -e /dev/stdout ./harbour-shmoose lhs &
xvfb-run -a -e /dev/stdout ./harbour-shmoose rhs &
cd ..

# build the test
cd test/integration_test/ClientCommunicationTest/
mkdir build
cd build
qmake ..
make
xvfb-run -a -e /dev/stdout ./ClientCommunicationTest
cd ../../..


#! /bin/bash

# a dbus session is needed
if test -z "$DBUS_SESSION_BUS_ADDRESS" ; then
    ## if not found, launch a new one
    eval 'dbus-launch --sh-syntax --exit-with-session'
    echo "D-Bus per-session daemon address is: $DBUS_SESSION_BUS_ADDRESS"
fi

# build for testing
mkdir testing
cd testing
qmake .. DEFINES+=TRAVIS DEFINES+=dbus
make

# run the test clients
./harbour-shmoose lhs &
./harbour-shmoose rhs &
cd ..

# build the test
cd test/integration_test/ClientCommunicationTest/
mkdir build
cd build
qmake ..
make
./ClientCommunicationTest
cd ../../..


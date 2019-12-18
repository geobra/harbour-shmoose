#! /bin/bash

# a dbus session is needed
if test -z "$DBUS_SESSION_BUS_ADDRESS" ; then
	## if not found, launch a new one
	eval `dbus-launch --sh-syntax`
	echo "D-Bus per-session daemon address is: $DBUS_SESSION_BUS_ADDRESS"
fi

echo "use $DBUS_SESSION_BUS_ADDRESS as dbus address"
echo "avilable session busses:"
ls -l ~/.dbus/session-bus/  
echo "done"

# build for testing
mkdir testing
cd testing
qmake .. DEFINES+=TRAVIS DEFINES+=DBUS
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


#! /bin/bash

TESTPATH="testing"
COVFILE="coverage.info"

# a dbus session is needed
if test -z "$DBUS_SESSION_BUS_ADDRESS" ; then
	## if not found, launch a new one
	eval `dbus-launch --sh-syntax`
	echo "D-Bus per-session daemon address is: $DBUS_SESSION_BUS_ADDRESS"
fi

echo "use $DBUS_SESSION_BUS_ADDRESS as dbus address"

# build for testing
mkdir $TESTPATH
cd $TESTPATH
qmake .. DEFINES+=TRAVIS DEFINES+=DBUS
make

# run the test clients for plain 1to1 msg tests
xvfb-run -a -e /dev/stdout ./harbour-shmoose lhs &
xvfb-run -a -e /dev/stdout ./harbour-shmoose rhs &
cd ..

# build the plain 1to1 msg test
cd test/integration_test/ClientCommunicationTest/
mkdir build
cd build
qmake ..
make
xvfb-run -a -e /dev/stdout ./ClientCommunicationTest
cd ../../../..


# run the test clients for the room msg tests
xvfb-run -a -e /dev/stdout ./$TESTPATH/harbour-shmoose lhs &
xvfb-run -a -e /dev/stdout ./$TESTPATH/harbour-shmoose mhs &
xvfb-run -a -e /dev/stdout ./$TESTPATH/harbour-shmoose rhs &

# build the plain room msg test
cd test/integration_test/ClientRoomMessagingTest/
mkdir build
cd build
qmake ..
make
xvfb-run -a -e /dev/stdout ./ClientRoomMessagingTest
cd ../../../..

# collect the coverage info
lcov --capture --directory $TESTPATH --output-file $COVFILE
# remove system files from /usr and generated moc files
lcov --remove $COVFILE '/usr/*' --output-file $COVFILE
lcov --remove $COVFILE '*/test/moc_*' --output-file $COVFILE

# Uploading report to CodeCov
bash <(curl -s https://codecov.io/bash) -f $COVFILE || echo "failed upload to Codecov"


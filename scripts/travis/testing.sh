#! /bin/bash
set -x

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

# build and run the roster test
${TRAVIS_BUILD_DIR}/scripts/travis/reset_ejabberd.sh
xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose lhs &
xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose mhs &
xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose rhs &

cd ${TRAVIS_BUILD_DIR}/test/integration_test/RosterTest/
mkdir build
cd build
qmake ..
make
xvfb-run -a -e /dev/stdout ./RosterTest


# build and run the plain 1to1 msg test
killall -9 harbour-shmoose
${TRAVIS_BUILD_DIR}/scripts/travis/reset_ejabberd.sh
xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose lhs &
xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose rhs &

cd ${TRAVIS_BUILD_DIR}/test/integration_test/ClientCommunicationTest/
mkdir build
cd build
qmake ..
make
xvfb-run -a -e /dev/stdout ./ClientCommunicationTest


# build the plain room msg test
killall -9 harbour-shmoose
${TRAVIS_BUILD_DIR}/scripts/travis/reset_ejabberd.sh

xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose lhs &
xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose mhs &
xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose rhs &

${TRAVIS_BUILD_DIR}/scripts/travis/reset_ejabberd.sh
cd ${TRAVIS_BUILD_DIR}/test/integration_test/ClientRoomMessagingTest/
mkdir build
cd build
qmake ..
make
xvfb-run -a -e /dev/stdout ./ClientRoomMessagingTest

# collect the coverage info
lcov --capture --directory ${TRAVIS_BUILD_DIR}/$TESTPATH --output-file ${TRAVIS_BUILD_DIR}/$COVFILE
# remove system files from /usr and generated moc files
lcov --remove ${TRAVIS_BUILD_DIR}/$COVFILE '/usr/*' --output-file ${TRAVIS_BUILD_DIR}/$COVFILE
lcov --remove ${TRAVIS_BUILD_DIR}/$COVFILE '*/test/moc_*' --output-file ${TRAVIS_BUILD_DIR}/$COVFILE

# Uploading report to CodeCov
bash <(curl -s https://codecov.io/bash) -f ${TRAVIS_BUILD_DIR}/$COVFILE || echo "failed upload to Codecov"


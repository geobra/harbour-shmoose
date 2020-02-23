#! /bin/bash

TESTPATH="testing"
COVFILE="coverage.info"
RESULTS="results"

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


GCOVRESULTSCOMMON="gcovresults"
RESULTSC1=${TRAVIS_BUILD_DIR}/${RESULTS}/${GCOVRESULTSCOMMON}/c1/
RESULTSC2=${TRAVIS_BUILD_DIR}/${RESULTS}/${GCOVRESULTSCOMMON}/c2/
RESULTSC3=${TRAVIS_BUILD_DIR}/${RESULTS}/${GCOVRESULTSCOMMON}/c3/

mkdir -p $RESULTSC1
mkdir -p $RESULTSC2
mkdir -p $RESULTSC3

BUILDTEST_PATH_DEPTH=$(echo "${TRAVIS_BUILD_DIR}/${TESTPATH}" | grep -o / | wc -l)
echo "BPD ####### $BUILDTEST_PATH_DEPTH #############"

# build and run the roster test
${TRAVIS_BUILD_DIR}/scripts/travis/reset_ejabberd.sh
export GCOV_PREFIX_STRIP=$BUILDTEST_PATH_DEPTH
GCOV_PREFIX=$RESULTSC1 xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose lhs &
GCOV_PREFIX=$RESULTSC2 xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose mhs &
GCOV_PREFIX=$RESULTSC3 xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose rhs &

cd ${TRAVIS_BUILD_DIR}/test/integration_test/RosterTest/
mkdir build && cd build
qmake .. && make
xvfb-run -a -e /dev/stdout ./RosterTest

# cp trace data to source dir; run lcov and generate test.cov
cp $RESULTSC1/*.gcda ${TRAVIS_BUILD_DIR}/${TESTPATH}/
lcov --capture --directory ${TRAVIS_BUILD_DIR}/$TESTPATH --output-file ${TRAVIS_BUILD_DIR}/t1c1.cov
rm -f ${TRAVIS_BUILD_DIR}/$TESTPATH/*.gcda

cp $RESULTSC2/*.gcda ${TRAVIS_BUILD_DIR}/${TESTPATH}/
lcov --capture --directory ${TRAVIS_BUILD_DIR}/$TESTPATH --output-file ${TRAVIS_BUILD_DIR}/t1c2.cov
rm -f ${TRAVIS_BUILD_DIR}/$TESTPATH/*.gcda

cp $RESULTSC2/*.gcda ${TRAVIS_BUILD_DIR}/${TESTPATH}/
lcov --capture --directory ${TRAVIS_BUILD_DIR}/$TESTPATH --output-file ${TRAVIS_BUILD_DIR}/t1c3.cov
rm -f ${TRAVIS_BUILD_DIR}/$TESTPATH/*.gcda

lcov -a ${TRAVIS_BUILD_DIR}/t1c1.cov -a ${TRAVIS_BUILD_DIR}/t1c2.cov -a ${TRAVIS_BUILD_DIR}/t1c3.cov -o ${TRAVIS_BUILD_DIR}/${COVFILE}

# build and run the plain 1to1 msg test
#killall -9 harbour-shmoose
#${TRAVIS_BUILD_DIR}/scripts/travis/reset_ejabberd.sh
#xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose lhs &
#xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose rhs &

#cd ${TRAVIS_BUILD_DIR}/test/integration_test/ClientCommunicationTest/
#mkdir build
#cd build
#qmake ..
#make
#xvfb-run -a -e /dev/stdout ./ClientCommunicationTest


# build the plain room msg test
#killall -9 harbour-shmoose
#${TRAVIS_BUILD_DIR}/scripts/travis/reset_ejabberd.sh

#xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose lhs &
#xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose mhs &
#xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose rhs &

#${TRAVIS_BUILD_DIR}/scripts/travis/reset_ejabberd.sh
#cd ${TRAVIS_BUILD_DIR}/test/integration_test/ClientRoomMessagingTest/
#mkdir build
#cd build
#qmake ..
#make
#xvfb-run -a -e /dev/stdout ./ClientRoomMessagingTest

# merge tracefiles with -a t1 -a t2 -a t3 -o final.cov

# remove system files from /usr and generated moc files
lcov --remove ${TRAVIS_BUILD_DIR}/$COVFILE '/usr/*' --output-file ${TRAVIS_BUILD_DIR}/$COVFILE
lcov --remove ${TRAVIS_BUILD_DIR}/$COVFILE '*/test/moc_*' --output-file ${TRAVIS_BUILD_DIR}/$COVFILE

# Uploading report to CodeCov
bash <(curl -s https://codecov.io/bash) -f ${TRAVIS_BUILD_DIR}/$COVFILE || echo "failed upload to Codecov"


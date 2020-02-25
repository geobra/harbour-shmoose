#! /bin/bash

TESTPATH="testing"
COVFILE="coverage.info"
RESULTS="results"

GCOVRESULTSCOMMON="gcovresults"
RESULTSC1=${TRAVIS_BUILD_DIR}/${RESULTS}/${GCOVRESULTSCOMMON}/c1/
RESULTSC2=${TRAVIS_BUILD_DIR}/${RESULTS}/${GCOVRESULTSCOMMON}/c2/
RESULTSC3=${TRAVIS_BUILD_DIR}/${RESULTS}/${GCOVRESULTSCOMMON}/c3/

mkdir -p $RESULTSC1
mkdir -p $RESULTSC2
mkdir -p $RESULTSC3

BUILDTEST_PATH_DEPTH=$(echo "${TRAVIS_BUILD_DIR}/${TESTPATH}" | grep -o / | wc -l)

collect_coverage_at_path_to_file()
{
	RPATH=$1
	CFILE=$2
	if [ "$(ls -A $RPATH)" ]; then
		cp $RPATH/*.gcda ${TRAVIS_BUILD_DIR}/${TESTPATH}/
		lcov --capture --directory ${TRAVIS_BUILD_DIR}/$TESTPATH --output-file ${TRAVIS_BUILD_DIR}/${RESULTS}/$CFILE
		rm -f ${TRAVIS_BUILD_DIR}/$TESTPATH/*.gcda
	fi

	rm -f $RPATH/*.gcda
}

merge_client_coverage_to_file()
{
	TFILE=$1
	APPEND=""
	for CF in $(find ${TRAVIS_BUILD_DIR}/${RESULTS} -name "*.cov"); do
		if [ -s "$CF" ]; then
			APPEND="$APPEND -a $CF "
		fi
	done
	lcov $APPEND -o  ${TRAVIS_BUILD_DIR}/$TFILE

	find ${TRAVIS_BUILD_DIR}/${RESULTS} -type f -name "*.cov" -exec rm -f {} \;
}

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
export GCOV_PREFIX_STRIP=$BUILDTEST_PATH_DEPTH
GCOV_PREFIX=$RESULTSC1 xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose lhs &
GCOV_PREFIX=$RESULTSC2 xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose mhs &
GCOV_PREFIX=$RESULTSC3 xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose rhs &

cd ${TRAVIS_BUILD_DIR}/test/integration_test/RosterTest/
mkdir build && cd build
qmake .. && make
xvfb-run -a -e /dev/stdout ./RosterTest

# cp trace data to source dir; run lcov and generate test.cov
collect_coverage_at_path_to_file "$RESULTSC1" "c1.cov"
collect_coverage_at_path_to_file "$RESULTSC2" "c2.cov"
collect_coverage_at_path_to_file "$RESULTSC3" "c3.cov"

merge_client_coverage_to_file roster.cov 

# build and run the plain 1to1 msg test
killall -9 harbour-shmoose
${TRAVIS_BUILD_DIR}/scripts/travis/reset_ejabberd.sh
GCOV_PREFIX=$RESULTSC1 xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose lhs &
GCOV_PREFIX=$RESULTSC2 xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose rhs &

cd ${TRAVIS_BUILD_DIR}/test/integration_test/ClientCommunicationTest/
mkdir build && cd build
qmake .. && make
xvfb-run -a -e /dev/stdout ./ClientCommunicationTest

# cp trace data to source dir; run lcov and generate test.cov
collect_coverage_at_path_to_file "$RESULTSC1" "c1.cov"
collect_coverage_at_path_to_file "$RESULTSC2" "c2.cov"

merge_client_coverage_to_file 1o1.cov 

# build the plain room msg test
killall -9 harbour-shmoose
${TRAVIS_BUILD_DIR}/scripts/travis/reset_ejabberd.sh
xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose lhs &
xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose mhs &
xvfb-run -a -e /dev/stdout ${TRAVIS_BUILD_DIR}/${TESTPATH}/harbour-shmoose rhs &

${TRAVIS_BUILD_DIR}/scripts/travis/reset_ejabberd.sh
cd ${TRAVIS_BUILD_DIR}/test/integration_test/ClientRoomMessagingTest/
mkdir build && cd build
qmake .. && make
xvfb-run -a -e /dev/stdout ./ClientRoomMessagingTest

# cp trace data to source dir; run lcov and generate test.cov
collect_coverage_at_path_to_file "$RESULTSC1" "c1.cov"
collect_coverage_at_path_to_file "$RESULTSC2" "c2.cov"
collect_coverage_at_path_to_file "$RESULTSC3" "c3.cov"

merge_client_coverage_to_file room.cov 


# merge test tracefiles to final cov
lcov -a ${TRAVIS_BUILD_DIR}/roster.cov -a ${TRAVIS_BUILD_DIR}/1o1.cov -a ${TRAVIS_BUILD_DIR}/room.cov -o ${TRAVIS_BUILD_DIR}/$COVFILE 

# remove system files from /usr and generated moc files
lcov --remove ${TRAVIS_BUILD_DIR}/$COVFILE '/usr/*' --output-file ${TRAVIS_BUILD_DIR}/$COVFILE
lcov --remove ${TRAVIS_BUILD_DIR}/$COVFILE '*/test/moc_*' --output-file ${TRAVIS_BUILD_DIR}/$COVFILE

# Uploading report to CodeCov
bash <(curl -s https://codecov.io/bash) -f ${TRAVIS_BUILD_DIR}/$COVFILE || echo "failed upload to Codecov"


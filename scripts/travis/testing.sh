#! /bin/bash

set -x

TESTPATH="testing"
COVFILE="coverage.info"
RESULTS="results"

GCOVRESULTSCOMMON="gcovresults"
RESULTSC1=${GITHUB_WORKSPACE}/${RESULTS}/${GCOVRESULTSCOMMON}/c1/
RESULTSC2=${GITHUB_WORKSPACE}/${RESULTS}/${GCOVRESULTSCOMMON}/c2/
RESULTSC3=${GITHUB_WORKSPACE}/${RESULTS}/${GCOVRESULTSCOMMON}/c3/

mkdir -p $RESULTSC1
mkdir -p $RESULTSC2
mkdir -p $RESULTSC3

BUILDTEST_PATH_DEPTH=$(echo "${GITHUB_WORKSPACE}/${TESTPATH}" | grep -o / | wc -l)

collect_coverage_at_path_to_file()
{
	RPATH=$1
	CFILE=$2
	if [ "$(ls -A $RPATH)" ]; then
		cp $RPATH/*.gcda ${GITHUB_WORKSPACE}/${TESTPATH}/
		echo "lcov --capture --directory ${GITHUB_WORKSPACE}/$TESTPATH --output-file ${GITHUB_WORKSPACE}/${RESULTS}/$CFILE"
		lcov --capture --directory ${GITHUB_WORKSPACE}/$TESTPATH --output-file ${GITHUB_WORKSPACE}/${RESULTS}/$CFILE
		rm -f ${GITHUB_WORKSPACE}/$TESTPATH/*.gcda
	fi

	rm -f $RPATH/*.gcda
}

merge_client_coverage_to_file()
{
	TFILE=$1
	APPEND=""
	for CF in $(find ${GITHUB_WORKSPACE}/${RESULTS} -name "*.cov"); do
		if [ -s "$CF" ]; then
			APPEND="$APPEND -a $CF "
		fi
	done

	if [ -n "$APPEND" ]; then
		echo "lcov $APPEND -o  ${GITHUB_WORKSPACE}/$TFILE"
		lcov $APPEND -o  ${GITHUB_WORKSPACE}/$TFILE
	fi

	find ${GITHUB_WORKSPACE}/${RESULTS} -type f -name "*.cov" -exec rm -f {} \;
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
make -j$(nproc)


##########################
# build and run the roster test
##########################
${GITHUB_WORKSPACE}/scripts/travis/reset_ejabberd.sh
export GCOV_PREFIX_STRIP=$BUILDTEST_PATH_DEPTH
GCOV_PREFIX=$RESULTSC1  ${GITHUB_WORKSPACE}/${TESTPATH}/harbour-shmoose lhs &
GCOV_PREFIX=$RESULTSC2  ${GITHUB_WORKSPACE}/${TESTPATH}/harbour-shmoose mhs &
GCOV_PREFIX=$RESULTSC3  ${GITHUB_WORKSPACE}/${TESTPATH}/harbour-shmoose rhs &

cd ${GITHUB_WORKSPACE}/test/integration_test/RosterTest/
mkdir build && cd build
qmake .. && make -j$(nproc)
 ./RosterTest

# cp trace data to source dir; run lcov and generate test.cov
collect_coverage_at_path_to_file "$RESULTSC1" "c1.cov"
collect_coverage_at_path_to_file "$RESULTSC2" "c2.cov"
collect_coverage_at_path_to_file "$RESULTSC3" "c3.cov"

merge_client_coverage_to_file roster.cov 

##########################
# build and run the plain 1to1 msg test
##########################
killall -9 harbour-shmoose
${GITHUB_WORKSPACE}/scripts/travis/reset_ejabberd.sh
GCOV_PREFIX=$RESULTSC1  ${GITHUB_WORKSPACE}/${TESTPATH}/harbour-shmoose lhs &
GCOV_PREFIX=$RESULTSC2  ${GITHUB_WORKSPACE}/${TESTPATH}/harbour-shmoose rhs &

cd ${GITHUB_WORKSPACE}/test/integration_test/ClientCommunicationTest/
mkdir build && cd build
qmake .. && make -j$(nproc)
 ./ClientCommunicationTest

# cp trace data to source dir; run lcov and generate test.cov
collect_coverage_at_path_to_file "$RESULTSC1" "c1.cov"
collect_coverage_at_path_to_file "$RESULTSC2" "c2.cov"

merge_client_coverage_to_file 1o1.cov 

##########################
# build and run the plain room msg test
##########################
killall -9 harbour-shmoose
${GITHUB_WORKSPACE}/scripts/travis/reset_ejabberd.sh
GCOV_PREFIX=$RESULTSC1 ${GITHUB_WORKSPACE}/${TESTPATH}/harbour-shmoose lhs &
GCOV_PREFIX=$RESULTSC2 ${GITHUB_WORKSPACE}/${TESTPATH}/harbour-shmoose mhs &
GCOV_PREFIX=$RESULTSC3 ${GITHUB_WORKSPACE}/${TESTPATH}/harbour-shmoose rhs &

${GITHUB_WORKSPACE}/scripts/travis/reset_ejabberd.sh
cd ${GITHUB_WORKSPACE}/test/integration_test/ClientRoomMessagingTest/
mkdir build && cd build
qmake .. && make -j$(nproc)
 ./ClientRoomMessagingTest

# cp trace data to source dir; run lcov and generate test.cov
collect_coverage_at_path_to_file "$RESULTSC1" "c1.cov"
collect_coverage_at_path_to_file "$RESULTSC2" "c2.cov"
collect_coverage_at_path_to_file "$RESULTSC3" "c3.cov"

merge_client_coverage_to_file room.cov 

##########################
# build and run a clean omemo msg test
##########################
echo "------omemo test----------"
killall -9 harbour-shmoose

# enable omemo feature, override everthing else.
echo -e "[swfeatures]\nomemo=true" > /home/runner/.config/shmooselhs/harbour-shmoose.conf
echo -e "[swfeatures]\nomemo=true" > /home/runner/.config/shmooserhs/harbour-shmoose.conf

#cat /home/runner/.config/shmooselhs/harbour-shmoose.conf
#echo "----------------"
#cat /home/runner/.config/shmooserhs/harbour-shmoose.conf
#echo "----------------"

${GITHUB_WORKSPACE}/scripts/travis/reset_ejabberd.sh
GCOV_PREFIX=$RESULTSC1 ${GITHUB_WORKSPACE}/${TESTPATH}/harbour-shmoose lhs &
GCOV_PREFIX=$RESULTSC2 ${GITHUB_WORKSPACE}/${TESTPATH}/harbour-shmoose rhs &

cd ${GITHUB_WORKSPACE}/test/integration_test/OmemoTest/
mkdir build && cd build
qmake .. && make
 ./OmemoTest

# cp trace data to source dir; run lcov and generate test.cov
collect_coverage_at_path_to_file "$RESULTSC1" "c1.cov"
collect_coverage_at_path_to_file "$RESULTSC2" "c2.cov"

merge_client_coverage_to_file omemo1.cov

##########################
# run omemo test with existing db
##########################
killall -9 harbour-shmoose

# enable omemo feature, override everthing else.
echo -e "[swfeatures]\nomemo=true" > /home/runner/.config/shmooselhs/harbour-shmoose.conf
echo -e "[swfeatures]\nomemo=true" > /home/runner/.config/shmooserhs/harbour-shmoose.conf

GCOV_PREFIX=$RESULTSC1 ${GITHUB_WORKSPACE}/${TESTPATH}/harbour-shmoose lhs &
GCOV_PREFIX=$RESULTSC2 ${GITHUB_WORKSPACE}/${TESTPATH}/harbour-shmoose rhs &

# wait sometime to get dbus connected
sleep 5

${GITHUB_WORKSPACE}/test/integration_test/OmemoTest/build/OmemoTest

# cp trace data to source dir; run lcov and generate test.cov
collect_coverage_at_path_to_file "$RESULTSC1" "c1.cov"
collect_coverage_at_path_to_file "$RESULTSC2" "c2.cov"

merge_client_coverage_to_file omemo2.cov

# merge test tracefiles to final cov
APPEND=""
for CF in $(ls ${GITHUB_WORKSPACE}/*.cov); do
	if [ -s "$CF" ]; then
		APPEND="$APPEND -a $CF "
	fi
done
echo "lcov $APPEND -o ${GITHUB_WORKSPACE}/$COVFILE"
lcov $APPEND -o ${GITHUB_WORKSPACE}/$COVFILE

# remove system files from /usr and generated moc files
lcov --remove ${GITHUB_WORKSPACE}/$COVFILE '/usr/*' --output-file ${GITHUB_WORKSPACE}/$COVFILE
lcov --remove ${GITHUB_WORKSPACE}/$COVFILE '*/test/moc_*' --output-file ${GITHUB_WORKSPACE}/$COVFILE

# Uploading report to CodeCov
bash <(curl -s https://codecov.io/bash) -f ${GITHUB_WORKSPACE}/$COVFILE || echo "failed upload to Codecov"


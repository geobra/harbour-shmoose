#! /bin/bash

set -x

TESTPATH="unit_testing"
COVFILE="coverage.info"
RESULTS="results"

GCOVRESULTSCOMMON="gcovresults"
RESULTSUT=${GITHUB_WORKSPACE}/${RESULTS}/${GCOVRESULTSCOMMON}/ut/

mkdir -p $RESULTSUT

# run unit tests at the specified location
run_unit_test_at_path()
{
	UTPATH=$1
	CURRENTPATH=$(pwd)

	mkdir -p $UTPATH/build
	cd $UTPATH/build
	qmake .. DEFINES+=TRAVIS
	make -j$(nproc)

	EXEC=$(find . -type f -executable | grep -v ".sh$")
	$EXEC

	cd "$CURRENTPATH"
}

# find all unit test which have a pro file
UTPRO=$(find ${GITHUB_WORKSPACE}/test/unit_tests/ -name "*.pro")
for PRO in $UTPRO; do
	UTPATH=$(dirname $PRO)
	run_unit_test_at_path $UTPATH
done


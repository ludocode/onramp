#!/bin/bash

# This script generates .i files for the given preprocessor.
#
# Note that any bugs in the preprocessor may result in bugs in the generated
# tests.
#
# Use this only to generate new tests or to re-generate all tests in
# case of a change in the output format of all preprocessors. In any case,
# review the diffs carefully to ensure it has not introduced any bugs.

if [ "$1" == "" ]; then
    echo "Need command to test."
    exit 1
fi

COMMAND="$@"

for TESTCASE in $(find $(dirname $0)/* -name '*.c'); do
    echo -n "Running $TESTCASE... "

    EXPECTED=$(echo $TESTCASE|sed 's/\.c$/.i/')
    ARGSFILE=$(echo $TESTCASE|sed 's/\.c$/.args/')

    ARGS=
    if [ -e $ARGSFILE ]; then
        # eval echo to expand shell macros
        ARGS=$(eval echo $(cat $ARGSFILE))
    fi

    TEMPFILE=$(mktemp)
    $COMMAND $ARGS $TESTCASE -o $TEMPFILE &> /dev/null
    RET=$?

    if [ $RET -eq 0 ]; then
        cp $TEMPFILE $EXPECTED
        echo "Generated $EXPECTED"
    else
        rm -f $EXPECTED
        echo "Failed, deleted $EXPECTED *****"
    fi

    rm -f $TEMFILE
done

# Run the test script to make sure we generated correctly
$(dirname $0)/test.sh $COMMAND

#!/bin/bash

# This script tests the given preprocessor by running it against all .c files
# in all subfolders of this test folder.
#
# If a corresponding .i file exists, the preprocessor must succeed and its
# output must match the file's contents.
#
# If no corresponding .i file exists, the preprocessor must return an error.
# Its output is ignored.
#
# If a corresponding .args file exists, the contents are passed as command-line
# arguments to the preprocessor.
#
# If a corresponding .strict file exists, the test is only performed in
# --strict mode. (This is typically used to test optional error-checking.)
#
# TODO we should have a special exit code so that we can differentiate between
# the preprocessor crashing as opposed to printing an error and exiting.

if [ "$1" == "" ]; then
    echo "Need command to test."
    exit 1
fi

COMMAND="$@"
TEMPFILE=$(mktemp)
ERROR=0

for TESTCASE in $(find $(dirname $0)/* -name '*.c'); do
    echo "Testing $TESTCASE"
    EXPECTED=$(echo $TESTCASE|sed 's/\.c$/.i/')
    ARGSFILE=$(echo $TESTCASE|sed 's/\.c$/.args/')

    ARGS=
    if [ -e $ARGSFILE ]; then
        # eval echo to expand shell macros
        ARGS=$(eval echo $(cat $ARGSFILE))
    fi

    $COMMAND $ARGS $TESTCASE -o $TEMPFILE &> /dev/null
    RET=$?

    if [ -e $EXPECTED ]; then
        if [ $RET -ne 0 ]; then
            echo "ERROR: $TESTCASE failed; expected success."
            echo "Command: $COMMAND $ARGS $TESTCASE -o $TEMPFILE"
            ERROR=1
        elif ! diff -q $EXPECTED $TEMPFILE > /dev/null; then
            echo "ERROR: $TESTCASE did not match expected $EXPECTED"
            echo "Command: $COMMAND $ARGS $TESTCASE -o $TEMPFILE"
            ERROR=1
        fi
    else
        if [ $RET -eq 0 ]; then
            echo "ERROR: $TESTCASE succeeded; expected error."
            echo "Command: $COMMAND $ARGS $TESTCASE -o $TEMPFILE"
            ERROR=1
        fi
    fi

    rm -f $TEMFILE
done

if [ $ERROR -eq 1 ]; then
    echo "Errors occurred."
    exit 1
fi

echo "Pass."

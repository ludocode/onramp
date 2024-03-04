#!/bin/bash

# This script tests the given hex tool by running it against all .ohx files
# in all subfolders of this test folder.
#
# If a corresponding .stdout file exists, the hex tool must succeed and its
# output must match the file's contents.
#
# If no corresponding .stdout file exists, the hex tool must exit with a
# non-zero status and its output is ignored, unless --lax is provided in which
# case the test case is skipped.
#
# You can pass --lax to skip test cases that check for error handling. A hex
# tool that passes under --lax is sufficient to bootstrap Onramp provided there
# are no errors in the files it reads.

LAX=0
if [ "$1" == "--lax" ]; then
    LAX=1
    shift
fi

if [ "$1" == "" ]; then
    echo "Need command to test."
    exit 1
fi

COMMAND="$@"
TEMPFILE=$(mktemp)
ERROR=0

echo "Running hex tests on: $COMMAND"

for TESTCASE in $(find "$(dirname $0)"/* -name '*.ohx'); do
    EXPECTED=$(echo $TESTCASE|sed 's/\.ohx$/.stdout/')

    if ! [ -e $EXPECTED ] && [ $LAX -eq 1 ]; then
        echo "Skipping $TESTCASE"
        continue
    fi

    echo "Testing $TESTCASE"
    $COMMAND $TESTCASE -o $TEMPFILE &> /dev/null
    RET=$?

    if [ -e $EXPECTED ]; then
        if [ $RET -ne 0 ]; then
            echo "ERROR: $TESTCASE failed; expected success."
            echo "Command: $COMMAND $TESTCASE -o $TEMPFILE"
            ERROR=1
        elif ! diff -q $EXPECTED $TEMPFILE > /dev/null; then
            echo "ERROR: $TESTCASE did not match expected $EXPECTED"
            echo "Command: $COMMAND $TESTCASE -o $TEMPFILE"
            ERROR=1
        fi
    else
        if [ $RET -eq 0 ]; then
            echo "ERROR: $TESTCASE succeeded; expected error."
            echo "Command: $COMMAND $TESTCASE -o $TEMPFILE"
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

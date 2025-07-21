#!/bin/bash

# This script tests the given VM by running it against all test files in all
# subfolders of this test folder.
#
# If corresponding .stdout or .stderr files exists, the program's output to
# stdout or stderr respectively must match the file contents. Otherwise, the
# program's output is ignored.
#
# If a corresponding .stdin file exists, the contents are fed to the program's
# stdin.
#
# If a corresponding .status file exists, it must contain a number, and the
# program must exit with a status code that matches the number. Otherwise, the
# program must exit with status 0.
#
# If a corresponding .abort file exists, the VM must abort execution due to
# some illegal instruction or other illegal behaviour and exit with status
# 125.
#
# If a corresponding .args file exists, the contents are passed as command-line
# arguments to the program. Otherwise, no additional arguments are passed.
#
# If a corresponding .env file exists, the contents are passed as environment
# variables to the program. Otherwise, the program inherits the current
# environment.
#
# If a corresponding .skip file exists, the test is skipped.
#
# If a corresponding .strict file exists and --lax is passed, the test is
# skipped.
#
# You can pass --lax to skip test cases that check for illegal instructions.
# Checking for illegal instructions is not required.

LAX=0
if [ "$1" == "--lax" ]; then
    LAX=1
    shift
fi

if [ "$1" == "" ]; then
    echo "Need command to test."
    exit 1
fi

if [ "$(realpath $(dirname $0)/../..)" != "$(realpath $(pwd))" ]; then
    echo "ERROR: This script must be run from the Onramp root."
    exit 1
fi

# Create a directory for temp files
ONRAMP_TMPDIR=
cleanup() {
    if ! [ -z "$ONRAMP_TMPDIR" ]; then
        rm -r "$ONRAMP_TMPDIR"
    fi
}
trap cleanup EXIT
ONRAMP_TMPDIR=$(mktemp -d)
if [ -z "$ONRAMP_TMPDIR" ]; then
    echo "$0: ERROR: Failed to create a temporary directory." >&1
    exit 1
fi

COMMAND="$@"
TEMP_OE=$ONRAMP_TMPDIR/onramp-test.oe
TEMP_STDOUT=$ONRAMP_TMPDIR/onramp-test.stdout
TEMP_STDERR=$ONRAMP_TMPDIR/onramp-test.stderr
ANY_ERROR=0

( $(dirname $0)/../../platform/hex/c89/build.sh ) || exit $?
HEX=$(dirname $0)/../../build/test/hex-c89/hex

echo "Running vm tests on: $COMMAND"

for HEXNAME in $(find $(dirname $0)/* -name '*.oe.ohx'|sort); do
    THIS_ERROR=0
    BASENAME=$(echo $HEXNAME|sed 's/\.oe\.ohx$//')

    if [ -e $BASENAME.skip ]; then
        echo "Skipping $BASENAME due to .skip"
        continue
    fi

    if [ $LAX -eq 1 ] && [ -e $BASENAME.strict ]; then
        echo "Skipping $BASENAME due to --lax"
        continue
    fi

    echo "Testing $BASENAME"

    ARGS=
    if [ -e $BASENAME.args ]; then
        # eval echo to expand shell macros
        ARGS=$(eval echo $(cat $BASENAME.args))
    fi

    # convert test case
    $HEX $HEXNAME -o $TEMP_OE || exit $?

    # get stdin
    if [ -e $BASENAME.stdin ]; then
        TESTSTDIN=$BASENAME.stdin
    else
        TESTSTDIN=/dev/null
    fi

    cat $TESTSTDIN | $COMMAND $TEMP_OE $ARGS 1>$TEMP_STDOUT 2>$TEMP_STDERR
    RET=$?

    # check for status or abort file
    if [ -e $BASENAME.status ] && [ -e $BASENAME.abort ]; then
        echo "ERROR: $BASENAME cannot have both .status and .abort files."
        THIS_ERROR=1
    elif [ -e $BASENAME.status ]; then
        EXPECTED=$(cat $BASENAME.status)
        if [ $RET -ne $EXPECTED ]; then
            echo "ERROR: $BASENAME exited with status $RET, expected status $EXPECTED"
            THIS_ERROR=1
        fi
    elif [ -e $BASENAME.abort ]; then
        if [ $RET -ne 125 ]; then
            echo "ERROR: $BASENAME exited with status $RET, expected VM to abort with status 125"
            THIS_ERROR=1
        fi
    else
        if [ $RET -ne 0 ]; then
            echo "ERROR: $BASENAME failed; expected success."
            THIS_ERROR=1
        fi
    fi

    # check for stdout
    if [ -e $BASENAME.stdout ]; then
        if ! diff -q $BASENAME.stdout $TEMP_STDOUT > /dev/null; then
            echo "ERROR: $BASENAME stdout did not match expected"
            THIS_ERROR=1
        fi
    fi

    # check for stderr
    if [ -e $BASENAME.stderr ]; then
        if ! diff -q $BASENAME.stderr $TEMP_STDERR > /dev/null; then
            echo "ERROR: $BASENAME stderr did not match expected"
            THIS_ERROR=1
        fi
    fi

    if [ $THIS_ERROR -ne 0 ]; then
        ANY_ERROR=1
        echo "Commands:"
        echo "    $HEX $HEXNAME -o $TEMP_OE && \\"
        echo -n "        "
        if [ -e $BASENAME.stdin ]; then
            echo -n "cat $TESTSTDIN | "
        fi
        echo "$COMMAND $TEMP_OE $ARGS"
    fi

    rm -f $TEMP_OE
    rm -f $TEMPOUTPUT
done

if [ $ANY_ERROR -eq 1 ]; then
    echo "Errors occurred."
    exit 1
fi

echo "Pass."

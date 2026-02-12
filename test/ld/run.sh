#!/bin/bash

# This script tests the given linker by running it against all .oo files
# in all subfolders of the given test folder.
#
#     Usage: run.sh <test folder> <run commands>
#
# e.g.
#
#     test/ld/run.sh test/ld/0-global onrampvm output/intermediate/ld-0-global/ld.oe
#
# - If a corresponding .fail file exists, the linker must return an error. Its
# output is ignored.
#
# - If a corresponding .oe file exists, the linker must succeed and its output
# must match the file's contents.
#
# - If no corresponding .oe or .fail file exists, the linker must succeed. Its
# output is ignored.
#
# - If a corresponding .skip file exists, the test is skipped.
#
# - If a corresponding .args file exists, the contents are passed as
# command-line arguments to the linker instead of the default arguments. Use
# $INPUT for the input filename, $OUTPUT for the output filename, and $LIBC for
# libc/0.
#
# - If a corresponding .stdout file exists, the program is run. The program
# must exit with status 0 and the output of running the program must match the
# file's contents.

if [ "$1" == "" ]; then
    echo "Need folder to test."
    exit 1
fi
if [ "$2" == "" ]; then
    echo "Need command to test."
    exit 1
fi

if ! command -v onrampvm > /dev/null; then
    echo "ERROR: onrampvm is required on PATH."
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

# build some dependencies
set -e
ROOT=$(dirname $0)/../..

SOURCE_FOLDER="$1"
shift
COMMAND="$@"
TEMP_OE=$ONRAMP_TMPDIR/onramp-test.oe
TEMP_STDOUT=$ONRAMP_TMPDIR/onramp-test.stdout
ANY_ERROR=0

# Some tests link against the libc. They provide arguments that use it here. We
# use libc/0 because early linker stages don't have the capacity to link the
# later libcs.
make -C $ROOT/test/libc/0-oo/ build
LIBC=$ROOT/output/test/libc-0-oo/libc.oa

TESTS_PATH="$(basename $(realpath $SOURCE_FOLDER/..))/$(basename $(realpath $SOURCE_FOLDER))"
echo "Running $TESTS_PATH tests on: $COMMAND"

for TESTFILE in $(find $SOURCE_FOLDER/* -name '*.oo'); do
    THIS_ERROR=0
    BASENAME=$(echo $TESTFILE|sed 's/\.oo$//')

    if [ -e $BASENAME.skip ]; then
        echo "Skipping $BASENAME"
        continue
    fi
    echo "Testing $TESTFILE"

    INPUT=$TESTFILE
    OUTPUT=$TEMP_OE
    ARGS=
    if [ -e $BASENAME.args ]; then
        # eval echo to expand shell macros
        ARGS=$(eval echo $(cat $BASENAME.args))
    else
        ARGS="$INPUT -o $OUTPUT"
    fi

    # link
    set +e
    $COMMAND $ARGS &> /dev/null
    RET=$?
    set -e

    # check link status and object code
    if [ $RET -eq 125 ]; then
        echo "ERROR: linker crashed on $BASENAME; expected success or error message."
        cat $TEMP_STDERR
        THIS_ERROR=1
    elif [ -e $BASENAME.fail ]; then
        if [ $RET -eq 0 ]; then
            echo "ERROR: $BASENAME linking succeeded; expected error."
            THIS_ERROR=1
        fi
    elif [ $RET -ne 0 ]; then
        echo "ERROR: $BASENAME linking failed; expected success."
        THIS_ERROR=1
    elif [ -e $BASENAME.oe ] && ! diff -q $BASENAME.oe $TEMP_OE > /dev/null; then
        echo "ERROR: $BASENAME did not match expected $BASENAME.oe"
        THIS_ERROR=1
    fi

    # if successful, run
    if [ $THIS_ERROR -ne 1 ] && [ -e $BASENAME.stdout ]; then
        set +e
        onrampvm $TEMP_OE > $TEMP_STDOUT
        RET=$?
        set -e
        if [ $RET -ne 0 ]; then
            echo "ERROR: $TESTFILE failed to run."
            THIS_ERROR=1
        elif ! diff $TEMP_STDOUT $BASENAME.stdout > /dev/null; then
            echo "ERROR: $TESTFILE output did not match expected $BASENAME.stdout"
            THIS_ERROR=1
        fi
    fi

    if [ $THIS_ERROR -eq 1 ]; then
        echo "Command: make build && $COMMAND $ARGS && onrampvm $TEMP_OE"
        ANY_ERROR=1
    fi

    rm -f $TEMP_OE
    rm -f $TEMP_STDOUT
done

if [ $ANY_ERROR -eq 1 ]; then
    echo "Errors occurred."
    exit 1
fi

echo "Pass."

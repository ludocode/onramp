#!/bin/bash

# This script tests the given linker by running it against all .oo files
# in all subfolders of the given test folder.
#
#     Usage: run.sh <test folder> <run commands>
#
# e.g.
#
#     test/ld/run.sh test/ld/0-global onrampvm build/intermediate/ld-0-global/ld.oe
#
# - If a corresponding .oe file exists, the linker must succeed and its output
# must match the file's contents.
#
# - If no corresponding .oe file exists, the linker must return an error. Its
# output is ignored.
#
# - If a corresponding .args file exists, the contents are passed as command-line
# arguments to the linker.
#
# - If a corresponding .stdout file exists, the resulting program is run, and the
# output must match the file's contents.

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

SOURCE_FOLDER="$1"
shift
COMMAND="$@"
TEMP_OE=/tmp/onramp-test.oe
TEMP_STDOUT=/tmp/onramp-test.stdout
ERROR=0

TESTS_PATH="$(basename $(realpath $SOURCE_FOLDER/..))/$(basename $(realpath $SOURCE_FOLDER))"
echo "Running $TESTS_PATH tests on: $COMMAND"

for TESTFILE in $(find $SOURCE_FOLDER/* -name '*.oo'); do
    echo "Testing $TESTFILE"
    BASENAME=$(echo $TESTFILE|sed 's/\.oo$//')

    INPUT=$TESTFILE
    OUTPUT=$TEMP_OE
    ARGS=
    if [ -e $BASENAME.args ]; then
        # eval echo to expand shell macros
        ARGS=$(eval echo $(cat $BASENAME.args))
    else
        ARGS="$INPUT -o $OUTPUT"
    fi

    $COMMAND $ARGS &> /dev/null
    RET=$?

    if [ -e $BASENAME.oe ]; then
        if [ $RET -ne 0 ]; then
            echo "ERROR: $TESTFILE failed to link; expected success."
            echo "Command: make build && $COMMAND $ARGS"
            ERROR=1
        elif ! diff -q $BASENAME.oe $TEMP_OE > /dev/null; then
            echo "ERROR: $TESTFILE did not match expected $BASENAME.oe"
            echo "Command: make build && $COMMAND $ARGS"
            ERROR=1
        fi
    else
        if [ $RET -eq 0 ]; then
            echo "ERROR: $TESTFILE linking succeeded; expected error."
            echo "Command: make build && $COMMAND $ARGS"
            ERROR=1
        fi
    fi

    if [ -e $BASENAME.stdout ]; then
        onrampvm $TEMP_OE > $TEMP_STDOUT
        if ! diff $TEMP_STDOUT $BASENAME.stdout > /dev/null; then
            echo "ERROR: $TESTFILE output did not match expected $BASENAME.stdout"
            echo "Command: make build && $COMMAND $ARGS && onrampvm $TEMP_OE"
            ERROR=1
        fi
    fi

    rm -f $TEMP_OE
    rm -f $TEMP_STDOUT
done

if [ $ERROR -eq 1 ]; then
    echo "Errors occurred."
    exit 1
fi

echo "Pass."

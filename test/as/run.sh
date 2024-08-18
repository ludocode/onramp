#!/bin/bash

# This script tests the given assembler by running it against all .os files
# in all subfolders of the given test folder.
#
#     Usage: run.sh <test folder> <run commands>
#
# e.g.
#
#     test/as/run.sh test/as/0-basic onrampvm build/intermediate/as-0-basic/as.oe
#
# For each test case:
#
# - If a corresponding .oo file exists, the assembler must succeed and its output
# must match the file's contents.
#
# - If no corresponding .oo file exists, the assembler must return an error. Its
# output is ignored.
#
# - If a corresponding .args file exists, the contents are passed as
# command-line arguments to the assembler instead of the default arguments. Use
# $INPUT for the input filename and $OUTPUT for the output filename.
#
# If the program assembles correctly, it is linked and run. In this case:
#
# - If a corresponding .status file exists, the exit code must match the file's
# contents. Otherwise, the program must exit with status 0.
#
# - If a corresponding .stdout file exists, the program's output must match the
# file's contents.
#
# - If a corresponding .skip file exists, the test is skipped.
#
# Pass --other-stage as the first argument to run tests with a different stage
# than the one for which the tests were intended. The assembly output will not
# be compared and any tests with a .nonstd file will be skipped.

OTHER_STAGE=0
if [ "$1" == "--other-stage" ]; then
    OTHER_STAGE=1
    shift
fi

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

# build some dependencies
set -e
ROOT=$(dirname $0)/../..
make -C $ROOT/test/ld/2-full/ build
make -C $ROOT/test/libc/0-oo/ build

SOURCE_FOLDER="$1"
shift
COMMAND="$@"
TEMP_OO=/tmp/onramp-test.oo
TEMP_OE=/tmp/onramp-test.oe
TEMP_STDOUT=/tmp/onramp-test.stdout
ANY_ERROR=0

TESTS_PATH="$(basename $(realpath $SOURCE_FOLDER/..))/$(basename $(realpath $SOURCE_FOLDER))"
echo "Running $TESTS_PATH tests on: $COMMAND"

for TESTFILE in $(find $SOURCE_FOLDER/* -name '*.os'); do
    THIS_ERROR=0
    BASENAME=$(echo $TESTFILE|sed 's/\.os$//')

    if [ -e $BASENAME.skip ] || ( [ $OTHER_STAGE -eq 1 ] && [ -e $BASENAME.nonstd ] ); then
        echo "Skipping $BASENAME"
        continue
    fi

    echo "Testing $BASENAME"

    # collect or generate the args
    if [ -e $BASENAME.args ]; then
        # eval echo to expand shell macros
        ARGS="$(eval echo $(cat $BASENAME.args))"
    else
        ARGS="$BASENAME.os -o $TEMP_OO"
    fi

    # assemble
    set +e
    $COMMAND $ARGS &> /dev/null
    RET=$?
    set -e

    # check compile status and object code
    if [ -e $BASENAME.oo ]; then
        if [ $RET -ne 0 ]; then
            echo "ERROR: $BASENAME failed; expected success."
            THIS_ERROR=1
        elif [ $OTHER_STAGE -eq 0 ] && ! diff -q $BASENAME.oo $TEMP_OO > /dev/null; then
            echo "ERROR: $BASENAME did not match expected $BASENAME.oo"
            THIS_ERROR=1
        fi
    else
        if [ $RET -eq 0 ]; then
            echo "ERROR: $BASENAME succeeded; expected error."
            THIS_ERROR=1
        fi
    fi

    if [ -e $BASENAME.oo ]; then

        # link and run
        if [ $THIS_ERROR -ne 1 ] && ! $ROOT/build/test/ld-2-full/ld -g \
                $ROOT/build/test/libc-0-oo/libc.oa $TEMP_OO -o $TEMP_OE &> /dev/null; then
            # TODO libc/1 not libc/0 ^^^^^
            echo "ERROR: $BASENAME failed to link."
            THIS_ERROR=1
        fi
        if [ $THIS_ERROR -ne 1 ]; then
            set +e
            onrampvm $TEMP_OE >$TEMP_STDOUT 2>/dev/null
            RET=$?
            set -e
        fi

        # check run status
        if [ $THIS_ERROR -ne 1 ]; then
            if [ -e $BASENAME.status ]; then
                EXPECTED=$(cat $BASENAME.status)
                if [ $RET -ne $EXPECTED ]; then
                    echo "ERROR: $BASENAME exited with status $RET, expected status $EXPECTED"
                    THIS_ERROR=1
                fi
            else
                if [ $RET -ne 0 ]; then
                    echo "ERROR: $BASENAME exited with status $RET, expected success"
                    THIS_ERROR=1
                fi
            fi
        fi

        # check stdout
        if [ $THIS_ERROR -ne 1 ] && [ -e $BASENAME.stdout ] && ! diff -q $BASENAME.stdout $TEMP_STDOUT >/dev/null; then
            echo "ERROR: $BASENAME stdout did not match expected"
            THIS_ERROR=1
        fi
    fi

    if [ $THIS_ERROR -eq 1 ]; then
        echo "Commands:"
        echo "    make build && \\"
        echo "    $COMMAND $ARGS && \\"
        echo "    $ROOT/build/test/ld-2-full/ld -g $ROOT/build/test/libc-0-oo/libc.oa $TEMP_OO -o $TEMP_OE && \\"
        echo "    onrampvm $TEMP_OE >$TEMP_STDOUT && \\"
        echo "    diff -u $BASENAME.oo $TEMP_OO"
        if [ -e $BASENAME.stdout ]; then
            echo "    diff -u $BASENAME.stdout $TEMP_STDOUT"
        fi
        ANY_ERROR=1
    fi

    # clean up
    rm -f $TEMP_I
    rm -f $TEMP_OS
    rm -f $TEMP_OO
    rm -f $TEMP_OE
    rm -f $TEMP_STDOUT
done

if [ $ANY_ERROR -eq 1 ]; then
    echo "Errors occurred."
    exit 1
fi

echo "Pass."

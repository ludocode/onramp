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

# Exit on Ctrl-C
trap "exit" INT

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

COMMAND="$@"
ANY_ERROR=0

# generate an output folder based on the VM name (including any wrapper
# executable and excluding paths. e.g. `dash platform/vm/sh/vm.sh` becomes
# `dash_vm_sh`)
OUTPUT_PATH=output/test/vm/$(echo "$COMMAND"|sed 's@[a-z_]*/@@g'|tr -c '[:alnum:]\n' _)

( $(dirname $0)/../../platform/hex/c89/build.sh ) || exit $?
HEX=$(dirname $0)/../../output/configure/hex-c89/hex

echo "Running vm tests on: $COMMAND"

for HEXNAME in $(find $(dirname $0)/* -name '*.oe.ohx'|sort); do
    THIS_ERROR=0
    BASENAME=$(echo $HEXNAME|sed 's/\.oe\.ohx$//')

    # generate an output name for the test
    OUTPUT_NAME=${BASENAME#$(dirname $0)}
    OUTPUT_NAME=${OUTPUT_NAME##/}
    OUTPUT_NAME=$OUTPUT_PATH/$OUTPUT_NAME
    mkdir -p "$(dirname "$OUTPUT_NAME")"

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
    $HEX $HEXNAME -o $OUTPUT_NAME.oe || exit $?

    # get stdin
    if [ -e $BASENAME.stdin ]; then
        TESTSTDIN=$BASENAME.stdin
    else
        TESTSTDIN=/dev/null
    fi

    cat $TESTSTDIN | $COMMAND $OUTPUT_NAME.oe $ARGS 1>$OUTPUT_NAME.stdout 2>$OUTPUT_NAME.stderr
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
        if ! diff -q $BASENAME.stdout $OUTPUT_NAME.stdout > /dev/null; then
            echo "ERROR: $BASENAME stdout did not match expected"
            THIS_ERROR=1
        fi
    fi

    # check for stderr
    if [ -e $BASENAME.stderr ]; then
        if ! diff -q $BASENAME.stderr $OUTPUT_NAME.stderr > /dev/null; then
            echo "ERROR: $BASENAME stderr did not match expected"
            THIS_ERROR=1
        fi
    fi

    if [ $THIS_ERROR -ne 0 ]; then
        ANY_ERROR=1
        echo "Commands:"
        echo "    $HEX $HEXNAME -o $OUTPUT_NAME.oe && \\"
        echo -n "        "
        if [ -e $BASENAME.stdin ]; then
            echo -n "cat $TESTSTDIN | "
        fi
        echo "$COMMAND $OUTPUT_NAME.oe $ARGS"
        continue
    fi

    # temporary files are only cleaned if the test passed
    rm -f $OUTPUT_NAME.oe
    rm -f $OUTPUT_NAME.stdout
    rm -f $OUTPUT_NAME.stderr
done

if [ $ANY_ERROR -eq 1 ]; then
    echo "Errors occurred."
    exit 1
fi

echo "Pass."

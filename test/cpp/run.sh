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
# TODO this is now done with cci, we make asan exit with code 125 which is the
# same code for a vm crash so we can detect crashes in both, need to do the
# same here

if [ "$1" == "" ]; then
    echo "Need command to test."
    exit 1
fi

# TODO the source folder needs to be the current folder to make paths in #line
# match, but we can't really switch to it because the command is in multiple
# parts and some may have relative paths. We should just not bother taking a
# source folder, instead the source folder should always just be '.', i.e. the
# working directory where the script is called
SOURCE_FOLDER="$1"
shift
COMMAND="$@"
TEMP_I=/tmp/onramp-test.i
ERROR=0

TESTS_PATH="$(basename $(realpath $SOURCE_FOLDER/..))/$(basename $(realpath $SOURCE_FOLDER))"
echo "Running $TESTS_PATH tests on: $COMMAND"

for TESTCASE in $(find $SOURCE_FOLDER/* -name '*.c'); do
    echo "Testing $TESTCASE"
    BASENAME=$(echo $TESTCASE|sed 's/\.c$//')

    INPUT=$TESTCASE
    OUTPUT=$TEMP_I
    ARGS=
    if [ -e $BASENAME.args ]; then
        # eval echo to expand shell macros
        ARGS=$(eval echo -- "\"$(cat $BASENAME.args)\"")
    else
        ARGS="$INPUT -o $OUTPUT"
    fi

    $COMMAND $ARGS &> /dev/null
    RET=$?

    if [ -e $BASENAME.i ]; then
        if [ $RET -ne 0 ]; then
            echo "ERROR: $TESTCASE failed; expected success."
            echo "Command: $COMMAND $ARGS"
            ERROR=1
        elif ! diff -q $BASENAME.i $TEMP_I > /dev/null; then
            echo "ERROR: $TESTCASE did not match expected $BASENAME.i"
            echo "Command: $COMMAND $ARGS"
            ERROR=1
        fi
    else
        if [ $RET -eq 0 ]; then
            echo "ERROR: $TESTCASE succeeded; expected error."
            echo "Command: $COMMAND $ARGS"
            ERROR=1
        fi
    fi

    rm -f $TEMP_I
done

if [ $ERROR -eq 1 ]; then
    echo "Errors occurred."
    exit 1
fi

echo "Pass."

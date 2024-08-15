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

SOURCE_FOLDER="$1"
shift
COMMAND="$@"
TEMP_I=/tmp/onramp-test.i

for TESTCASE in $(find $SOURCE_FOLDER/* -name '*.c'); do
    echo -n "Running $TESTCASE... "
    BASENAME=$(echo $TESTCASE|sed 's/\.c$//')

    INPUT=$TESTCASE
    OUTPUT=$TEMP_I
    ARGS=
    if [ -e $BASENAME.args ]; then
        # eval echo to expand shell macros
        ARGS=$(eval echo "\"$(cat $BASENAME.args)\"")
    else
        ARGS="$INPUT -o $OUTPUT"
    fi

    $COMMAND $ARGS &> /dev/null
    RET=$?

    if [ $RET -eq 0 ]; then
        cp $TEMP_I $BASENAME.i
        echo "Generated $BASENAME.i"
    else
        rm -f $BASENAME.i
        echo "Failed, deleted $BASENAME.i *****"
    fi

    rm -f $TEMP_I
done

# Run the test script to make sure we generated correctly
$(dirname $0)/run.sh $SOURCE_FOLDER $COMMAND

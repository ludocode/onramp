#!/bin/bash

# This script generates .oo files using the given assembler.
#
# Note that any bugs in the assembler may result in bugs in the generated
# tests.
#
# Use this only to generate new tests or to re-generate all tests in
# case of a change in the assembler spec. In any case, review the diffs
# carefully to ensure it has not introduced any bugs.

if [ "$1" == "" ]; then
    echo "Need folder to test."
    exit 1
fi
if [ "$2" == "" ]; then
    echo "Need command to test."
    exit 1
fi

SOURCE_FOLDER="$1"
shift
COMMAND="$@"
TEMP_OO=/tmp/onramp-test.oo

TESTS_PATH="$(basename $(realpath $SOURCE_FOLDER/..))/$(basename $(realpath $SOURCE_FOLDER))"
echo "Generating $TESTS_PATH with: $COMMAND"

for TESTFILE in $(find $SOURCE_FOLDER/* -name '*.os'); do
    BASENAME=$(echo $TESTFILE|sed 's/\.os$//')
    echo -n "Running $BASENAME... "
    EXPECTED=$BASENAME.oo
    ARGSFILE=$BASENAME.args

    ARGS="$TESTFILE -o $TEMP_OO"
    if [ -e $ARGSFILE ]; then
        # eval echo to expand shell macros
        ARGS=$(eval echo $(cat $ARGSFILE))
    fi

    $COMMAND $ARGS &> /dev/null
    RET=$?

    if [ $RET -eq 0 ]; then
        cp $TEMP_OO $EXPECTED
        echo "Generated $EXPECTED"
    else
        rm -f $EXPECTED
        echo "Failed, deleted $EXPECTED *****"
    fi

    rm -f $TEMFILE
done

# Run the test script to make sure we generated correctly
$(dirname $0)/run.sh $SOURCE_FOLDER $COMMAND

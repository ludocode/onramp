#!/bin/bash

# This script generates .out files for the given hex tool.
#
# Note that any bugs in the hex tool may result in bugs in the generated
# tests. In particular, you should only generate with a tool that passes all
# strict tests.
#
# Use this only to generate new tests or to re-generate all tests in
# case of a change in the output format of all hex tools. In any case,
# review the diffs carefully to ensure it has not introduced any bugs.

if [ "$1" == "" ]; then
    echo "Need command to test."
    exit 1
fi

COMMAND=$(realpath $1)

cd $(dirname $0)
for TESTCASE in $(find * -name '*.hex'); do
    echo -n "Running $TESTCASE... "

    EXPECTED=$(echo $TESTCASE|sed 's/\.hex$/.out/')
    TEMPFILE=$(mktemp)
    $COMMAND $TESTCASE -o $TEMPFILE &> /dev/null
    RET=$?

    if [ $RET -eq 0 ]; then
        cp $TEMPFILE $EXPECTED
        echo "Generated $EXPECTED"
    else
        rm -f $EXPECTED
        echo "Failed, deleted $EXPECTED *****"
    fi

    rm -f $TEMFILE
done

# Run the test script to make sure we generated correctly
$(dirname $0)/test.sh --strict $COMMAND

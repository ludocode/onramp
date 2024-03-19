#!/bin/bash

# This script generates .oe files in the given folder using the given linker.
#
# Note that any bugs in the linker may result in bugs in the generated tests.
#
# Use this only to generate new tests or to re-generate all tests in
# case of a change in the linker implementation. In any case, review the diffs
# carefully to ensure it has not introduced any bugs.

if [ "$1" == "" ]; then
    echo "Need command to test."
    exit 1
fi

SOURCE_FOLDER="$1"
shift
COMMAND="$@"
TEMP_OE=/tmp/onramp-test.oe
TEMP_STDOUT=/tmp/onramp-test.stdout

for TESTFILE in $(find $SOURCE_FOLDER/* -name '*.oo'); do
    echo -n "Running $TESTFILE... "
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

    if [ $RET -eq 0 ]; then
        cp $TEMP_OE $BASENAME.oe
        echo -n "Generated $BASENAME.oe"
        rm -f $BASENAME.stdout
        onrampvm $TEMP_OE > $TEMP_STDOUT 2>/dev/null
        if [ $? -eq 0 ] && [ -s $TEMP_STDOUT ]; then
            cp $TEMP_STDOUT $BASENAME.stdout
            echo -n ", $BASENAME.stdout"
        fi
    else
        rm -f $BASENAME.oe
        rm -f $BASENAME.stdout
        echo -n "Failed, deleting $BASENAME.oe *****"
    fi

    echo
    rm -f $TEMFILE
done

# Run the test script to make sure we generated correctly
$(dirname $0)/run.sh $SOURCE_FOLDER $COMMAND

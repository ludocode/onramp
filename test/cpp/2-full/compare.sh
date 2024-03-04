#!/bin/bash

# This script tests the given preprocessor by running it against all .c files
# in all subfolders of this test folder recursively and comparing the output to
# that of the system compiler.
#
# The output of each compiler has blank lines eliminated and is fed through
# clang-format to ensure consistent formatting.

if ! which clang-format > /dev/null; then
    echo "clang-format is required."
fi
if [ "$1" == "" ]; then
    echo "Need command to test."
    exit 1
fi
if [ "$2" == "" ]; then
    echo "Need compiler to test against."
    exit 1
fi

COMMAND=$1
REFCC=$(realpath $(which $2))
ACTUAL=$(mktemp)
EXPECTED=$(mktemp)
ERROR=0

for TESTCASE in $(find $(dirname $0)* -name '*.c'); do

    # TODO for now we aren't supporting extensions
    case $TESTCASE in
        extensions/*)
            echo "Skipping $TESTCASE"
            continue
            ;;
        *) ;;
    esac

    ARGSFILE=$(echo $TESTCASE|sed 's/\.c$/.args/')
    ARGS=
    if [ -e $ARGSFILE ]; then
        # echo args to expand shell macros
        ARGS=$(echo $(cat $ARGSFILE))
    fi

    echo "Testing $TESTCASE"
    $COMMAND $ARGS -P $TESTCASE -o $ACTUAL.i &> /dev/null
    ACTUAL_RET=$?
    $REFCC $ARGS -E -P $TESTCASE -o $EXPECTED.i &> /dev/null
    EXPECTED_RET=$?

    if [ $ACTUAL_RET -ne 0 ]; then
        if [ $EXPECTED_RET -eq 0 ]; then
            echo "ERROR: $TESTCASE failed; expected success."
            echo "Commands:"
            echo "    $COMMAND $ARGS -P $TESTCASE -o $ACTUAL.i"
            echo "    $REFCC $ARGS -E -P $TESTCASE -o $EXPECTED.i"
            ERROR=1
        fi
    elif [ $EXPECTED_RET -ne 0 ]; then
        echo "ERROR: $TESTCASE succeeded; expected error."
        echo "Commands:"
        echo "    $COMMAND $ARGS -P $TESTCASE -o $ACTUAL.i"
        echo "    $REFCC $ARGS -E -P $TESTCASE -o $EXPECTED.i"
        ERROR=1
    else
        cat $ACTUAL.i | sed '/^ *$/d' | clang-format > $ACTUAL
        cat $EXPECTED.i | sed '/^ *$/d' | clang-format > $EXPECTED
        if ! diff -q $ACTUAL $EXPECTED > /dev/null; then
            echo "ERROR: $TESTCASE did not match system compiler"
            echo "Commands:"
            echo "    $COMMAND $ARGS -P $TESTCASE -o $ACTUAL.i"
            echo "    $REFCC $ARGS -E -P $TESTCASE -o $EXPECTED.i"
            ERROR=1
        fi
    fi

    rm -f {$ACTUAL,$EXPECTED}{,.i}
done

if [ $ERROR -eq 1 ]; then
    echo "Errors occurred."
    exit 1
fi

echo "Pass."

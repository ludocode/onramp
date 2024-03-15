#!/bin/bash

# This script tests the given cci compiler by running it against all .i and .c
# files in all subfolders of this test folder.
#
#     Usage: run.sh <test_folder> <compiler_id> <run_commands...>
#
# e.g.
#
#     test/cci/run.sh test/cci/0-omc omc onrampvm build/intermediate/cci-0-omc/cci.oe
#
# If a .c file exists, the preprocessor is run on it first with the libc/1
# headers, then the compiler is run on the output. If a .i file exists, the
# compiler is run on it directly.
#
# The program is then assembled and linked against libc/0
# TODO libc/1
# and run. If assembly, linking or execution fails, the test fails.
#
# - If a corresponding .os file exists, the compiler must succeed and its output
# must match the file's contents.
#
# - If no corresponding .os file exists, the compiler must return an error. Its
# output is ignored.
#
# - If a corresponding .args file exists, the contents are passed as
# command-line arguments to the compiler instead of the default arguments. Use
# $INPUT for the input filename and $OUTPUT for the output filename.
#
# - If a corresponding .stdout file exists, the program's output must match the
# contents.
#
# - If a corresponding .status file exists, the program must return with the
# given status code. Otherwise, the program must return with status 0
# (success.)
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
make -C $ROOT/test/cpp/1-omc/ build  # TODO cpp/2
make -C $ROOT/test/as/2-full/ build
make -C $ROOT/test/ld/2-full/ build
make -C $ROOT/test/libc/0-oo/ build  # TODO libc/1 not libc/0
if ! command -v onrampvm > /dev/null; then
    echo "ERROR: onrampvm is required on PATH."
    exit 1
fi

SOURCE_FOLDER="$1"
shift
COMPILER_ID="$1"
shift
COMMAND="$@"
TEMP_I=/tmp/onramp-test.i
TEMP_OS=/tmp/onramp-test.os
TEMP_OO=/tmp/onramp-test.oo
TEMP_OE=/tmp/onramp-test.oe
TEMP_STDOUT=/tmp/onramp-test.stdout
TOTAL_ERRORS=0

# we want address sanitizer to return the same error code as the vm so we can
# detect crashes on both
export ASAN_OPTIONS=exitcode=125

# determine macros to use for this compiler
if [ "$COMPILER_ID" = "omc" ]; then
    MACROS="-D__onramp_cci_omc__=1 -Drestrict= -D_Noreturn="
elif [ "$COMPILER_ID" = "opc" ]; then
    MACROS="-D__onramp_cci_opc__=1 -Dlong=int"
elif [ "$COMPILER_ID" = "full" ]; then
    MACROS="-D__onramp_cci_full__=1"
else
    echo "ERROR: Unknown compiler: $COMPILER_ID"
fi
MACROS="$MACROS -D__onramp__=1 -D__onramp_cci__=1"
# TODO use cpp/2
#MACROS="$MACROS -D__onramp_cpp__=1 -D__onramp_cpp_full__=1"
MACROS="$MACROS -D__onramp_cpp__=1 -D__onramp_cpp_omc__=1"
MACROS="$MACROS -include __onramp/__predef.h"

TESTS_PATH="$(basename $(realpath $SOURCE_FOLDER/..))/$(basename $(realpath $SOURCE_FOLDER))"
echo "Running $TESTS_PATH tests on: $COMMAND"

#FILES="$SOURCE_FOLDER/programs/eight-queens.c"
FILES="$(find $SOURCE_FOLDER/* -name '*.c') $(find $SOURCE_FOLDER/* -name '*.i')"

for TESTFILE in $FILES; do
    THIS_ERROR=0
    BASENAME=$(echo $TESTFILE|sed 's/\..$//')

    if [ $OTHER_STAGE -eq 1 ] && [ -e $BASENAME.nonstd ]; then
        echo "Skipping $BASENAME"
        continue
    fi

    echo "Testing $BASENAME"

    # preprocess
    if echo $TESTFILE | grep -q '\.c$'; then
        $ROOT/build/test/cpp-1-omc/cpp $MACROS \
            -I$ROOT/core/libc/0-oo/include -I$ROOT/core/libc/1-omc/include \
            $BASENAME.c -o $TEMP_I
        INPUT=$TEMP_I
    else
        INPUT=$BASENAME.i
    fi
    OUTPUT=$TEMP_OS

    # collect or generate the args
    if [ -e $BASENAME.args ]; then
        # eval echo to expand shell macros
        ARGS="$(eval echo $(cat $BASENAME.args))"
    else
        ARGS="$INPUT -o $OUTPUT"
    fi

    # compile
    set +e
    $COMMAND $ARGS &> /dev/null
    RET=$?
    set -e

    # check compile status and assembly
    if [ $RET -eq 125 ]; then
        echo "ERROR: compiler crashed on $BASENAME; expected success or error message."
        THIS_ERROR=1
    elif [ -e $BASENAME.os ]; then
        if [ $RET -ne 0 ]; then
            echo "ERROR: $BASENAME failed to compile; expected success."
            THIS_ERROR=1
        elif [ $OTHER_STAGE -eq 1 ]; then
            true # don't compare assembly
        elif ! diff -q $BASENAME.os $TEMP_OS > /dev/null; then
            echo "ERROR: $BASENAME did not match expected $BASENAME.os"
            THIS_ERROR=1
        fi
    else
        if [ $RET -eq 0 ]; then
            echo "ERROR: $BASENAME compilation succeeded; expected error."
            THIS_ERROR=1
        fi
    fi

    if [ -e $BASENAME.os ]; then

        # assemble, link and run
        if [ $THIS_ERROR -ne 1 ] && ! $ROOT/build/test/as-2-full/as $TEMP_OS -o $TEMP_OO &> /dev/null; then
            echo "ERROR: $BASENAME failed to assemble."
            THIS_ERROR=1
        fi
        if [ $THIS_ERROR -ne 1 ] && ! $ROOT/build/test/ld-2-full/ld \
                -g $ROOT/build/test/libc-0-oo/libc.oa $TEMP_OO -o $TEMP_OE &> /dev/null; then
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
        if echo $TESTFILE | grep -q '\.c$'; then
            echo "    $ROOT/build/test/cpp-1-omc/cpp $MACROS" \
                "-I$ROOT/core/libc/0-oo/include -I$ROOT/core/libc/1-omc/include" \
                "$BASENAME.c -o $TEMP_I && \\"
        fi
        echo "    $COMMAND $ARGS && \\"
        echo "    $ROOT/build/test/as-2-full/as $TEMP_OS -o $TEMP_OO && \\"
        echo "    $ROOT/build/test/ld-2-full/ld -g $ROOT/build/test/libc-0-oo/libc.oa $TEMP_OO -o $TEMP_OE && \\"
        echo "    onrampvm $TEMP_OE >$TEMP_STDOUT"
        if [ -e $BASENAME.stdout ]; then
            echo "    diff -q $BASENAME.stdout $TEMP_STDOUT"
        fi
        TOTAL_ERRORS=$(( $TOTAL_ERRORS + 1 ))
    fi

    # clean up
    rm -f $TEMP_I
    rm -f $TEMP_OS
    rm -f $TEMP_OO
    rm -f $TEMP_OE
    rm -f $TEMP_STDOUT
done

if [ $TOTAL_ERRORS -ne 0 ]; then
    echo "$TOTAL_ERRORS tests failed."
    exit 1
fi

echo "Pass."

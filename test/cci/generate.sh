#!/bin/bash

# This script generates .os files using the given compiler.
#
# Note that any bugs in the compiler may result in bugs in the generated
# tests.
#
# Use this only to generate new tests or to re-generate all tests in
# case of a change in the compiler output. In any case, review the diffs
# carefully to ensure it has not introduced any bugs.

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
make -C $ROOT/test/ld/0-global/ build
make -C $ROOT/test/libc/0-oo/ build  # TODO libc/1 not libc/0
if ! command -v onrampvm > /dev/null; then
    echo "ERROR: onrampvm is required on PATH."
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
ANY_ERROR=0

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

FILES="$(find $SOURCE_FOLDER/* -name '*.c') $(find $SOURCE_FOLDER/* -name '*.i')"

for TESTFILE in $FILES; do
    BASENAME=$(echo $TESTFILE|sed 's/\..$//')
    echo -n "Generating $BASENAME... "

    # preprocess
    if echo $TESTFILE | grep -q '\.c$'; then
        $ROOT/build/test/cpp-1-omc/cpp $MACROS \
            -I$ROOT/core/libc/0-oo/include -I$ROOT/core/libc/1-omc/include \
            -include __onramp/__predef.h \
            $BASENAME.c -o $TEMP_I
        INPUT=$TEMP_I
    else
        INPUT=$BASENAME.i
    fi
    OUTPUT=$TEMP_OS

    # collect or generate the args
    if [ -e $BASENAME.args ]; then
        # eval echo to expand shell macros
        ARGS=$(eval echo -- "\"$(cat $BASENAME.args)\"")
    else
        ARGS="$INPUT -o $OUTPUT"
    fi

    # compile
    set +e
    $COMMAND $ARGS &> /dev/null
    RET=$?
    set -e

    # handle assembly
    if [ $RET -eq 0 ]; then
        sed -e '/^ *$/d' -e '/^#/d' $TEMP_OS > $BASENAME.os
        echo "Generated $BASENAME.os"
    else
        rm -f $BASENAME.os
        echo "Failed, deleted $BASENAME.os *****"
    fi

    # TODO we don't current bother to generate the stdout.

    # clean up
    rm -f $TEMP_I
    rm -f $TEMP_OS
    rm -f $TEMP_OO
    rm -f $TEMP_OE
    rm -f $TEMP_STDOUT
done

# Run the test script to make sure we generated correctly
$(dirname $0)/run.sh $SOURCE_FOLDER $COMPILER_ID $COMMAND

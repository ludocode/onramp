#!/bin/bash

# This script tests the given preprocessor by running it against all .c files
# in all subfolders of the current folder.
#
#     Usage: run.sh . <run_commands...>
#
# - If a corresponding .fail file exists, the preprocessor must exit with an
#   error (and without crashing.) Otherwise, the preprocessor must succeed.
#
# - If a corresponding .i file exists, the preprocessor's output must match the
#   file's contents (including linemarkers if --strict is passed.) Otherwise,
#   the program is compiled and run. If compilation fails, the test fails.
#
# - If a corresponding .stdout file exists, the program's output must match the
#   contents.
#
# - If a corresponding .status file exists, the program must return with the
#   given status code. Otherwise, the program must return with status 0
#   (success.) (TODO this is deprecated; remove this.)
#
# - If a corresponding .args file exists, the contents are passed as
#   command-line arguments to the prepocessor instead of the default arguments.
#   Use $INPUT for the input filename and $OUTPUT for the output filename.
#
# - If a corresponding .skip file exists, the test is skipped.
#
# - If a corresponding .nonstd file exists and the --nonstd argument is not
#   provided, the test is skipped.
#
# TODO: Having the output match a corresponding .i file is deprecated. We don't
# want to store generated output in the repository for security reasons. (The
# .i output also uses deprecated GNU-style linemarkers.) Instead, all (or
# almost all) of the tests should be converted to be runnable programs and
# should exit with status 0 on success. In the meantime we can compare the
# output stripped.
#
# TODO we should have a special exit code so that we can differentiate between
# the preprocessor crashing as opposed to printing an error and exiting.
# TODO this is now done with cci, we make asan exit with code 125 which is the
# same code for a vm crash so we can detect crashes in both, need to do the
# same here

set -e
ROOT=$(dirname $0)/../..

NONSTD=0
STRICT=0
while true; do
    if [ "$1" == "--strict" ]; then
        STRICT=1
        shift
    elif [ "$1" == "--nonstd" ]; then
        NONSTD=1
        shift
    else
        break
    fi
done

# TODO the source folder needs to be the current folder to make paths in #line
# match, but we can't really switch to it because the command is in multiple
# parts and some may have relative paths. We should just not bother taking a
# source folder, instead the source folder should always just be '.', i.e. the
# working directory where the script is called
if [ "$1" != "." ]; then
    echo "First argument must be \`.\`."
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

SOURCE_FOLDER="$1"
shift
COMMAND="$@"
TEMP_I=/tmp/onramp-test.i
TEMP_I_EXPECTED=/tmp/onramp-test-expected.i
TEMP_I_ACTUAL=/tmp/onramp-test-actual.i
TEMP_OS=/tmp/onramp-test.os
TEMP_OO=/tmp/onramp-test.oo
TEMP_OE=/tmp/onramp-test.oe
TEMP_STDOUT=/tmp/onramp-test.stdout
TEMP_STDERR=/tmp/onramp-test.stderr
TEMP_FILES=/tmp/onramp-test-files
TOTAL_ERRORS=0

# build dependencies
make -C $ROOT/test/cci/2-full/ build
make -C $ROOT/test/as/2-full/ build
make -C $ROOT/test/ld/2-full/ build
make -C $ROOT/test/libc/3-full/ build

# we want address sanitizer to return the same error code as the vm so we can
# detect crashes on both
export ASAN_OPTIONS="$ASAN_OPTIONS:exitcode=125"

TESTS_PATH="$(basename $(realpath $SOURCE_FOLDER/..))/$(basename $(realpath $SOURCE_FOLDER))"
echo "Running $TESTS_PATH tests on: $COMMAND"

function clean() {
    IN=$1
    OUT=$2
    if [ $STRICT -eq 1 ]; then
        cp $IN $OUT
    else
        sed \
            -e '/^#/d' \
            -e '/^ *$/d' \
            -e 's/.*/ & /' \
            -e 's/  */ /' \
            $IN > $OUT
    fi
}

# Collect and sort file list
find $SOURCE_FOLDER/* -name '*.c' > $TEMP_FILES
FILES="$(cat $TEMP_FILES | sort)"
rm -f $TEMP_FILES

for TESTFILE in $(find $SOURCE_FOLDER/* -name '*.c'); do
    THIS_ERROR=0
    BASENAME=$(echo $TESTFILE|sed 's/\.c$//')

    if [ -e $BASENAME.skip ] || ( [ $NONSTD -eq 0 ] && [ -e $BASENAME.nonstd ] ); then
        echo "Skipping $BASENAME"
        continue
    fi

    echo "Testing $BASENAME"

    INPUT=$TESTFILE
    OUTPUT=$TEMP_I

    # collect or generate the args
    if [ -e $BASENAME.args ]; then
        # eval echo to expand shell macros
        ARGS=$(eval echo "\"$(cat $BASENAME.args)\"")
    else
        ARGS="$INPUT -o $OUTPUT"
    fi

    # preprocess
    set +e
    $COMMAND $ARGS 1> $TEMP_STDOUT 2> $TEMP_STDERR
    RET=$?
    set -e

    # check compile status
    if [ $RET -eq 125 ]; then
        echo "ERROR: preprocessor crashed on $BASENAME; expected success or error message."
        cat $TEMP_STDERR
        THIS_ERROR=1
    elif ! [ -e $BASENAME.fail ]; then
        if [ $RET -ne 0 ]; then
            echo "ERROR: $BASENAME failed; expected success."
            cat $TEMP_STDERR
            THIS_ERROR=1
        fi
    else
        if [ $RET -eq 0 ]; then
            echo "ERROR: $BASENAME succeeded; expected error."
            THIS_ERROR=1
        fi
    fi

    if [ $THIS_ERROR -ne 1 ] && ! [ -e $BASENAME.fail ]; then
        if [ -e $BASENAME.i ]; then

            # compare output
            clean $BASENAME.i $TEMP_I_EXPECTED
            clean $TEMP_I $TEMP_I_ACTUAL
            if ! diff -q $TEMP_I_EXPECTED $TEMP_I_ACTUAL > /dev/null; then
                echo "ERROR: $BASENAME did not match expected $BASENAME.i"
                THIS_ERROR=1
            fi

        else
            # compile, assemble, link and run
            if [ $THIS_ERROR -ne 1 ] && ! $ROOT/build/test/cci-2-full/cci $OUTPUT -o $TEMP_OS &> /dev/null; then
                echo "ERROR: $BASENAME failed to compile."
                THIS_ERROR=1
            fi
            if [ $THIS_ERROR -ne 1 ] && ! $ROOT/build/test/as-2-full/as $TEMP_OS -o $TEMP_OO &> /dev/null; then
                echo "ERROR: $BASENAME failed to assemble."
                THIS_ERROR=1
            fi
            if [ $THIS_ERROR -ne 1 ] && ! $ROOT/build/test/ld-2-full/ld \
                    -g $ROOT/build/test/libc-3-full/libc.oa $TEMP_OO -o $TEMP_OE &> /dev/null; then
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
    fi

    if [ $THIS_ERROR -eq 1 ]; then
        echo "Commands:"
        echo "    make build && \\"
        echo "    $COMMAND $ARGS && \\"
        if [ -e $BASENAME.fail ]; then
            echo "    cat $TEMP_I"
        elif [ -e $BASENAME.i ]; then
            echo "    diff -u $BASENAME.i $TEMP_I"
        else
            echo "    $ROOT/build/test/cci-2-full/cci $OUTPUT -o $TEMP_OS && \\"
            echo "    $ROOT/build/test/as-2-full/as $TEMP_OS -o $TEMP_OO && \\"
            echo "    $ROOT/build/test/ld-2-full/ld -g $ROOT/build/test/libc-3-full/libc.oa $TEMP_OO -o $TEMP_OE && \\"
            echo "    onrampvm $TEMP_OE"
        fi
        TOTAL_ERRORS=$(( $TOTAL_ERRORS + 1 ))
    fi

    # clean up
    rm -f $TEMP_I
    rm -f $TEMP_I_ACTUAL
    rm -f $TEMP_I_EXPECTED
    rm -f $TEMP_OS
    rm -f $TEMP_OO
    rm -f $TEMP_OE
    rm -f $TEMP_STDOUT
    rm -f $TEMP_STDERR
done

if [ $TOTAL_ERRORS -ne 0 ]; then
    echo "$TOTAL_ERRORS tests failed."
    exit 1
fi

echo "Pass."

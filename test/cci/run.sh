#!/bin/bash

# This script tests the given cci compiler by running it against all .i and .c
# files in all subfolders of a given test folder.
#
#     Usage: run.sh <test_folder> <compiler_id> <run_commands...>
#
# Options:
#
#     --tests <path>     Path to test cases to run
#     --cci <cci>        cci to use
#     --cci-id <id>      Identifier of cci bootstrapping stage (omc, opc or full)
#     --noskip           Don't skip tests marked skip
#     --nonstd           Run tests marked non-standard
#     --output <path>    Output intermediate files to the given path

set -e

# Make sure our working directory is correct
if [ "$(realpath "$(dirname $0)/../..")" != "$(realpath "$(pwd)")" ]; then
    echo "$0: ERROR: This script must be run from the root of the Onramp repository." >&2
    exit 1
fi

SOURCE_FOLDER=
OUTPUT_PATH=
NONSTD_ARG=
NOSKIP_ARG=
CCI=
CCI_ID=

# parse command-line options
set +e
while true; do
    ARG=$1
    if ! shift; then
        break
    fi
    case $ARG in
        --tests) SOURCE_FOLDER="$1"; shift ;;
        --output) OUTPUT_PATH="$1"; shift ;;
        --nonstd) NONSTD_ARG=--nonstd ;;
        --noskip) NOSKIP_ARG=--noskip ;;
        --cci-id) CCI_ID="$1"; shift ;;
        --cci) CCI="$1"; shift ;;
        *)
            echo "$0: ERROR: Invalid command-line argument: $ARG" >&2
            exit 1
            ;;
    esac
done
set -e

if [ "$SOURCE_FOLDER" == "" ]; then
    echo "Need folder to test."
    exit 1
fi
if [ "$CCI_ID" == "" ]; then
    echo "Need id of compiler."
    exit 1
fi
if [ "$CCI" == "" ]; then
    echo "Need cci tool to test."
    exit 1
fi
if [ "$OUTPUT_PATH" == "" ]; then
    echo "Need output path."
    exit 1
fi

# build dependencies
make -C test/cpp/2-full build
#make -C test/cci/0-omc build
#make -C test/cci/1-opc build
#make -C test/cci/2-full build
make -C test/as/2-full build
make -C test/ld/2-full build
make -C test/libc/3-full build

# Collect and sort file list
# Note: We support .i files only because we have test cases that test error
# handling on things that the preprocessor would otherwise break on (such as
# unclosed string and character literals.) Almost all tests are .c files.
TEMP_FILES=$OUTPUT_PATH/onramp-test-files
rm -f $TEMP_FILES
find $SOURCE_FOLDER/* -name '*.c' >> $TEMP_FILES
find $SOURCE_FOLDER/* -name '*.i' >> $TEMP_FILES
FILES="$(cat $TEMP_FILES | sort)"
rm -f $TEMP_FILES

TOTAL_ERRORS=0
for TESTFILE in $FILES; do
    TEST_OUTPUT="$OUTPUT_PATH/$(dirname $TESTFILE)"
    mkdir -p "$TEST_OUTPUT"
    COMMAND="$(dirname "$0")/test-case.sh \
        --test "$TESTFILE" \
        --output "$TEST_OUTPUT" \
        --cci "$CCI" \
        --cci-id "$CCI_ID" \
        $NONSTD_ARG \
        $NOSKIP_ARG"
    if ! $COMMAND ; then
        echo $COMMAND
        TOTAL_ERRORS=$(( $TOTAL_ERRORS + 1 ))
    fi
done

if [ $TOTAL_ERRORS -ne 0 ]; then
    echo "$TOTAL_ERRORS tests failed."
    exit 1
fi

echo "Pass."

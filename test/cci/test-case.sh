#!/bin/bash

# The MIT License (MIT)
#
# Copyright (c) 2025 Fraser Heavy Software
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.



# This script runs a test case in C. It is designed to test stages of the cci
# and cg tools.
#
# This is not intended to be run directly. It is currently run by the separate
# run.sh tool.
#
# Rather than using the driver, each tool is run separately. This ensures that
# tests that are expected to fail only fail in the expected tool, not in any
# other tool. This also allows us to specify custom options for different tools
# (for example, cci can be run with -O and cg can be run without, or vice
# versa.) It also makes it easier to distinguish failures from crashes: if a
# test is expected to fail, it must give a nice error message, not crash.



# Command-Line Arguments
#
# Components:
#
# --cpp <cpp>        cpp to use; default native cpp/2
# --cci <cci>        cci to use; default native cci/2
# --cg <cg>          cg to use; default native cg/1 or none if --cci-id is not full
# --as <as>          as to use; default native as/2
# --ld <ld>          ld to use; default native ld/2
# --libc <libc>      libc to use; default libc/3
# --cpp-id <id>      Identifier of cpp bootstrapping stage (strip, omc or full)
# --cci-id <id>      Identifier of cci bootstrapping stage (omc, opc or full)
#
# Additional arguments:
#
# --test <test.c>    Test case to run
# --output <path/>   Output intermediate files to the given path
# --cpp-opts <opts>  Pass extra arguments to cpp (e.g. -D__onramp_cci_omc=1)
# --cci-opts <opts>  Pass extra arguments to cci (e.g. -O)
# --cg-opts <opts>   Pass extra arguments to cg (e.g. -O)
# --nonstd           Run the test even if it is marked non-standard
# --noskip           Run the test even if it is marked skip
# --noclean          Don't delete intermediate files after passing.
# -v, --verbose      Print commands, print all output
#
# The cg tool is optional, but note that testing of cci/2 requires (or will
# soon require) cg/1 to translate its IR.
#
# If a tool ends in `.oe`, it is run with the `onrampvm` on your `PATH`.
# Otherwise it is run natively.
#
# The --cci-id and --cpp-id options are necessary to pass the correct macros to
# the preprocessor. (Bootstrapping stages cannot parse the full libc headers so
# macros disable parts that are not supported.)
#
# When testing cci/0 and cci/1, usually a cg tool is not provided to speed up
# testing and to ensure their unoptimized assembly output is valid. cg/0 is
# tested using this script with cci/0 and cci/1.
#
# When testing cci/2, cg/1 is used without optimization, and when testing cg/1,
# it is used with optimization. (TODO currently it is not used at all because
# it doesn't exist yet.)



# Test case format:
#
# The test case can have the extension `.c` or `.i`. The extension determines
# which phases of translation are run. If compilation is not expected to fail,
# the resulting program is run.
#
        # A test directive is a comment that starts with "//% " in C files or "#% " in
        # lower-level Onramp files.
# A test directive is a comment that starts with "//% ". It provides details on
# how the test is to be run by the test harness.
#
# Test files can contain the following directives:
#
# //% FAIL <tool>          The given tool must fail and report an error (and not crash.)
# //% STATUS <num>         The test must run and exit with the given status.
# //% SKIP                 The test is skipped unless --noskip is given
# //% NON_STANDARD         The test is skipped unless --nonstd is given
# //% ARGS <tool> <args>   Command-line arguments to pass to the given tool
# //% STDOUT "output"      Output must match given string
#
# If STDOUT is given, the string is passed to `printf(1)`. The output of
# running the program must exactly match it.
#
# Legacy `.skip`, `.fail`, `.args`, `.status` and `,output` files are still
# supported for backwards compatibility.



set -e

# Make sure our working directory is correct
if [ "$(realpath "$(dirname $0)/../..")" != "$(realpath "$(pwd)")" ]; then
    echo "$0: ERROR: This script must be run from the root of the Onramp repository." >&2
    exit 1
fi

# We want Address Sanitizer to return the same error code as the VM so we can
# detect crashes on both. An error code of 125 indicates a crash.
export ASAN_OPTIONS="$ASAN_OPTIONS:exitcode=125"



###########################
# Options
###########################

CPP=output/test/cpp-2-full/cpp
CCI=output/test/cci-2-full/cci
CG= #no default for now, eventually this will be cg/1
AS=output/test/as-2-full/as
LD=output/test/ld-2-full/ld
LIBC=output/test/libc-3-full/libc.oa
CPP_ID=full
CCI_ID=full
CC=
RUN_NONSTD=0
RUN_SKIP=0
OUTPUT_PATH=
TESTFILE=
CLEAN=1
VERBOSE=0

# Parse command-line options
set +e
while true; do
    ARG=$1
    if ! shift; then
        break
    fi
    case $ARG in
        --cpp) CPP="$1"; shift ;;
        --cci) CCI="$1"; shift ;;
        --cg) CG="$1"; shift ;;
        --as) AS="$1"; shift ;;
        --ld) LD="$1"; shift ;;
        --libc) LIBC="$1"; shift ;;
        --cpp-id) CPP_ID="$1"; shift ;;
        --cci-id) CCI_ID="$1"; shift ;;
        --output) OUTPUT_PATH="$1"; shift ;;
        --test) TESTFILE="$1"; shift ;;
        --nonstd) RUN_NONSTD=1 ;;
        --noclean) CLEAN=0 ;;
        -v) VERBOSE=1 ;;
        --verbose) VERBOSE=1 ;;
        *)
            echo "$0: ERROR: Invalid command-line argument: $ARG" >&2
            exit 1
            ;;
    esac
done
set -e

# Make sure arguments are reasonable
if [ "$TESTFILE" = "" ]; then
    echo "$0: ERROR: The test to run must be provided with the --test option." >&2
    exit 1
fi
if [ "$CCI" = "" ]; then
    echo "$0: ERROR: A cci command must be provided with the --cci option." >&2
    exit 1
fi

BASENAME="$(basename "$TESTFILE")"
BASEPATH=$(echo "$TESTFILE"|sed 's/\.[^.]$//')



###########################
# Directives
###########################

# Defaults
LINE=0
FAIL=0
STATUS=
IS_SKIP=0
IS_NONSTD=0
HAS_STDOUT=0
ARGS='$INPUT -o $OUTPUT'
EXPECTED_STDOUT=
EXPECTED_STDOUT_STRING=

# Check for //% comments
while read COMMENT DIRECTIVE VALUE; do
    LINE=$(($LINE + 1))
    if [ "$COMMENT" != "//%" ]; then
        continue
    fi
    case $DIRECTIVE in
        FAIL) FAIL=1 ;;
        STATUS) STATUS="$VALUE" ;;
        SKIP) IS_SKIP=1 ;;
        NON_STANDARD) IS_NONSTD=1 ;;
        ARGS) ARGS="$VALUE" ;;
        STDOUT)
            HAS_STDOUT=1;
            # bit of a hack to avoid word splitting. pull out and decode the
            # quoted string
            EXPECTED_STDOUT_STRING="$(
                    printf "$(grep '//% *STDOUT' "$TESTFILE" \
                        | grep -o '".*"' \
                        | sed -e 's/^"//' -e 's/"$//' )" )"
            ;;
        *)
            echo "$0: ERROR: Invalid directive on line $LINE: //% $DIRECTIVE" >&2
            exit 1
            ;;
    esac
done <"$TESTFILE"

# Check for deprecated files
if [ -e $BASEPATH.fail ]; then
    FAIL=1;
fi
if [ -e $BASEPATH.status ]; then
    STATUS="$(cat $BASEPATH.status)"
fi
if [ -e $BASEPATH.args ]; then
    ARGS="$(cat $BASEPATH.args)"
fi
if [ -e $BASEPATH.nonstd ]; then
    IS_NONSTD=1
fi
if [ -e $BASEPATH.skip ]; then
    IS_SKIP=1
fi
if [ -e $BASEPATH.stdout ]; then
    HAS_STDOUT=1
    EXPECTED_STDOUT=$BASEPATH.stdout
fi

# Check if we should skip the test
if [ $RUN_SKIP -eq 0 ] && [ $IS_SKIP -eq 1 ]; then
    echo "Skipping $BASEPATH"
    exit 0
fi
if [ $RUN_NONSTD -eq 0 ] && [ $IS_NONSTD -eq 1 ]; then
    echo "Skipping non-standard $BASEPATH"
    exit 0
fi

echo "Testing $BASEPATH"



###########################
# Miscellaneous
###########################

# Make sure we have a VM
if ! command -v onrampvm > /dev/null; then
    echo "ERROR: onrampvm is required on PATH."
    exit 1
fi

# Make sure we have a directory for temporary files
OUTPUT_PATH_TO_DELETE=
clean_temp_dir() {
    if [ "$OUTPUT_PATH_TO_DELETE" != "" ]; then
        rm -rf "$OUTPUT_PATH_TO_DELETE"
    fi
}
if [ "$OUTPUT_PATH" = "" ]; then
    trap clean_temp_dir EXIT
    OUTPUT_PATH_TO_DELETE="$(mktemp -d)"
    if [ -z "$OUTPUT_PATH_TO_DELETE" ]; then
        echo "$0: ERROR: Failed to create a temporary directory." >&2
        exit 1
    fi
    OUTPUT_PATH="$OUTPUT_PATH_TO_DELETE"
fi

# Determine macros to use for this compiler
if [ "$CCI_ID" = "omc" ]; then
    MACROS="-D__onramp_cci_omc__=1"
elif [ "$CCI_ID" = "opc" ]; then
    MACROS="-D__onramp_cci_opc__=1"
fi
MACROS="$MACROS -Icore/libc/common/include"
MACROS="$MACROS -D__onramp__=1 -D__onramp_cci__=1"
MACROS="$MACROS -D__onramp_cpp__=1"
MACROS="$MACROS -include __onramp/__predef.h"
if [ "$CPP_ID" = "strip" ]; then
    # the first stage preprocessor does not support macros
    MACROS=
fi

# Run tools with onrampvm if they have .oe extension
case "$CPP" in *.oe) CPP_PREFIX=onrampvm ;; *) CPP_PREFIX= ;; esac
case "$CCI" in *.oe) CCI_PREFIX=onrampvm ;; *) CCI_PREFIX= ;; esac
case "$CG" in *.oe) CG_PREFIX=onrampvm ;; *) CG_PREFIX= ;; esac
case "$AS" in *.oe) AS_PREFIX=onrampvm ;; *) AS_PREFIX= ;; esac
case "$LD" in *.oe) LD_PREFIX=onrampvm ;; *) LD_PREFIX= ;; esac

# Write our stdout string to a file so we can diff it
TEMP_EXPECTED_STDOUT=
if [ "$EXPECTED_STDOUT_STRING" != "" ]; then
    TEMP_EXPECTED_STDOUT=$OUTPUT_PATH/$BASENAME.expected-stdout
    echo "$EXPECTED_STDOUT_STRING" > $TEMP_EXPECTED_STDOUT
    EXPECTED_STDOUT=$TEMP_EXPECTED_STDOUT
fi



###########################
# Compile the test
###########################

TOOL_LOG="$OUTPUT_PATH/$BASENAME.tool-log"
rm -f "$TOOL_LOG"
mkdir -p "$(dirname "$TOOL_LOG")"
touch "$TOOL_LOG"

# Preprocess (if not .i)
CPP_OUTPUT=$OUTPUT_PATH/$BASENAME.i
INPUT="$TESTFILE"
if echo "$TESTFILE" | grep -q '\.c$'; then
    OUTPUT=$CPP_OUTPUT
    COMMAND="$CPP_PREFIX $CPP $MACROS $INPUT -o $OUTPUT"
    set +e
    if [ $VERBOSE -eq 1 ]; then
        echo Running preprocessor: $COMMAND
        $COMMAND
    else
        $COMMAND &> $TOOL_LOG
    fi
    set -e
    if [ $? -ne 0 ]; then
        echo "$0: ERROR: Preprocessing failed." >&2
        cat $TOOL_LOG
        exit 1
    fi
    INPUT="$OUTPUT"
elif echo $TESTFILE | grep -q '\.i$'; then
    INPUT="$TESTFILE"
else
    echo "$0: ERROR: Unrecognized test file extension: $TESTFILE" >&2
    exit 1
fi

# Compile
if [ "$CG" != "" ]; then
    CCI_OUTPUT="$OUTPUT_PATH/$BASENAME.oir"
else
    CCI_OUTPUT="$OUTPUT_PATH/$BASENAME.os"
fi
OUTPUT=$CCI_OUTPUT
set +e
COMMAND="$CCI_PREFIX $CCI $(eval echo $ARGS)"
if [ $VERBOSE -eq 1 ]; then
    echo Running compiler: $COMMAND
    $COMMAND
else
    $COMMAND &> $TOOL_LOG
fi
RET=$?
set -e

# Check compile status
if [ $RET -eq 125 ]; then
    echo "$0: ERROR: Compiler crashed on: $TESTFILE" >&2
    cat $TOOL_LOG >&2
    exit 1
fi
if [ $FAIL -eq 0 ]; then
    if [ $RET -ne 0 ]; then
        echo "$0: ERROR: Compiler failed; expected success on: $TESTFILE" >&2
        cat $TOOL_LOG >&2
        exit 1
    fi
else
    if [ $RET -eq 0 ]; then
        echo "$0: ERROR: Compiler succeeded; expected failure on: $TESTFILE" >&2
        cat $TOOL_LOG >&2
        exit 1
    fi
    # Failure is expected here. The test passes.
    rm -f $CPP_OUTPUT $CCI_OUTPUT $TOOL_LOG
    exit 0
fi

# Codegen
CG_OUTPUT=
if [ "$CG" != "" ]; then
    CG_OUTPUT=$OUTPUT_PATH/$BASENAME.os
    INPUT=$OUTPUT
    OUTPUT=$CG_OUTPUT
    COMMAND="$CG_PREFIX $CG $INPUT -o $OUTPUT"
    if [ $VERBOSE -eq 1 ]; then
        echo Running code generator: $COMMAND
        $COMMAND
    else
        $COMMAND &> $TOOL_LOG
    fi
    if [ $? -ne 0 ]; then
        echo "$0: ERROR: Failed to codegen: $TESTFILE" >&2
        cat $TOOL_LOG >&2
        exit 1
    fi
fi

# Assemble
AS_OUTPUT=$OUTPUT_PATH/$BASENAME.oo
INPUT=$OUTPUT
OUTPUT=$AS_OUTPUT
COMMAND="$AS_PREFIX $AS $INPUT -o $OUTPUT"
if [ $VERBOSE -eq 1 ]; then
    echo Running assembler: $COMMAND
    $COMMAND
else
    $COMMAND &> $TOOL_LOG
fi
if [ $? -ne 0 ]; then
    echo "$0: ERROR: Failed to assemble: $TESTFILE" >&2
    cat $TOOL_LOG >&2
    exit 1
fi

# Link
LD_OUTPUT=$OUTPUT_PATH/$BASENAME.oe
INPUT=$OUTPUT
OUTPUT=$LD_OUTPUT
COMMAND="$LD_PREFIX $LD -g $LIBC $INPUT -o $OUTPUT"
if [ $VERBOSE -eq 1 ]; then
    echo Running linker: $COMMAND
    $COMMAND
else
    $COMMAND &> $TOOL_LOG
fi
if [ $? -ne 0 ]; then
    echo "$0: ERROR: Failed to link: $TESTFILE" >&2
    echo $LD_PREFIX $LD -g $LIBC $INPUT -o $OUTPUT >&2
    cat $TOOL_LOG >&2
    exit 1
fi



###########################
# Run the test
###########################

# Run it
ACTUAL_STDOUT=$OUTPUT_PATH/$BASENAME.actual-stdout
ACTUAL_STDERR=$OUTPUT_PATH/$BASENAME.actual-stderr
rm -f "$ACTUAL_STDOUT" "$ACTUAL_STDERR"
set +e
if [ $VERBOSE -eq 1 ]; then
    echo Running program: onrampvm $OUTPUT
    onrampvm $OUTPUT >"$ACTUAL_STDOUT"
    RET=$?
    cat "$ACTUAL_STDOUT"
else
    onrampvm $OUTPUT >"$ACTUAL_STDOUT" 2>"$ACTUAL_STDERR"
    RET=$?
fi
set -e

# Check status
if [ "$STATUS" != "" ]; then
    if [ $RET -ne $STATUS ]; then
        echo "$0: ERROR: Expected exit status $STATUS, got $RET running: $TESTFILE" >&2
        exit 1
    fi
else
    if [ $RET -ne 0 ]; then
        echo "$0: ERROR: Expected success, got exit status $RET running: $TESTFILE" >&2
        exit 1
    fi
fi

# Check stdout
if [ $HAS_STDOUT -ne 0 ]; then
    if ! diff -q $ACTUAL_STDOUT $EXPECTED_STDOUT >/dev/null; then
        echo "$0: ERROR: stdout did not match: $TESTFILE" >&2
        exit 1
    fi
fi

# Success. Clean up all temp files.
# (We don't clean up temp files on failure to make it easier to debug. We also
# support --noclean to keep files on a pass.)
if [ $CLEAN -ne 0 ]; then
    rm -f \
        $TOOL_LOG \
        $CPP_OUTPUT \
        $CCI_OUTPUT \
        $CG_OUTPUT \
        $AS_OUTPUT \
        $LD_OUTPUT $LD_OUTPUT.od \
        $ACTUAL_STDERR $ACTUAL_STDOUT $TEMP_EXPECTED_STDOUT
fi
exit 0

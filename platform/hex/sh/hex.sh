#!/bin/sh

# Copyright (c) 2023-2024 Fraser Heavy Software
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

########################################################################

# This is a pure POSIX shell implementation of the Onramp hex tool.
#
# It implements address assertions and almost all other optional error checking
# and provides good error messages with line numbers for most errors. It passes
# all tests except backslashes at the end of lines (because it is not possible
# to detect them with pure POSIX `read` which is why the restriction exists.)
#
# When run in bash, dash, zsh and BusyBox, it uses only shell builtins. Some of
# its commands, in particular `read` and `printf`, may be separate tools on
# other platforms but they are all in POSIX.
#
# This tool requires advanced POSIX shell features including arithmetic
# expressions (i.e. $((foo + bar))) and substring parameter expansion (i.e.
# ${foo#bar}). If your shell does not have these features you could try the
# sh-alt/ implementation instead.

set -e

# Check for too many arguments
if ! [ -z "$4" ]; then
    echo "ERROR: Wrong number of arguments." >&2
    echo "Usage: $0 <input_file> -o <output_file>" >&2
    exit 1
fi

# Allow -o before and after the input file
if [ "$1" = "-o" ]; then
    output="$2"
    input="$3"
elif [ "$2" = "-o" ]; then
    input="$1"
    output="$3"
else
    echo "ERROR: Invalid arguments." >&2
    echo "Usage: $0 <input_file> -o <output_file>" >&2
    exit 1
fi

# Make sure both files were provided
if [ "$input" = "" ]; then
    echo "ERROR: Input file required." >&2
    echo "Usage: $0 <input_file> -o <output_file>" >&2
    exit 1
fi
if [ "$output" = "" ]; then
    echo "ERROR: Output file required." >&2
    echo "Usage: $0 <input_file> -o <output_file>" >&2
    exit 1
fi

# Make sure the input file exists
if ! [ -e "$input" ]; then
    echo "ERROR: Input file does not exist: $input" >&2
    exit 1
fi

# Prepare the output
rm -f "$output" 2>/dev/null || true
if ! touch "$output"; then
    echo "ERROR: Failed to create output file." >&2
    exit 1
fi

address=0
linenum=0

# Collect our escape characters. ANSI C quoting isn't in POSIX so we use printf
# instead.
CHAR_T="$(printf '\011')"  # $'\t' horizontal tab
CHAR_N="$(printf '\012')"  # $'\n' line feed
CHAR_V="$(printf '\013')"  # $'\v' vertical tab
CHAR_F="$(printf '\014')"  # $'\f' form feed
CHAR_R="$(printf '\015')"  # $'\r' carriage return

# This `read` will escape backslash newline. We don't want it to do that but
# the `-r` option to disable it is not portable. This is the reason backslashes
# at the end of lines are forbidden in our hex language.
# We also check for a line with [ -n "$line" ] because `read line` returns
# false on a final line without a trailing newline. (see tests/truncated.ohx)
while IFS= read line || [ -n "$line" ]; do

    linenum=$((linenum+1))
    byte=""
    while [ -n "$line" ]; do

        # pop first character
        rest="${line#?}"
        first="${line%"$rest"}"
        line="$rest"

        # figure out what it is
        token=u
        case "$first" in
            0) token=char ;; 1) token=char ;; 2) token=char ;; 3) token=char ;;
            4) token=char ;; 5) token=char ;; 6) token=char ;; 7) token=char ;;
            8) token=char ;; 9) token=char ;; A) token=char ;; B) token=char ;;
            C) token=char ;; D) token=char ;; E) token=char ;; F) token=char ;;
            a) token=char ;; b) token=char ;; c) token=char ;; d) token=char ;;
            e) token=char ;; f) token=char ;;

            \#) token=comment ;;
            \;) token=comment ;;

            \@) token=assertion ;;

            " ")       token=whitespace ;;  # space
            "$CHAR_T") token=whitespace ;;  # horizontal tab
            "$CHAR_N") token=whitespace ;;  # line feed
            "$CHAR_V") token=whitespace ;;  # vertical tab
            "$CHAR_F") token=whitespace ;;  # form feed
            "$CHAR_R") token=whitespace ;;  # carriage return

            *)
                echo "ERROR: Invalid character on line $linenum: \"$first\"" >&2
                exit 1
                ;;
        esac

        # handle it
        case "$token" in
            char)
                byte=$byte$first
                ;;

            comment)
                line=""
                ;;

            assertion)
                value=${rest%% *}
                prefix=${value%${value##0x}}
                if [ $prefix != "0x" ] && [ $prefix != "0X" ]; then
                    echo "ERROR: Address assertion does not start with 0x on line $linenum" >&2
                    exit 1
                fi
                if [ ${#value} -le 2 ]; then
                    echo "ERROR: Address assertion truncated on line $linenum" >&2
                    exit 1
                fi
                if [ ${#value} -gt 10 ]; then
                    echo "ERROR: Address assertion too long on line $linenum" >&2
                    exit 1
                fi
                if [ $(($value)) -ne $address ]; then
                    # TODO convert to hex, show expected, match c syntax
                    echo "ERROR: Address assertion failed on line $linenum: declared $value which is $(($value - $address)) from actual address" >&2
                    exit 1
                fi
                line=""
                ;;

            whitespace)
                # make sure a space isn't interrupting our byte
                if [ "$byte" != "" ]; then
                    echo "ERROR: Incomplete byte on line $linenum: \"$byte\"" >&2
                    exit 1
                fi
                ;;
        esac

        # if the byte is complete, output it
        if [ ${#byte} -eq 2 ]; then
            printf \\$(printf %o $(( 0x$byte ))) >> $output
            byte=

            # update the current address
            address=$(($address + 1))
        fi
    done
done < "$input"

if [ "$byte" != "" ]; then
    echo "ERROR: Incomplete byte at end of file: \"$byte\"" >&2
    exit 1
fi

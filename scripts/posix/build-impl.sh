#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2026 Fraser Heavy Software
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


# This script is the implementation of the build.sh generated from the
# configure script.
#
# This is probably way overkill and should be simplified a lot. We don't
# actually want people passing arguments to this script.


set -e
BUILD="$(pwd)"
ROOT="$(dirname "$0")/../.."

usage() {
    cat <<EOF

Options:

    --test           Run all tests during bootstrap process

The --test option is implied if it was passed to configure.

See the Setup Guide for details:
    docs/setup-guide.md
EOF
}

parse_options() {
    TEST=0

    while [ $# -ne 0 ]; do
        case "$1" in
            --test) TEST=1; shift ;;
            --help) usage ; exit 0 ;;

            *)
                echo "ERROR: Invalid option: $1"
                usage
                exit 1
                ;;
        esac
    done
}

check_configure() {
    FILES="
        output/posix/bin/onrampvm
        output/posix/bin/onramphex
        output/posix/share/onramp/platform/wrap-header
        output/intermediate/sh/sh.oe
        output/intermediate/hex-0-onramp/hex.oe
    "
    for FILE in $FILES; do
        if ! [ -e $FILE ]; then
            echo "ERROR: Missing file: $FILE"
            echo "Did you run configure?"
            exit 1
        fi
    done
}

build() {
    if [ $TEST -eq 1 ]; then
        . scripts/posix/env.sh
        test/test-bootstrap.sh
    else
        output/posix/bin/onrampvm output/intermediate/sh/sh.oe core/build.sh
        output/posix/bin/onrampvm output/intermediate/sh/sh.oe extra/build.sh
    fi
}

go() {
    parse_options "$@"
    check_configure
    build
    echo
    echo 'Build complete. Run `./install.sh` to install Onramp.'
}

go "$@"

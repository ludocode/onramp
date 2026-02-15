#!/bin/false

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
# It is meant to be sourced so it is not executable.


set -e
BUILD="$(pwd)"

usage() {
    cat <<EOF

Usage:

    $0 [options]

Options:

    --clean         Don't build; delete all built files
    --help          Print this help

See the Setup Guide for details:
    docs/setup-guide.md
EOF
}

parse_options() {
    CLEAN=0

    while [ $# -ne 0 ]; do
        case "$1" in
            --clean) CLEAN=1; shift ;;
            --help) usage ; exit 0 ;;
        esac
    done
}

check_configure() {
    FILES="
        output/posix/bin/onrampvm
        output/posix/share/onramp/platform/wrap-header
        output/configure/sh/sh.oe
        output/configure/hex-0-onramp/hex.oe
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
    if [ $NATIVE -eq 1 ]; then
        ninja
    elif [ $TEST -eq 1 ]; then
        . $ROOT/scripts/posix/env.sh
        $ROOT/test/test-bootstrap.sh
    else
        output/posix/bin/onrampvm output/configure/sh/sh.oe core/build.sh
        output/posix/bin/onrampvm output/configure/sh/sh.oe extra/build.sh
    fi
}

clean() {
    # TODO this doesn't work, need to move sh and hex/0
    echo rm -rf output/intermediate output/final output/test
    rm -rf output/intermediate output/final output/test
}

go() {
    parse_options "$@"

    if [ $CLEAN -eq 1 ]; then
        clean
        exit 0
    fi

    check_configure
    build
    echo
    echo 'Build complete.'
    echo 'Run `./install.sh` to install Onramp (or `. env.sh` to use it in-place.)'
}

go "$@"

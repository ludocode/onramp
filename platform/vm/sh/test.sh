#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2023-2026 Fraser Heavy Software
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


# This script tests the POSIX shell VM.


set -e
cd "$(dirname "$0")/../../.."

trap "exit 1" INT

# It takes a while to run the tests so we search for a shell roughly in order
# of speed, fastest to slowest. To run with a particular shell, just run the
# test script manually. For example:
#
#     test/vm/run.sh dash platform/vm/sh/vm.sh

try_shell() {
    if command -v $1 >/dev/null 2>&1 && $@ -c true >/dev/null 2>&1; then
        exec test/vm/run.sh $@ platform/vm/sh/vm.sh
    fi
}

# sorted roughly in order of speed, fastest to slowest
try_shell dash
try_shell busybox ash
try_shell ksh
try_shell bash
try_shell oksh
try_shell mksh
try_shell osh
try_shell yash
try_shell zsh
try_shell busybox hush

# nsh doesn't work:
# - no support for shifts in arithmetic expansions
# - no support for additional arguments when passing script argument to nsh
#try_shell nsh

# posh doesn't work:
# - no support for reading variables by name (without $) in arithmetic expressions
# - no support for compound assignment operators (e.g. +=, &=)
#try_shell posh

# toybox sh is incomplete:
# - elif is broken: https://github.com/landley/toybox/issues/460
#try_shell toybox sh

# If we haven't found a shell by now, just run with the default shell.
exec test/vm/run.sh platform/vm/sh/vm.sh

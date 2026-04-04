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


# This script tests the POSIX shell VM on a variety of shells.
#
# All of these shells must be installed for the test to pass. If you don't want
# to install one of these shells you can comment it out below.


set -e
cd "$(dirname "$0")/../../.."

test_shell() {
    echo
    echo "Testing with shell: $@"
    test/vm/run.sh $@ platform/vm/sh/vm.sh
}

test_shell ksh
test_shell busybox ash
test_shell dash
test_shell zsh
test_shell busybox hush
test_shell bash
test_shell osh
test_shell oksh
#test_shell nsh           # doesn't work, bugs in arithmetic expansions
#test_shell yash          # doesn't work, haven't debugged why
#test_shell toybox sh     # doesn't work, see e.g. https://github.com/landley/toybox/issues/460

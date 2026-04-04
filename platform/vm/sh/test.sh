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

# The times below are from the last time I measured the tests. Most of the time
# is taken by the eight queens test.
try_shell busybox ash   # 4m55s
try_shell dash          # 6m7s
try_shell zsh           # 12m48s
try_shell busybox hush  # 14m15s
try_shell bash          # 17m35s
try_shell oksh          # 24m21s
#try_shell yash          # doesn't work, haven't debugged why
#try_shell toybox sh     # doesn't work, see e.g. https://github.com/landley/toybox/issues/460
# still need to test: osh, nsh, yash

# If we haven't found a shell by now, just run with the default shell.
exec test/vm/run.sh platform/vm/sh/vm.sh

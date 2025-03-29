#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2024 Fraser Heavy Software
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



# Runs all platform-specific tests that can run on the current platform.
#
# Any tools that are not supported on the current platform are skipped. This
# script assumes only that a POSIX shell is available (since the script is
# running in it.)



set -e
cd "$(dirname "$0")/.."



################
# hex
################

# NOTE: We don't test c89/ or onramp/ here. They're tested as part of
# test-core.sh .

# python
if command -v python >/dev/null; then
    test/hex/run.sh platform/hex/python/hex.py
fi

# python-golf
if command -v python >/dev/null; then
    test/hex/run.sh --lax platform/hex/python-golf/hex.py
fi

# sh
test/hex/run.sh --lax platform/hex/sh/hex.sh
if command -v busybox >/dev/null; then
    busybox sh test/hex/run.sh --lax 'busybox sh platform/hex/sh/hex.sh'
fi

# sh-alt
test/hex/run.sh --lax platform/hex/sh-alt/hexcomb.sh
test/hex/run.sh --lax platform/hex/sh-alt/hexsemi.sh
test/hex/run.sh --lax platform/hex/sh-alt/hexsplit.sh

# x86_64-linux
if [ "$(uname -s)" = "Linux" ] && [ "$(uname -m)" = "x86_64" ]; then
    platform/hex/x86_64-linux/test.sh
fi

# xxd
if command -v xxd >/dev/null; then
    test/hex/run.sh --lax platform/hex/xxd/hex.sh
fi



################
# vm
################

# c-debugger
if command -v cc >/dev/null; then
    platform/vm/c-debugger/test.sh
fi

# x86_64-linux
if [ "$(uname -s)" = "Linux" ] && [ "$(uname -m)" = "x86_64" ]; then
    platform/vm/x86_64-linux/test.sh
fi

# python
if command -v python >/dev/null; then
    platform/vm/python/test.sh
fi

# c89
if command -v cc >/dev/null; then
    platform/vm/c89/test.sh
fi

# sh
# TODO disabled for now, it's too slow
#test/vm/run.sh platform/vm/sh/vm.sh

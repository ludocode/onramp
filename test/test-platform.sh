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
# (For now we assume the current platform is x86_64 Linux.)



set -e
cd "$(dirname "$0")/.."

# Setup for a POSIX environment
#scripts/posix/setup.sh
. scripts/posix/env.sh



################
# hex
################

# c89
( cd platform/hex/c89 && make )

# onramp
mkdir -p build/test/hex-onramp
build/test/hex-c89/hex platform/hex/onramp/hex.oe.ohx -o build/test/hex-onramp/hex.oe
test/hex/run.sh onrampvm build/test/hex-onramp/hex.oe

# python
test/hex/run.sh platform/hex/python/hex.py

# python-golf
test/hex/run.sh --lax platform/hex/python-golf/hex.py

# sh
test/hex/run.sh --lax platform/hex/sh/hex.sh

# sh-alt
test/hex/run.sh --lax platform/hex/sh-alt/hexcomb.sh
test/hex/run.sh --lax platform/hex/sh-alt/hexsemi.sh
test/hex/run.sh --lax platform/hex/sh-alt/hexsplit.sh

# x86_64-linux
mkdir -p build/test/hex-x86_64-linux
build/test/hex-c89/hex platform/hex/x86_64-linux/hex.ohx -o build/test/hex-x86_64-linux/hex
chmod +x build/test/hex-x86_64-linux/hex
test/hex/run.sh --lax build/test/hex-x86_64-linux/hex

# xxd
test/hex/run.sh --lax platform/hex/xxd/hex.sh



################
# vm
################

# c-debugger
( cd platform/vm/c-debugger && make )

# c89
mkdir -p build/test/vm-c89
# TODO disabled for now, buggy, needs cleanup
#cc -Wall -Wextra -Werror -Wpedantic -std=c89 platform/vm/c89/vm.c -o build/test/vm-c89/vm
#test/vm/run.sh build/test/vm-c89/vm

# x86_64-linux
mkdir -p build/test/vm-x86_64-linux
build/test/hex-c89/hex platform/vm/x86_64-linux/vm.ohx -o build/test/vm-x86_64-linux/vm
chmod +x build/test/vm-x86_64-linux/vm
test/vm/run.sh build/test/vm-x86_64-linux/vm

# python
test/vm/run.sh platform/vm/python/vm.py

# sh
# TODO disabled for now, it's too slow
#test/vm/run.sh platform/vm/sh/vm.sh

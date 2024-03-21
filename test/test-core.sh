#!/bin/sh

# The MIT License (MIT)
#
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

# This script compiles each core component with the best available toolchain
# and runs its test suite.
#
# This does not test anything in platform/ (although the test scripts for the
# core hex stages are in platform/.)

set -e
cd "$(dirname "$0")/.."

# Setup for a POSIX environment
#scripts/posix/setup.sh
. scripts/posix/env.sh

# initial hex tool
platform/hex/onramp/test.sh

# linker
make -C test/ld/0-global
make -C test/ar/0-cat
make -C test/libc/0-oo

# assembler
make -C test/libo/0-oo
make -C test/as/0-basic
make -C test/as/1-compound

# omC compiler
make -C test/cpp/0-strip
make -C test/cci/0-omc

# omC toolchain
make -C test/cpp/1-omc
make -C test/cc
make -C test/ld/1-omc
make -C test/libc/1-omc

# opC compiler
make -C test/cci/1-opc

# full compiler
# TODO some tools not done yet
#make -C test/libc/2-opc
#make -C test/cpp/2-full
#make -C test/cci/2-full

# full toolchain
#make -C test/libc/3-full
make -C test/ld/2-full
make -C test/as/2-full

# extra tools
platform/hex/c89/test.sh
#make -C test/ar/1-unix

echo "All tests pass."

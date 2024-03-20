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


# This script builds the C89 VM. You need a C compiler.
#
# Set CC to use a specific compiler.
# Set CFLAGS to override the flags to use.


set -e
cd "$(dirname "$0")/../../.."

# Find a compiler
if [ "x$CC" = "x" ]; then
    if ! command -v cc > /dev/null; then
        echo "ERROR: A compiler is required."
        exit 1
    fi
    CC=cc
fi

# Choose compiler flags
# (set CFLAGS to override the defaults)
if [ "x$CFLAGS" = "x" ]; then
    CFLAGS="-O2 -g -std=c89 -Wall -Wextra -Wpedantic"
fi

# Compile it
mkdir -p build/test/vm-c89
$CC $CFLAGS platform/vm/c89/vm.c -o build/test/vm-c89/vm
echo "Compiled: build/test/vm-c89/vm"

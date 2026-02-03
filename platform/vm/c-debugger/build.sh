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


# This script builds the debugger VM. You need a C compiler.
#
# Set CC to use a specific compiler.
# Set CFLAGS to override the flags to use.
#
# You can also use the Makefile to work on the VM. This script is used by the
# setup scripts so that the VM can be built without a make tool.


set -e

SRCS="
    platform/vm/c-debugger/src/vm.c
    platform/vm/c-debugger/src/debug.c
    platform/vm/c-debugger/src/vmcommon.c
    platform/vm/c-debugger/src/terminal.c
    core/libo/1-opc/src/libo-error.c
    core/libo/1-opc/src/libo-string.c
    core/libo/1-opc/src/libo-table.c
    core/libo/1-opc/src/libo-util.c
    core/libo/1-opc/src/libo-vector.c
"

# Find a compiler
if [ "x$CC" = "x" ]; then
    if ! command -v cc > /dev/null; then
        echo "ERROR: A compiler is required."
        exit 1
    fi
    CC=cc
fi

# Choose compiler flags
# (set CFLAGS to override the defaults or EXTRA_CFLAGS to add flags)
if [ "x$CFLAGS" = "x" ]; then
    CFLAGS="-O2 -g -Wall -Wextra -Wpedantic -Wno-unused-parameter"
fi
CFLAGS="$CFLAGS $EXTRA_CFLAGS -Icore/libo/1-opc/include"

# Compile it
mkdir -p build/test/vm-c-debugger
$CC $CFLAGS $SRCS -o build/test/vm-c-debugger/vm
echo "Compiled: build/test/vm-c-debugger/vm"

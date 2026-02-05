#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2023-2025 Fraser Heavy Software
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

set -e
mkdir -p output/intermediate/libc-3-full

echo
echo === Building libc/3-full

echo Compiling libc/3-full atexit.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/atexit.c \
    -o output/intermediate/libc-3-full/atexit.oo

echo Compiling libc/3-full bsearch.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/bsearch.c \
    -o output/intermediate/libc-3-full/bsearch.oo

echo Compiling libc/3-full malloc.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/malloc.c \
    -o output/intermediate/libc-3-full/malloc.oo

echo Compiling libc/3-full qsort.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/qsort.c \
    -o output/intermediate/libc-3-full/qsort.oo

echo Compiling libc/3-full rand.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/rand.c \
    -o output/intermediate/libc-3-full/rand.oo

echo Compiling libc/3-full signal.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/signal.c \
    -o output/intermediate/libc-3-full/signal.oo

echo Compiling libc/3-full stdbit_llong.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/stdbit_llong.c \
    -o output/intermediate/libc-3-full/stdbit_llong.oo

echo Compiling libc/3-full stdlib_3.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/stdlib_3.c \
    -o output/intermediate/libc-3-full/stdlib_3.oo

echo Compiling libc/3-full strtol.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/strtol.c \
    -o output/intermediate/libc-3-full/strtol.oo

echo Compiling libc/3-full time.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/time.c \
    -o output/intermediate/libc-3-full/time.oo

# Note: start.oo must come first!
echo Archiving libc/3-full
onrampvm output/intermediate/ar-0-cat/ar.oe \
    rc output/intermediate/libc-3-full/libc.oa \
    \
    output/intermediate/libc-2-opc/start.oo \
    \
    core/libc/0-oo/src/errno.oo \
    core/libc/0-oo/src/spawn.oo \
    \
    output/intermediate/libc-2-opc/assert.oo \
    output/intermediate/libc-2-opc/ctype.oo \
    output/intermediate/libc-2-opc/environ.oo \
    output/intermediate/libc-2-opc/float.oo \
    output/intermediate/libc-2-opc/format.oo \
    output/intermediate/libc-2-opc/llong.oo \
    output/intermediate/libc-2-opc/multibyte.oo \
    output/intermediate/libc-2-opc/posixio.oo \
    output/intermediate/libc-2-opc/setjmp.oo \
    output/intermediate/libc-2-opc/stdbit.oo \
    output/intermediate/libc-2-opc/stdio.oo \
    output/intermediate/libc-2-opc/stdlib.oo \
    output/intermediate/libc-2-opc/string.oo \
    output/intermediate/libc-2-opc/strings.oo \
    output/intermediate/libc-2-opc/syscalls.oo \
    output/intermediate/libc-2-opc/system.oo \
    \
    output/intermediate/libc-3-full/atexit.oo \
    output/intermediate/libc-3-full/bsearch.oo \
    output/intermediate/libc-3-full/malloc.oo \
    output/intermediate/libc-3-full/qsort.oo \
    output/intermediate/libc-3-full/rand.oo \
    output/intermediate/libc-3-full/signal.oo \
    output/intermediate/libc-3-full/stdbit_llong.oo \
    output/intermediate/libc-3-full/stdlib_3.oo \
    output/intermediate/libc-3-full/strtol.oo \
    output/intermediate/libc-3-full/time.oo \


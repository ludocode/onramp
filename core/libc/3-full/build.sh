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

set -e
mkdir -p build/intermediate/libc-3-full

echo
echo === Building libc/3-full

echo Compiling libc/3-full atexit.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/atexit.c \
    -o build/intermediate/libc-3-full/atexit.oo

echo Compiling libc/3-full bsearch.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/bsearch.c \
    -o build/intermediate/libc-3-full/bsearch.oo

echo Compiling libc/3-full malloc.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/malloc.c \
    -o build/intermediate/libc-3-full/malloc.oo

echo Compiling libc/3-full qsort.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/qsort.c \
    -o build/intermediate/libc-3-full/qsort.oo

echo Compiling libc/3-full rand.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/rand.c \
    -o build/intermediate/libc-3-full/rand.oo

echo Compiling libc/3-full stdbit_llong.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/stdbit_llong.c \
    -o build/intermediate/libc-3-full/stdbit_llong.oo

echo Compiling libc/3-full stdlib_3.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/build-ccargs \
    -c core/libc/3-full/src/stdlib_3.c \
    -o build/intermediate/libc-3-full/stdlib_3.oo

# Note: start.oo must come first!
# TODO remove libc/1 malloc.oo when libc/3 malloc builds
echo Archiving libc/3-full
onrampvm build/intermediate/ar-0-cat/ar.oe \
    rc build/intermediate/libc-3-full/libc.oa \
    \
    build/intermediate/libc-2-opc/start.oo \
    \
    core/libc/0-oo/src/errno.oo \
    core/libc/0-oo/src/malloc_util.oo \
    core/libc/0-oo/src/spawn.oo \
    \
    build/intermediate/libc-1-omc/malloc.oo \
    build/intermediate/libc-1-omc/strtol.oo \
    \
    build/intermediate/libc-2-opc/assert.oo \
    build/intermediate/libc-2-opc/ctype.oo \
    build/intermediate/libc-2-opc/environ.oo \
    build/intermediate/libc-2-opc/file.oo \
    build/intermediate/libc-2-opc/format.oo \
    build/intermediate/libc-2-opc/io.oo \
    build/intermediate/libc-2-opc/llong.oo \
    build/intermediate/libc-2-opc/multibyte.oo \
    build/intermediate/libc-2-opc/setjmp.oo \
    build/intermediate/libc-2-opc/stdbit.oo \
    build/intermediate/libc-2-opc/string.oo \
    build/intermediate/libc-2-opc/strings.oo \
    build/intermediate/libc-2-opc/syscalls.oo \
    build/intermediate/libc-2-opc/system.oo \
    \
    build/intermediate/libc-3-full/atexit.oo \
    build/intermediate/libc-3-full/bsearch.oo \
    build/intermediate/libc-3-full/malloc.oo \
    build/intermediate/libc-3-full/qsort.oo \
    build/intermediate/libc-3-full/rand.oo \
    build/intermediate/libc-3-full/stdbit_llong.oo \
    build/intermediate/libc-3-full/stdlib_3.oo \


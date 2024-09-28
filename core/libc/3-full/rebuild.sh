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

set -e
mkdir -p build/intermediate/libc-3-full-re
mkdir -p build/output/lib

echo
echo === Rebuilding libc/3-full


# libc/1 sources

# TODO remove libc/1 malloc.oo when libc/3 malloc builds
echo Compiling libc/1-omc malloc.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/1-omc/src/malloc.c \
    -o build/intermediate/libc-3-full-re/malloc-libc1.oo

echo Compiling libc/1-omc strtol.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/1-omc/src/strtol.c \
    -o build/intermediate/libc-3-full-re/strtol.oo


# libc/2 sources

echo Assembling libc/2-opc start.os
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/start.os \
    -o build/intermediate/libc-3-full-re/start.oo

echo Compiling libc/2-opc assert.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/assert.c \
    -o build/intermediate/libc-3-full-re/assert.oo

echo Compiling libc/2-opc ctype.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/ctype.c \
    -o build/intermediate/libc-3-full-re/ctype.oo

echo Compiling libc/2-opc environ.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/environ.c \
    -o build/intermediate/libc-3-full-re/environ.oo

echo Compiling libc/2-opc file.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/file.c \
    -o build/intermediate/libc-3-full-re/file.oo

echo Compiling libc/2-opc format.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/format.c \
    -o build/intermediate/libc-3-full-re/format.oo

echo Compiling libc/2-opc io.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/io.c \
    -o build/intermediate/libc-3-full-re/io.oo

echo Compiling libc/2-opc llong.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/llong.c \
    -o build/intermediate/libc-3-full-re/llong.oo

echo Compiling libc/2-opc multibyte.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/multibyte.c \
    -o build/intermediate/libc-3-full-re/multibyte.oo

echo Assembling libc/2-opc setjmp.os
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/setjmp.os \
    -o build/intermediate/libc-3-full-re/setjmp.oo

echo Compiling libc/2-opc stdbit.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/stdbit.c \
    -o build/intermediate/libc-3-full-re/stdbit.oo

echo Compiling libc/2-opc string.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/string.c \
    -o build/intermediate/libc-3-full-re/string.oo

echo Compiling libc/2-opc strings.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/strings.c \
    -o build/intermediate/libc-3-full-re/strings.oo

echo Assembling libc/2-opc syscalls.os
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/syscalls.os \
    -o build/intermediate/libc-3-full-re/syscalls.oo

echo Compiling libc/2-opc system.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/system.c \
    -o build/intermediate/libc-3-full-re/system.oo



# libc/3 sources

echo Compiling libc/3-full atexit.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/atexit.c \
    -o build/intermediate/libc-3-full-re/atexit.oo

echo Compiling libc/3-full bsearch.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/bsearch.c \
    -o build/intermediate/libc-3-full-re/bsearch.oo

echo Compiling libc/3-full malloc.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/malloc.c \
    -o build/intermediate/libc-3-full-re/malloc.oo

echo Compiling libc/3-full qsort.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/qsort.c \
    -o build/intermediate/libc-3-full-re/qsort.oo

echo Compiling libc/3-full rand.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/rand.c \
    -o build/intermediate/libc-3-full-re/rand.oo

echo Compiling libc/3-full stdbit_llong.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/stdbit_llong.c \
    -o build/intermediate/libc-3-full-re/stdbit_llong.oo

echo Compiling libc/3-full stdlib_3.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/stdlib_3.c \
    -o build/intermediate/libc-3-full-re/stdlib_3.oo

echo Compiling libc/3-full time.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/time.c \
    -o build/intermediate/libc-3-full-re/time.oo


# link

# Note: start.oo must come first!
# TODO remove libc/1 malloc.oo when libc/3 malloc builds
echo Archiving libc/3-full
onrampvm build/intermediate/ar-0-cat/ar.oe \
    rc build/output/lib/libc.oa \
    \
    build/intermediate/libc-2-opc/start.oo \
    \
    core/libc/0-oo/src/errno.oo \
    core/libc/0-oo/src/malloc_util.oo \
    core/libc/0-oo/src/spawn.oo \
    \
    build/intermediate/libc-3-full-re/malloc-libc1.oo \
    build/intermediate/libc-3-full-re/strtol.oo \
    \
    build/intermediate/libc-3-full-re/assert.oo \
    build/intermediate/libc-3-full-re/ctype.oo \
    build/intermediate/libc-3-full-re/environ.oo \
    build/intermediate/libc-3-full-re/file.oo \
    build/intermediate/libc-3-full-re/format.oo \
    build/intermediate/libc-3-full-re/io.oo \
    build/intermediate/libc-3-full-re/llong.oo \
    build/intermediate/libc-3-full-re/multibyte.oo \
    build/intermediate/libc-3-full-re/setjmp.oo \
    build/intermediate/libc-3-full-re/stdbit.oo \
    build/intermediate/libc-3-full-re/string.oo \
    build/intermediate/libc-3-full-re/strings.oo \
    build/intermediate/libc-3-full-re/syscalls.oo \
    build/intermediate/libc-3-full-re/system.oo \
    \
    build/intermediate/libc-3-full-re/atexit.oo \
    build/intermediate/libc-3-full-re/bsearch.oo \
    build/intermediate/libc-3-full-re/malloc.oo \
    build/intermediate/libc-3-full-re/qsort.oo \
    build/intermediate/libc-3-full-re/rand.oo \
    build/intermediate/libc-3-full-re/stdbit_llong.oo \
    build/intermediate/libc-3-full-re/stdlib_3.oo \
    build/intermediate/libc-3-full-re/time.oo \


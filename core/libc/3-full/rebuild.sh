#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2024-2025 Fraser Heavy Software
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
mkdir -p output/intermediate/libc-3-full-re
mkdir -p output/final/lib

echo
echo === Rebuilding libc/3-full


# libc/1 sources

echo Assembling libc/1-omc string-fast.os
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/1-omc/src/string-fast.os \
    -o output/intermediate/libc-3-full-re/string-fast.oo


# libc/2 sources

echo Assembling libc/2-opc start.os
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/start.os \
    -o output/intermediate/libc-3-full-re/start.oo

echo Compiling libc/2-opc assert.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/assert.c \
    -o output/intermediate/libc-3-full-re/assert.oo

echo Compiling libc/2-opc ctype.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/ctype.c \
    -o output/intermediate/libc-3-full-re/ctype.oo

echo Compiling libc/2-opc environ.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/environ.c \
    -o output/intermediate/libc-3-full-re/environ.oo

echo Compiling libc/2-opc errno.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/errno.c \
    -o output/intermediate/libc-3-full-re/errno.oo

echo Compiling libc/2-opc float.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/float.c \
    -o output/intermediate/libc-3-full-re/float.oo

echo Compiling libc/2-opc format.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/format.c \
    -o output/intermediate/libc-3-full-re/format.oo

echo Compiling libc/2-opc llong.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/llong.c \
    -o output/intermediate/libc-3-full-re/llong.oo

echo Compiling libc/2-opc multibyte.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/multibyte.c \
    -o output/intermediate/libc-3-full-re/multibyte.oo

echo Compiling libc/2-opc posixio.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/posixio.c \
    -o output/intermediate/libc-3-full-re/posixio.oo

echo Compiling libc/2-opc path.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/path.c \
    -o output/intermediate/libc-3-full-re/path.oo

echo Assembling libc/2-opc setjmp.os
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/setjmp.os \
    -o output/intermediate/libc-3-full-re/setjmp.oo

echo Compiling libc/2-opc stdbit.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/stdbit.c \
    -o output/intermediate/libc-3-full-re/stdbit.oo

echo Compiling libc/2-opc stdio.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/stdio.c \
    -o output/intermediate/libc-3-full-re/stdio.oo

echo Compiling libc/2-opc stdlib.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/stdlib.c \
    -o output/intermediate/libc-3-full-re/stdlib.oo

echo Compiling libc/2-opc string.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/string.c \
    -o output/intermediate/libc-3-full-re/string.oo

echo Compiling libc/2-opc strings.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/strings.c \
    -o output/intermediate/libc-3-full-re/strings.oo

echo Assembling libc/2-opc syscalls.os
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/syscalls.os \
    -o output/intermediate/libc-3-full-re/syscalls.oo

echo Compiling libc/2-opc system.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/2-opc/src/system.c \
    -o output/intermediate/libc-3-full-re/system.oo



# libc/3 sources

echo Compiling libc/3-full atexit.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/atexit.c \
    -o output/intermediate/libc-3-full-re/atexit.oo

echo Compiling libc/3-full bsearch.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/bsearch.c \
    -o output/intermediate/libc-3-full-re/bsearch.oo

echo Compiling libc/3-full malloc.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/malloc.c \
    -o output/intermediate/libc-3-full-re/malloc.oo

echo Compiling libc/3-full qsort.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/qsort.c \
    -o output/intermediate/libc-3-full-re/qsort.oo

echo Compiling libc/3-full rand.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/rand.c \
    -o output/intermediate/libc-3-full-re/rand.oo

echo Compiling libc/3-full signal.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/signal.c \
    -o output/intermediate/libc-3-full-re/signal.oo

echo Compiling libc/3-full stdbit_llong.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/stdbit_llong.c \
    -o output/intermediate/libc-3-full-re/stdbit_llong.oo

echo Compiling libc/3-full stdlib_3.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/stdlib_3.c \
    -o output/intermediate/libc-3-full-re/stdlib_3.oo

echo Compiling libc/3-full strtol.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/strtol.c \
    -o output/intermediate/libc-3-full-re/strtol.oo

echo Compiling libc/3-full time.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/3-full/rebuild-ccargs \
    -c core/libc/3-full/src/time.c \
    -o output/intermediate/libc-3-full-re/time.oo


# link

# Note: start.oo must come first!
echo Archiving libc/3-full
onrampvm output/intermediate/ar-0-cat/ar.oe \
    rc output/final/lib/libc.oa \
    \
    output/intermediate/libc-2-opc/start.oo \
    \
    core/libc/0-oo/src/spawn.oo \
    \
    output/intermediate/libc-3-full-re/string-fast.oo \
    \
    output/intermediate/libc-3-full-re/assert.oo \
    output/intermediate/libc-3-full-re/ctype.oo \
    output/intermediate/libc-3-full-re/environ.oo \
    output/intermediate/libc-3-full-re/errno.oo \
    output/intermediate/libc-3-full-re/float.oo \
    output/intermediate/libc-3-full-re/format.oo \
    output/intermediate/libc-3-full-re/llong.oo \
    output/intermediate/libc-3-full-re/multibyte.oo \
    output/intermediate/libc-3-full-re/path.oo \
    output/intermediate/libc-3-full-re/posixio.oo \
    output/intermediate/libc-3-full-re/setjmp.oo \
    output/intermediate/libc-3-full-re/stdbit.oo \
    output/intermediate/libc-3-full-re/stdio.oo \
    output/intermediate/libc-3-full-re/stdlib.oo \
    output/intermediate/libc-3-full-re/string.oo \
    output/intermediate/libc-3-full-re/strings.oo \
    output/intermediate/libc-3-full-re/syscalls.oo \
    output/intermediate/libc-3-full-re/system.oo \
    \
    output/intermediate/libc-3-full-re/atexit.oo \
    output/intermediate/libc-3-full-re/bsearch.oo \
    output/intermediate/libc-3-full-re/malloc.oo \
    output/intermediate/libc-3-full-re/qsort.oo \
    output/intermediate/libc-3-full-re/rand.oo \
    output/intermediate/libc-3-full-re/signal.oo \
    output/intermediate/libc-3-full-re/stdbit_llong.oo \
    output/intermediate/libc-3-full-re/stdlib_3.oo \
    output/intermediate/libc-3-full-re/strtol.oo \
    output/intermediate/libc-3-full-re/time.oo \


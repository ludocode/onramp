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

set -e
mkdir -p output/intermediate/libc-2-opc
mkdir -p output/intermediate/libc-2-opc/math

echo
echo === Building libc/2-opc

echo Assembling libc/2-opc start.os
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/start.os \
    -o output/intermediate/libc-2-opc/start.oo

# math

echo Compiling libc/2-opc math/bigdec.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/math/bigdec.c \
    -o output/intermediate/libc-2-opc/math/bigdec.oo

echo Compiling libc/2-opc math/bigint.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/math/bigint.c \
    -o output/intermediate/libc-2-opc/math/bigint.oo

echo Compiling libc/2-opc math/float_funcs.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/math/float_funcs.c \
    -o output/intermediate/libc-2-opc/math/float_funcs.oo

echo Compiling libc/2-opc math/float_ops.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/math/float_ops.c \
    -o output/intermediate/libc-2-opc/math/float_ops.oo

echo Compiling libc/2-opc math/float_util.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/math/float_util.c \
    -o output/intermediate/libc-2-opc/math/float_util.oo

echo Compiling libc/2-opc math/double_funcs.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/math/double_funcs.c \
    -o output/intermediate/libc-2-opc/math/double_funcs.oo

echo Compiling libc/2-opc math/double_ops.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/math/double_ops.c \
    -o output/intermediate/libc-2-opc/math/double_ops.oo

echo Compiling libc/2-opc math/double_util.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/math/double_util.c \
    -o output/intermediate/libc-2-opc/math/double_util.oo

echo Compiling libc/2-opc math/llong.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/math/llong.c \
    -o output/intermediate/libc-2-opc/math/llong.oo

echo Compiling libc/2-opc math/int.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/math/int.c \
    -o output/intermediate/libc-2-opc/math/int.oo

# misc

echo Compiling libc/2-opc assert.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/assert.c \
    -o output/intermediate/libc-2-opc/assert.oo

echo Compiling libc/2-opc ctype.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/ctype.c \
    -o output/intermediate/libc-2-opc/ctype.oo

echo Compiling libc/2-opc environ.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/environ.c \
    -o output/intermediate/libc-2-opc/environ.oo

echo Compiling libc/2-opc errno.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/errno.c \
    -o output/intermediate/libc-2-opc/errno.oo

echo Compiling libc/2-opc format.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/format.c \
    -o output/intermediate/libc-2-opc/format.oo

echo Compiling libc/2-opc multibyte.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/multibyte.c \
    -o output/intermediate/libc-2-opc/multibyte.oo

echo Compiling libc/2-opc posixio.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/posixio.c \
    -o output/intermediate/libc-2-opc/posixio.oo

echo Compiling libc/2-opc path.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/path.c \
    -o output/intermediate/libc-2-opc/path.oo

echo Assembling libc/2-opc setjmp.os
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/setjmp.os \
    -o output/intermediate/libc-2-opc/setjmp.oo

echo Compiling libc/2-opc stdbit.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/stdbit.c \
    -o output/intermediate/libc-2-opc/stdbit.oo

echo Compiling libc/2-opc stdio.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/stdio.c \
    -o output/intermediate/libc-2-opc/stdio.oo

echo Compiling libc/2-opc string.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/string.c \
    -o output/intermediate/libc-2-opc/string.oo

echo Compiling libc/2-opc strings.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/strings.c \
    -o output/intermediate/libc-2-opc/strings.oo

echo Compiling libc/2-opc stubs.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/stubs.c \
    -o output/intermediate/libc-2-opc/stubs.oo

echo Assembling libc/2-opc syscalls.os
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/syscalls.os \
    -o output/intermediate/libc-2-opc/syscalls.oo

echo Compiling libc/2-opc system.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/system.c \
    -o output/intermediate/libc-2-opc/system.oo

# Note: start.oo must come first!
echo Archiving libc/2-opc
onrampvm output/intermediate/ar-0-cat/ar.oe \
    rc output/intermediate/libc-2-opc/libc.oa \
    \
    output/intermediate/libc-2-opc/start.oo \
    \
    core/libc/0-oo/src/malloc.oo \
    core/libc/0-oo/src/malloc_util.oo \
    core/libc/0-oo/src/spawn.oo \
    \
    output/intermediate/libc-1-omc/string-fast.oo \
    output/intermediate/libc-1-omc/strtol.oo \
    \
    output/intermediate/libc-2-opc/math/bigdec.oo \
    output/intermediate/libc-2-opc/math/bigint.oo \
    output/intermediate/libc-2-opc/math/double_funcs.oo \
    output/intermediate/libc-2-opc/math/double_ops.oo \
    output/intermediate/libc-2-opc/math/double_util.oo \
    output/intermediate/libc-2-opc/math/float_funcs.oo \
    output/intermediate/libc-2-opc/math/float_ops.oo \
    output/intermediate/libc-2-opc/math/float_util.oo \
    output/intermediate/libc-2-opc/math/llong.oo \
    output/intermediate/libc-2-opc/math/int.oo \
    \
    output/intermediate/libc-2-opc/assert.oo \
    output/intermediate/libc-2-opc/ctype.oo \
    output/intermediate/libc-2-opc/environ.oo \
    output/intermediate/libc-2-opc/errno.oo \
    output/intermediate/libc-2-opc/format.oo \
    output/intermediate/libc-2-opc/multibyte.oo \
    output/intermediate/libc-2-opc/path.oo \
    output/intermediate/libc-2-opc/posixio.oo \
    output/intermediate/libc-2-opc/setjmp.oo \
    output/intermediate/libc-2-opc/stdbit.oo \
    output/intermediate/libc-2-opc/stdio.oo \
    output/intermediate/libc-2-opc/string.oo \
    output/intermediate/libc-2-opc/strings.oo \
    output/intermediate/libc-2-opc/stubs.oo \
    output/intermediate/libc-2-opc/syscalls.oo \
    output/intermediate/libc-2-opc/system.oo \


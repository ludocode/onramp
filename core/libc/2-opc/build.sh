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
mkdir -p build
mkdir -p build/intermediate
mkdir -p build/intermediate/libc-2-opc


echo Compiling libc/2-opc start.os
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/start.os \
    -o build/intermediate/libc-2-opc/start.oo


echo Compiling libc/2-opc assert.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/assert.c \
    -o build/intermediate/libc-2-opc/assert.oo

echo Compiling libc/2-opc constructors.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/constructors.c \
    -o build/intermediate/libc-2-opc/constructors.oo

echo Compiling libc/2-opc ctype.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/ctype.c \
    -o build/intermediate/libc-2-opc/ctype.oo

echo Compiling libc/2-opc environ.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/environ.c \
    -o build/intermediate/libc-2-opc/environ.oo

echo Compiling libc/2-opc file.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/file.c \
    -o build/intermediate/libc-2-opc/file.oo

echo Compiling libc/2-opc format.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/format.c \
    -o build/intermediate/libc-2-opc/format.oo

echo Compiling libc/2-opc io.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/io.c \
    -o build/intermediate/libc-2-opc/io.oo

echo Compiling libc/2-opc math64.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/math64.c \
    -o build/intermediate/libc-2-opc/math64.oo

echo Compiling libc/2-opc string.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/string.c \
    -o build/intermediate/libc-2-opc/string.oo

echo Compiling libc/2-opc syscalls.os
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/syscalls.os \
    -o build/intermediate/libc-2-opc/syscalls.oo

echo Compiling libc/2-opc system.c
onrampvm build/intermediate/cc/cc.oe \
    @core/libc/2-opc/build-ccargs \
    -c core/libc/2-opc/src/system.c \
    -o build/intermediate/libc-2-opc/system.oo

echo Archiving libc/2-opc
onrampvm build/intermediate/ar-0-cat/ar.oe \
    rc build/intermediate/libc-2-opc/libc.oa \
    \
    build/intermediate/libc-2-opc/start.oo \
    \
    build/intermediate/libc-2-opc/assert.oo \
    build/intermediate/libc-2-opc/constructors.oo \
    build/intermediate/libc-2-opc/ctype.oo \
    build/intermediate/libc-2-opc/environ.oo \
    build/intermediate/libc-2-opc/file.oo \
    build/intermediate/libc-2-opc/format.oo \
    build/intermediate/libc-2-opc/io.oo \
    build/intermediate/libc-2-opc/math64.oo \
    build/intermediate/libc-2-opc/setjmp.oo \
    build/intermediate/libc-2-opc/string.oo \
    build/intermediate/libc-2-opc/syscalls.oo \
    build/intermediate/libc-2-opc/system.oo \

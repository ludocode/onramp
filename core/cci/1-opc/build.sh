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
mkdir -p build/intermediate/cci-1-opc

# TODO implement @file in cc. each build program can have a `ccargs` file which
# contains arguments to the compiler.

# TODO or, alternatively, just pass all input files to a single call to cc

echo Compiling cci/1-opc common.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/common.c \
    -o build/intermediate/cci-1-opc/common.oo

echo Compiling cci/1-opc compile.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/compile.c \
    -o build/intermediate/cci-1-opc/compile.oo

echo Compiling cci/1-opc emit.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/emit.c \
    -o build/intermediate/cci-1-opc/emit.oo

echo Compiling cci/1-opc lexer.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/lexer.c \
    -o build/intermediate/cci-1-opc/lexer.oo

echo Compiling cci/1-opc main.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/main.c \
    -o build/intermediate/cci-1-opc/main.oo

echo Compiling cci/1-opc parse.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/parse.c \
    -o build/intermediate/cci-1-opc/parse.oo

echo Compiling cci/1-opc type.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/type.c \
    -o build/intermediate/cci-1-opc/type.oo

echo Compiling cci/1-opc variable.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/variable.c \
    -o build/intermediate/cci-1-opc/variable.oo

echo Linking cci/1-opc
onrampvm build/intermediate/ld-1-omc/ld.oe \
    build/intermediate/libc-1-omc/libc.oa \
    build/intermediate/libo-0-oo/libo.oa \
    build/intermediate/cci-1-opc/common.oo \
    build/intermediate/cci-1-opc/compile.oo \
    build/intermediate/cci-1-opc/emit.oo \
    build/intermediate/cci-1-opc/lexer.oo \
    build/intermediate/cci-1-opc/main.oo \
    build/intermediate/cci-1-opc/parse.oo \
    build/intermediate/cci-1-opc/type.oo \
    build/intermediate/cci-1-opc/variable.oo \
    -o build/intermediate/cci-1-opc/cci.oe

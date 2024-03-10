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

echo Compiling cci/1-opc global.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/global.c \
    -o build/intermediate/cci-1-opc/global.oo

echo Compiling cci/1-opc lexer.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/lexer.c \
    -o build/intermediate/cci-1-opc/lexer.oo

echo Compiling cci/1-opc locals.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/locals.c \
    -o build/intermediate/cci-1-opc/locals.oo

echo Compiling cci/1-opc main.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/main.c \
    -o build/intermediate/cci-1-opc/main.oo

echo Compiling cci/1-opc parse-decl.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/parse-decl.c \
    -o build/intermediate/cci-1-opc/parse-decl.oo

echo Compiling cci/1-opc parse-expr.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/parse-expr.c \
    -o build/intermediate/cci-1-opc/parse-expr.oo

echo Compiling cci/1-opc parse-stmt.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/parse-stmt.c \
    -o build/intermediate/cci-1-opc/parse-stmt.oo

echo Compiling cci/1-opc type.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/1-opc/build-ccargs \
    -c core/cci/1-opc/src/type.c \
    -o build/intermediate/cci-1-opc/type.oo

echo Linking cci/1-opc
onrampvm build/intermediate/ld-1-omc/ld.oe \
    build/intermediate/libc-1-omc/libc.oa \
    build/intermediate/libo-0-oo/libo.oa \
    build/intermediate/cci-1-opc/common.oo \
    build/intermediate/cci-1-opc/compile.oo \
    build/intermediate/cci-1-opc/emit.oo \
    build/intermediate/cci-1-opc/global.oo \
    build/intermediate/cci-1-opc/lexer.oo \
    build/intermediate/cci-1-opc/locals.oo \
    build/intermediate/cci-1-opc/main.oo \
    build/intermediate/cci-1-opc/parse-decl.oo \
    build/intermediate/cci-1-opc/parse-expr.oo \
    build/intermediate/cci-1-opc/parse-stmt.oo \
    build/intermediate/cci-1-opc/type.oo \
    -o build/intermediate/cci-1-opc/cci.oe

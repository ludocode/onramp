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
mkdir -p output/intermediate/cpp-2-full

echo
echo === Building cpp/2-full

echo Compiling cpp/2-full common.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/common.c \
    -o output/intermediate/cpp-2-full/common.oo

echo Compiling cpp/2-full directive.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/directive.c \
    -o output/intermediate/cpp-2-full/directive.oo

echo Compiling cpp/2-full emit.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/emit.c \
    -o output/intermediate/cpp-2-full/emit.oo

echo Compiling cpp/2-full expression.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/expression.c \
    -o output/intermediate/cpp-2-full/expression.oo

echo Compiling cpp/2-full file.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/file.c \
    -o output/intermediate/cpp-2-full/file.oo

echo Compiling cpp/2-full hideset.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/hideset.c \
    -o output/intermediate/cpp-2-full/hideset.oo

echo Compiling cpp/2-full lexer.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/lexer.c \
    -o output/intermediate/cpp-2-full/lexer.oo

echo Compiling cpp/2-full macro.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/macro.c \
    -o output/intermediate/cpp-2-full/macro.oo

echo Compiling cpp/2-full main.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/main.c \
    -o output/intermediate/cpp-2-full/main.oo

echo Compiling cpp/2-full options.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/options.c \
    -o output/intermediate/cpp-2-full/options.oo

echo Compiling cpp/2-full preprocess.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/preprocess.c \
    -o output/intermediate/cpp-2-full/preprocess.oo

echo Compiling cpp/2-full stream.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/stream.c \
    -o output/intermediate/cpp-2-full/stream.oo

echo Compiling cpp/2-full strings.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/strings.c \
    -o output/intermediate/cpp-2-full/strings.oo

echo Compiling cpp/2-full token.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cpp/2-full/build-ccargs \
    -c core/cpp/2-full/src/token.c \
    -o output/intermediate/cpp-2-full/token.oo

echo Linking cpp/2-full
onrampvm output/intermediate/ld-2-full/ld.oe \
    output/intermediate/libc-2-opc/libc.oa \
    output/intermediate/libo-1-opc/libo.oa \
    output/intermediate/cpp-2-full/common.oo \
    output/intermediate/cpp-2-full/directive.oo \
    output/intermediate/cpp-2-full/emit.oo \
    output/intermediate/cpp-2-full/expression.oo \
    output/intermediate/cpp-2-full/file.oo \
    output/intermediate/cpp-2-full/hideset.oo \
    output/intermediate/cpp-2-full/lexer.oo \
    output/intermediate/cpp-2-full/macro.oo \
    output/intermediate/cpp-2-full/main.oo \
    output/intermediate/cpp-2-full/options.oo \
    output/intermediate/cpp-2-full/preprocess.oo \
    output/intermediate/cpp-2-full/stream.oo \
    output/intermediate/cpp-2-full/strings.oo \
    output/intermediate/cpp-2-full/token.oo \
    -o output/intermediate/cpp-2-full/cpp.oe

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
mkdir -p output/intermediate/cpp-2-full-re

echo
echo === Rebuilding cpp/2-full

echo Compiling cpp/2-full common.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/common.c \
    -o output/intermediate/cpp-2-full-re/common.oo

echo Compiling cpp/2-full directive.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/directive.c \
    -o output/intermediate/cpp-2-full-re/directive.oo

echo Compiling cpp/2-full emit.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/emit.c \
    -o output/intermediate/cpp-2-full-re/emit.oo

echo Compiling cpp/2-full expression.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/expression.c \
    -o output/intermediate/cpp-2-full-re/expression.oo

echo Compiling cpp/2-full file.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/file.c \
    -o output/intermediate/cpp-2-full-re/file.oo

echo Compiling cpp/2-full hideset.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/hideset.c \
    -o output/intermediate/cpp-2-full-re/hideset.oo

echo Compiling cpp/2-full lexer.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/lexer.c \
    -o output/intermediate/cpp-2-full-re/lexer.oo

echo Compiling cpp/2-full macro.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/macro.c \
    -o output/intermediate/cpp-2-full-re/macro.oo

echo Compiling cpp/2-full main.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/main.c \
    -o output/intermediate/cpp-2-full-re/main.oo

echo Compiling cpp/2-full options.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/options.c \
    -o output/intermediate/cpp-2-full-re/options.oo

echo Compiling cpp/2-full preprocess.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/preprocess.c \
    -o output/intermediate/cpp-2-full-re/preprocess.oo

echo Compiling cpp/2-full stream.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/stream.c \
    -o output/intermediate/cpp-2-full-re/stream.oo

echo Compiling cpp/2-full strings.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/strings.c \
    -o output/intermediate/cpp-2-full-re/strings.oo

echo Compiling cpp/2-full token.c
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    -c core/cpp/2-full/src/token.c \
    -o output/intermediate/cpp-2-full-re/token.oo

echo Linking cpp/2-full
onrampvm build/output/bin/cc.oe \
    @core/cpp/2-full/rebuild-ccargs \
    output/intermediate/libo-1-opc-re/libo.oa \
    output/intermediate/cpp-2-full-re/common.oo \
    output/intermediate/cpp-2-full-re/directive.oo \
    output/intermediate/cpp-2-full-re/emit.oo \
    output/intermediate/cpp-2-full-re/expression.oo \
    output/intermediate/cpp-2-full-re/file.oo \
    output/intermediate/cpp-2-full-re/hideset.oo \
    output/intermediate/cpp-2-full-re/lexer.oo \
    output/intermediate/cpp-2-full-re/macro.oo \
    output/intermediate/cpp-2-full-re/main.oo \
    output/intermediate/cpp-2-full-re/options.oo \
    output/intermediate/cpp-2-full-re/preprocess.oo \
    output/intermediate/cpp-2-full-re/stream.oo \
    output/intermediate/cpp-2-full-re/strings.oo \
    output/intermediate/cpp-2-full-re/token.oo \
    -o build/output/bin/cpp.oe

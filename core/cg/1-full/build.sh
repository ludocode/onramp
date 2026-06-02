#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2026 Fraser Heavy Software
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
mkdir -p output/intermediate/cg-1-full

echo
echo === Building cg/1-full

echo Compiling cg/1-full argument.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cg/1-full/build-ccargs \
    -c core/cg/1-full/src/argument.c \
    -o output/intermediate/cg-1-full/argument.oo

echo Compiling cg/1-full block.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cg/1-full/build-ccargs \
    -c core/cg/1-full/src/block.c \
    -o output/intermediate/cg-1-full/block.oo

echo Compiling cg/1-full common.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cg/1-full/build-ccargs \
    -c core/cg/1-full/src/common.c \
    -o output/intermediate/cg-1-full/common.oo

echo Compiling cg/1-full convert.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cg/1-full/build-ccargs \
    -c core/cg/1-full/src/convert.c \
    -o output/intermediate/cg-1-full/convert.oo

echo Compiling cg/1-full emit.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cg/1-full/build-ccargs \
    -c core/cg/1-full/src/emit.c \
    -o output/intermediate/cg-1-full/emit.oo

echo Compiling cg/1-full instruction.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cg/1-full/build-ccargs \
    -c core/cg/1-full/src/instruction.c \
    -o output/intermediate/cg-1-full/instruction.oo

echo Compiling cg/1-full location.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cg/1-full/build-ccargs \
    -c core/cg/1-full/src/location.c \
    -o output/intermediate/cg-1-full/location.oo

echo Compiling cg/1-full main.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cg/1-full/build-ccargs \
    -c core/cg/1-full/src/main.c \
    -o output/intermediate/cg-1-full/main.oo

echo Compiling cg/1-full opcode.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cg/1-full/build-ccargs \
    -c core/cg/1-full/src/opcode.c \
    -o output/intermediate/cg-1-full/opcode.oo

echo Compiling cg/1-full optimize.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cg/1-full/build-ccargs \
    -c core/cg/1-full/src/optimize.c \
    -o output/intermediate/cg-1-full/optimize.oo

echo Compiling cg/1-full parse.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cg/1-full/build-ccargs \
    -c core/cg/1-full/src/parse.c \
    -o output/intermediate/cg-1-full/parse.oo

echo Compiling cg/1-full symbol.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cg/1-full/build-ccargs \
    -c core/cg/1-full/src/symbol.c \
    -o output/intermediate/cg-1-full/symbol.oo

echo Linking cg/1-full
onrampvm output/intermediate/ld-2-full/ld.oe \
    -g \
    output/intermediate/libc-2-opc/libc.oa \
    output/intermediate/libo-1-opc/libo.oa \
    output/intermediate/cg-1-full/argument.oo \
    output/intermediate/cg-1-full/block.oo \
    output/intermediate/cg-1-full/common.oo \
    output/intermediate/cg-1-full/convert.oo \
    output/intermediate/cg-1-full/emit.oo \
    output/intermediate/cg-1-full/instruction.oo \
    output/intermediate/cg-1-full/location.oo \
    output/intermediate/cg-1-full/main.oo \
    output/intermediate/cg-1-full/opcode.oo \
    output/intermediate/cg-1-full/optimize.oo \
    output/intermediate/cg-1-full/parse.oo \
    output/intermediate/cg-1-full/symbol.oo \
    -o output/intermediate/cg-1-full/cg.oe

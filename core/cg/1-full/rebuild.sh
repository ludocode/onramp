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
mkdir -p output/intermediate/cg-1-full-re

echo
echo === Rebuilding cg/1-full

echo Compiling cg/1-full argument.c
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    -c core/cg/1-full/src/argument.c \
    -o output/intermediate/cg-1-full-re/argument.oo

echo Compiling cg/1-full block.c
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    -c core/cg/1-full/src/block.c \
    -o output/intermediate/cg-1-full-re/block.oo

echo Compiling cg/1-full common.c
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    -c core/cg/1-full/src/common.c \
    -o output/intermediate/cg-1-full-re/common.oo

echo Compiling cg/1-full convert.c
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    -c core/cg/1-full/src/convert.c \
    -o output/intermediate/cg-1-full-re/convert.oo

echo Compiling cg/1-full emit.c
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    -c core/cg/1-full/src/emit.c \
    -o output/intermediate/cg-1-full-re/emit.oo

echo Compiling cg/1-full instruction.c
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    -c core/cg/1-full/src/instruction.c \
    -o output/intermediate/cg-1-full-re/instruction.oo

echo Compiling cg/1-full location.c
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    -c core/cg/1-full/src/location.c \
    -o output/intermediate/cg-1-full-re/location.oo

echo Compiling cg/1-full main.c
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    -c core/cg/1-full/src/main.c \
    -o output/intermediate/cg-1-full-re/main.oo

echo Compiling cg/1-full opcode.c
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    -c core/cg/1-full/src/opcode.c \
    -o output/intermediate/cg-1-full-re/opcode.oo

echo Compiling cg/1-full optimize.c
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    -c core/cg/1-full/src/optimize.c \
    -o output/intermediate/cg-1-full-re/optimize.oo

echo Compiling cg/1-full parse.c
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    -c core/cg/1-full/src/parse.c \
    -o output/intermediate/cg-1-full-re/parse.oo

echo Compiling cg/1-full symbol.c
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    -c core/cg/1-full/src/symbol.c \
    -o output/intermediate/cg-1-full-re/symbol.oo

echo Linking cg/1-full
onrampvm output/final/bin/cc.oe \
    @core/cg/1-full/rebuild-ccargs \
    output/intermediate/libo-1-opc-re/libo.oa \
    output/intermediate/cg-1-full-re/argument.oo \
    output/intermediate/cg-1-full-re/block.oo \
    output/intermediate/cg-1-full-re/common.oo \
    output/intermediate/cg-1-full-re/convert.oo \
    output/intermediate/cg-1-full-re/emit.oo \
    output/intermediate/cg-1-full-re/instruction.oo \
    output/intermediate/cg-1-full-re/location.oo \
    output/intermediate/cg-1-full-re/main.oo \
    output/intermediate/cg-1-full-re/opcode.oo \
    output/intermediate/cg-1-full-re/optimize.oo \
    output/intermediate/cg-1-full-re/parse.oo \
    output/intermediate/cg-1-full-re/symbol.oo \
    -o output/final/bin/cci.oe

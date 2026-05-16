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
mkdir -p output/intermediate/as-2-full

echo
echo === Building as/2-full

echo Compiling as/2-full common.c
onrampvm output/intermediate/cc/cc.oe \
    @core/as/2-full/build-ccargs \
    -c core/as/2-full/src/common.c \
    -o output/intermediate/as-2-full/common.oo

echo Compiling as/2-full emit.c
onrampvm output/intermediate/cc/cc.oe \
    @core/as/2-full/build-ccargs \
    -c core/as/2-full/src/emit.c \
    -o output/intermediate/as-2-full/emit.oo

echo Compiling as/2-full main.c
onrampvm output/intermediate/cc/cc.oe \
    @core/as/2-full/build-ccargs \
    -c core/as/2-full/src/main.c \
    -o output/intermediate/as-2-full/main.oo

echo Compiling as/2-full opcodes.c
onrampvm output/intermediate/cc/cc.oe \
    @core/as/2-full/build-ccargs \
    -c core/as/2-full/src/opcodes.c \
    -o output/intermediate/as-2-full/opcodes.oo

echo Compiling as/2-full parse.c
onrampvm output/intermediate/cc/cc.oe \
    @core/as/2-full/build-ccargs \
    -c core/as/2-full/src/parse.c \
    -o output/intermediate/as-2-full/parse.oo

echo Compiling as/2-full symbol.c
onrampvm output/intermediate/cc/cc.oe \
    @core/as/2-full/build-ccargs \
    -c core/as/2-full/src/symbol.c \
    -o output/intermediate/as-2-full/symbol.oo

echo Linking as/2-full
onrampvm output/intermediate/ld-2-full/ld.oe \
    output/intermediate/libc-3-full/libc.oa \
    output/intermediate/libo-1-opc/libo.oa \
    output/intermediate/as-2-full/common.oo \
    output/intermediate/as-2-full/emit.oo \
    output/intermediate/as-2-full/main.oo \
    output/intermediate/as-2-full/opcodes.oo \
    output/intermediate/as-2-full/parse.oo \
    output/intermediate/as-2-full/symbol.oo \
    -o output/intermediate/as-2-full/as.oe

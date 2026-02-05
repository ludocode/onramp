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
mkdir -p output/intermediate/libo-1-opc-re

echo
echo === Rebuilding libo/1-opc

echo Assembling libo/1-opc libo-data.os
onrampvm output/intermediate/cc/cc.oe \
    @core/libo/1-opc/build-ccargs \
    -c core/libo/1-opc/src/libo-data.os \
    -o output/intermediate/libo-1-opc-re/libo-data.oo

echo Compiling libo/1-opc libo-error.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libo/1-opc/build-ccargs \
    -c core/libo/1-opc/src/libo-error.c \
    -o output/intermediate/libo-1-opc-re/libo-error.oo

echo Compiling libo/1-opc libo-reader.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libo/1-opc/build-ccargs \
    -c core/libo/1-opc/src/libo-reader.c \
    -o output/intermediate/libo-1-opc-re/libo-reader.oo

echo Compiling libo/1-opc libo-string.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libo/1-opc/build-ccargs \
    -c core/libo/1-opc/src/libo-string.c \
    -o output/intermediate/libo-1-opc-re/libo-string.oo

echo Compiling libo/1-opc libo-table.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libo/1-opc/build-ccargs \
    -c core/libo/1-opc/src/libo-table.c \
    -o output/intermediate/libo-1-opc-re/libo-table.oo

echo Compiling libo/1-opc libo-unicode.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libo/1-opc/build-ccargs \
    -c core/libo/1-opc/src/libo-unicode.c \
    -o output/intermediate/libo-1-opc-re/libo-unicode.oo

echo Compiling libo/1-opc libo-util.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libo/1-opc/build-ccargs \
    -c core/libo/1-opc/src/libo-util.c \
    -o output/intermediate/libo-1-opc-re/libo-util.oo

echo Compiling libo/1-opc libo-vector.c
onrampvm output/intermediate/cc/cc.oe \
    @core/libo/1-opc/build-ccargs \
    -c core/libo/1-opc/src/libo-vector.c \
    -o output/intermediate/libo-1-opc-re/libo-vector.oo

echo Archiving libo/1-opc
onrampvm output/intermediate/ar-0-cat/ar.oe \
    rc output/intermediate/libo-1-opc-re/libo.oa \
        output/intermediate/libo-1-opc-re/libo-data.oo \
        output/intermediate/libo-1-opc-re/libo-error.oo \
        output/intermediate/libo-1-opc-re/libo-reader.oo \
        output/intermediate/libo-1-opc-re/libo-string.oo \
        output/intermediate/libo-1-opc-re/libo-table.oo \
        output/intermediate/libo-1-opc-re/libo-unicode.oo \
        output/intermediate/libo-1-opc-re/libo-util.oo \
        output/intermediate/libo-1-opc-re/libo-vector.oo

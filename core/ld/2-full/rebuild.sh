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
mkdir -p build/intermediate/ld-2-full-re

echo
echo === Rebuilding ld/2-full

echo Compiling ld/2-full common.c
onrampvm build/output/bin/cc.oe \
    @core/ld/2-full/rebuild-ccargs \
    -c core/ld/2-full/src/common.c \
    -o build/intermediate/ld-2-full-re/common.oo

echo Compiling ld/2-full emit.c
onrampvm build/output/bin/cc.oe \
    @core/ld/2-full/rebuild-ccargs \
    -c core/ld/2-full/src/emit.c \
    -o build/intermediate/ld-2-full-re/emit.oo

echo Compiling ld/2-full label.c
onrampvm build/output/bin/cc.oe \
    @core/ld/2-full/rebuild-ccargs \
    -c core/ld/2-full/src/label.c \
    -o build/intermediate/ld-2-full-re/label.oo

echo Compiling ld/2-full main.c
onrampvm build/output/bin/cc.oe \
    @core/ld/2-full/rebuild-ccargs \
    -c core/ld/2-full/src/main.c \
    -o build/intermediate/ld-2-full-re/main.oo

echo Compiling ld/2-full parse.c
onrampvm build/output/bin/cc.oe \
    @core/ld/2-full/rebuild-ccargs \
    -c core/ld/2-full/src/parse.c \
    -o build/intermediate/ld-2-full-re/parse.oo

echo Compiling ld/2-full symbol.c
onrampvm build/output/bin/cc.oe \
    @core/ld/2-full/rebuild-ccargs \
    -c core/ld/2-full/src/symbol.c \
    -o build/intermediate/ld-2-full-re/symbol.oo

echo Linking ld/2-full
onrampvm build/output/bin/cc.oe \
    @core/ld/2-full/rebuild-ccargs \
    build/intermediate/libo-1-opc/libo.oa \
    build/intermediate/ld-2-full-re/common.oo \
    build/intermediate/ld-2-full-re/emit.oo \
    build/intermediate/ld-2-full-re/label.oo \
    build/intermediate/ld-2-full-re/main.oo \
    build/intermediate/ld-2-full-re/parse.oo \
    build/intermediate/ld-2-full-re/symbol.oo \
    -o build/output/bin/ld.oe

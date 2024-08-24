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
mkdir -p build/intermediate/cc-re
mkdir -p build/output/bin

echo
echo === Building cc

echo Compiling cc
onrampvm build/intermediate/cc/cc.oe \
    -with-cpp=build/intermediate/cpp-2-full/cpp.oe \
    -with-cci=build/intermediate/cci-2-full/cci.oe \
    -with-as=build/intermediate/as-2-full/as.oe \
    -nostdinc \
    -Icore/libc/common/include \
    -Icore/libo/1-opc/include \
    -include __onramp/__predef.h \
    -O \
    -g \
    -c core/cc/cc.c \
    -o build/intermediate/cc-re/cc.oo

echo Linking cc
onrampvm build/intermediate/ld-2-full/ld.oe \
    build/output/lib/libc.oa \
    build/intermediate/libo-1-opc-re/libo.oa \
    build/intermediate/cc-re/cc.oo \
    -o build/output/bin/cc.oe

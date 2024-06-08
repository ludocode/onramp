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
mkdir -p build
mkdir -p build/intermediate
mkdir -p build/intermediate/cc

echo
echo === Building cc

echo Preprocessing cc
onrampvm build/intermediate/cpp-1-omc/cpp.oe \
    -D__onramp__=1 \
    -D__onramp_cci__=1 -D__onramp_cci_omc__=1 \
    -D__onramp_cpp__=1 -D__onramp_cpp_omc__=1 \
    -Drestrict= -D_Noreturn= \
    -Icore/cc/include \
    -Icore/libc/1-omc/include \
    -Icore/libc/0-oo/include \
    -Icore/libo/0-oo/include \
    -include __onramp/__predef.h \
    core/cc/cc.c -o build/intermediate/cc/cc.i

echo Compiling cc
onrampvm build/intermediate/cci-0-omc/cci.oe \
    build/intermediate/cc/cc.i -o build/intermediate/cc/cc.os

echo Assembling cc
onrampvm build/intermediate/as-1-compound/as.oe \
    build/intermediate/cc/cc.os -o build/intermediate/cc/cc.oo

echo Linking cc
onrampvm build/intermediate/ld-1-omc/ld.oe \
    build/intermediate/libc-1-omc/libc.oa \
    build/intermediate/libo-0-oo/libo.oa \
    build/intermediate/cc/cc.oo \
    -o build/intermediate/cc/cc.oe

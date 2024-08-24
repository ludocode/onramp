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
mkdir -p build/intermediate/cpp-1-omc

echo
echo === Building cpp/1-omc

echo Preprocessing cpp/1-omc
onrampvm build/intermediate/cpp-0-strip/cpp.oe \
    core/cpp/1-omc/cpp.c -o build/intermediate/cpp-1-omc/cpp.i

echo Compiling cpp/1-omc
onrampvm build/intermediate/cci-0-omc/cci.oe \
    build/intermediate/cpp-1-omc/cpp.i -o build/intermediate/cpp-1-omc/cpp.os

echo Assembling cpp/1-omc
onrampvm build/intermediate/as-1-compound/as.oe \
    build/intermediate/cpp-1-omc/cpp.os -o build/intermediate/cpp-1-omc/cpp.oo

echo Linking cpp/1-omc
onrampvm build/intermediate/ld-0-global/ld.oe \
    build/intermediate/libc-0-oo/libc.oa \
    build/intermediate/libo-0-oo/libo.oa \
    build/intermediate/cpp-1-omc/cpp.oo \
    -o build/intermediate/cpp-1-omc/cpp.oe

#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2023-2025 Fraser Heavy Software
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
mkdir -p output/intermediate/cpp-1-omc-unopt

echo
echo === Building cpp/1-omc

echo Preprocessing cpp/1-omc
onrampvm output/intermediate/cpp-0-strip/cpp.oe \
    core/cpp/1-omc/cpp.c \
    -o output/intermediate/cpp-1-omc-unopt/cpp.i

echo Compiling cpp/1-omc
onrampvm output/intermediate/cci-0-omc/cci.oe \
    output/intermediate/cpp-1-omc-unopt/cpp.i \
    -o output/intermediate/cpp-1-omc-unopt/cpp.os

echo Assembling cpp/1-omc
onrampvm output/intermediate/as-1-compound/as.oe \
    output/intermediate/cpp-1-omc-unopt/cpp.os \
    -o output/intermediate/cpp-1-omc-unopt/cpp.oo

echo Linking cpp/1-omc
onrampvm output/intermediate/ld-0-global/ld.oe \
    output/intermediate/libc-0-oo/libc.oa \
    output/intermediate/libo-0-oo/libo.oa \
    output/intermediate/cpp-1-omc-unopt/cpp.oo \
    -o output/intermediate/cpp-1-omc-unopt/cpp.oe

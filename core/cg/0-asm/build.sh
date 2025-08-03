#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2025 Fraser Heavy Software
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
mkdir -p build/intermediate/cg-0-asm-unopt
mkdir -p build/intermediate/cg-0-asm

echo
echo === Building cg/0-asm


# We start by producing an unoptimized build of cg/0.

echo Preprocessing cg/0-asm
onrampvm build/intermediate/cpp-1-omc-unopt/cpp.oe \
    -D__onramp__=1 \
    -D__onramp_cci__=1 \
    -D__onramp_cci_omc__=1 \
    -D__onramp_libc_oo__=1 \
    -Icore/libc/common/include \
    -Icore/libo/0-oo/include \
    -include __onramp/__predef.h \
    core/cg/0-asm/src/main.c \
    -o build/intermediate/cg-0-asm-unopt/main.i

echo Compiling cg/0-asm
onrampvm build/intermediate/cci-0-omc/cci.oe \
    build/intermediate/cg-0-asm-unopt/main.i \
    -o build/intermediate/cg-0-asm-unopt/main.os

echo Assembling unoptimized cg/0-asm
onrampvm build/intermediate/as-1-compound/as.oe \
    build/intermediate/cg-0-asm-unopt/main.os -o \
    build/intermediate/cg-0-asm-unopt/main.oo

echo Linking unoptimized cg/0-asm
onrampvm build/intermediate/ld-0-global/ld.oe \
    -o build/intermediate/cg-0-asm-unopt/cg.oe \
        build/intermediate/libc-0-oo/libc.oa \
        build/intermediate/libo-0-oo/libo.oa \
        build/intermediate/cg-0-asm-unopt/main.oo


# We then run our unoptimized build on our own assembly that we just compiled,
# producing an optimized build of cg/0.

echo Optimizing cg/0-asm
onrampvm build/intermediate/cg-0-asm-unopt/cg.oe \
    build/intermediate/cg-0-asm-unopt/main.os \
    -o build/intermediate/cg-0-asm/main.os

echo Assembling optimized cg/0-asm
onrampvm build/intermediate/as-1-compound/as.oe \
    build/intermediate/cg-0-asm/main.os \
    -o build/intermediate/cg-0-asm/main.oo

echo Linking optimized cg/0-asm
onrampvm build/intermediate/ld-0-global/ld.oe \
    -o build/intermediate/cg-0-asm/cg.oe \
        build/intermediate/libc-0-oo/libc.oa \
        build/intermediate/libo-0-oo/libo.oa \
        build/intermediate/cg-0-asm/main.oo

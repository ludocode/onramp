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


# This script builds the final stage Onramp hex tool as part of the Onramp
# bootstrap process. This is an Onramp shell script.
#
# To build the Onramp hex tool for specific platforms, see the build scripts in:
#
#     platform/hex/c89/


set -e
mkdir -p build
mkdir -p build/intermediate
mkdir -p build/intermediate/hex-1-c89

# TODO the final stage tools aren't done yet so for now we build with earlier
# stages. This should eventually be built after the final stages have been
# rebuilt.
echo Compiling hex/1-c89
onrampvm build/intermediate/cc/cc.oe \
    -with-cpp=build/intermediate/cpp-1-omc/cpp.oe \
    -with-cci=build/intermediate/cci-1-opc/cci.oe \
    -with-as=build/intermediate/as-1-compound/as.oe \
    -with-ld=build/intermediate/ld-1-omc/ld.oe \
    -nostddef \
    -nostdinc \
    -D__onramp__=1 \
    -D__onramp_cpp__=1 -D__onramp_cpp_omc__=1 \
    -D__onramp_cci__=1 -D__onramp_cci_omc__=1 \
    -Icore/libc/1-omc/include \
    -Icore/libc/0-oo/include \
    -include __onramp/__predef.h \
    core/hex/1-c89/src/hex.c \
    -o build/intermediate/hex-1-c89/hex.oe

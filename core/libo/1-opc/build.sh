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
mkdir -p build
mkdir -p build/intermediate
mkdir -p build/intermediate/libo-1-opc

echo Compiling libo-error.c
onrampvm build/intermediate/cc/cc.oe \
    -with-cpp=build/intermediate/cpp-1-omc/cpp.oe \
    -with-cci=build/intermediate/cci-1-opc/cci.oe \
    -with-as=build/intermediate/as-1-compound/as.oe \
    -c core/libo/1-opc/src/libo-error.c \
    -o build/intermediate/libo-1-opc/libo-error.oo

echo Compiling libo-string.c
onrampvm build/intermediate/cc/cc.oe \
    -with-cpp=build/intermediate/cpp-1-omc/cpp.oe \
    -with-cci=build/intermediate/cci-1-opc/cci.oe \
    -with-as=build/intermediate/as-1-compound/as.oe \
    -c core/libo/1-opc/src/libo-string.c \
    -o build/intermediate/libo-1-opc/libo-string.oo

echo Compiling libo-table.c
onrampvm build/intermediate/cc/cc.oe \
    -with-cpp=build/intermediate/cpp-1-omc/cpp.oe \
    -with-cci=build/intermediate/cci-1-opc/cci.oe \
    -with-as=build/intermediate/as-1-compound/as.oe \
    -c core/libo/1-opc/src/libo-table.c \
    -o build/intermediate/libo-1-opc/libo-table.oo

echo Compiling libo-util.c
onrampvm build/intermediate/cc/cc.oe \
    -with-cpp=build/intermediate/cpp-1-omc/cpp.oe \
    -with-cci=build/intermediate/cci-1-opc/cci.oe \
    -with-as=build/intermediate/as-1-compound/as.oe \
    -c core/libo/1-opc/src/libo-util.c \
    -o build/intermediate/libo-1-opc/libo-util.oo

echo Compiling libo-vector.c
onrampvm build/intermediate/cc/cc.oe \
    -with-cpp=build/intermediate/cpp-1-omc/cpp.oe \
    -with-cci=build/intermediate/cci-1-opc/cci.oe \
    -with-as=build/intermediate/as-1-compound/as.oe \
    -c core/libo/1-opc/src/libo-vector.c \
    -o build/intermediate/libo-1-opc/libo-vector.oo

echo Archiving libo
onrampvm build/intermediate/ar-0-cat/ar.oe \
    rc build/intermediate/libo-1-opc/libo.oa \
        build/intermediate/libo-1-opc/libo-error.oo \
        build/intermediate/libo-1-opc/libo-string.oo \
        build/intermediate/libo-1-opc/libo-table.oo \
        build/intermediate/libo-1-opc/libo-util.oo \
        build/intermediate/libo-1-opc/libo-vector.oo

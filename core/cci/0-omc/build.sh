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
mkdir -p build/intermediate/cci-0-omc

echo Assembling cci/0-omc common.os
onrampvm build/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/common.os -o build/intermediate/cci-0-omc/common.oo

echo Assembling cci/0-omc compile.os
onrampvm build/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/compile.os -o build/intermediate/cci-0-omc/compile.oo

echo Assembling cci/0-omc emit.os
onrampvm build/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/emit.os -o build/intermediate/cci-0-omc/emit.oo

echo Assembling cci/0-omc globals.os
onrampvm build/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/globals.os -o build/intermediate/cci-0-omc/globals.oo

echo Assembling cci/0-omc lexer.os
onrampvm build/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/lexer.os -o build/intermediate/cci-0-omc/lexer.oo

echo Assembling cci/0-omc locals.os
onrampvm build/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/locals.os -o build/intermediate/cci-0-omc/locals.oo

echo Assembling cci/0-omc main.os
onrampvm build/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/main.os -o build/intermediate/cci-0-omc/main.oo

echo Assembling cci/0-omc parse.os
onrampvm build/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/parse.os -o build/intermediate/cci-0-omc/parse.oo

echo Assembling cci/0-omc type.os
onrampvm build/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/type.os -o build/intermediate/cci-0-omc/type.oo

echo Linking cci/0-omc
onrampvm build/intermediate/ld-0-global/ld.oe \
    build/intermediate/libc-0-oo/libc.oa \
    build/intermediate/libo-0-oo/libo.oa \
    build/intermediate/cci-0-omc/common.oo \
    build/intermediate/cci-0-omc/compile.oo \
    build/intermediate/cci-0-omc/emit.oo \
    build/intermediate/cci-0-omc/globals.oo \
    build/intermediate/cci-0-omc/lexer.oo \
    build/intermediate/cci-0-omc/locals.oo \
    build/intermediate/cci-0-omc/main.oo \
    build/intermediate/cci-0-omc/parse.oo \
    build/intermediate/cci-0-omc/type.oo \
    -o build/intermediate/cci-0-omc/cci.oe

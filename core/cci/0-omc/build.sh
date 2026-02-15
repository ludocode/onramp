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
mkdir -p output/intermediate/cci-0-omc

echo
echo === Building cci/0-omc

echo Assembling cci/0-omc common.os
onrampvm output/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/common.os -o output/intermediate/cci-0-omc/common.oo

echo Assembling cci/0-omc compile.os
onrampvm output/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/compile.os -o output/intermediate/cci-0-omc/compile.oo

echo Assembling cci/0-omc emit.os
onrampvm output/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/emit.os -o output/intermediate/cci-0-omc/emit.oo

echo Assembling cci/0-omc globals.os
onrampvm output/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/globals.os -o output/intermediate/cci-0-omc/globals.oo

echo Assembling cci/0-omc lexer.os
onrampvm output/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/lexer.os -o output/intermediate/cci-0-omc/lexer.oo

echo Assembling cci/0-omc locals.os
onrampvm output/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/locals.os -o output/intermediate/cci-0-omc/locals.oo

echo Assembling cci/0-omc main.os
onrampvm output/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/main.os -o output/intermediate/cci-0-omc/main.oo

echo Assembling cci/0-omc parse.os
onrampvm output/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/parse.os -o output/intermediate/cci-0-omc/parse.oo

echo Assembling cci/0-omc type.os
onrampvm output/intermediate/as-1-compound/as.oe \
    core/cci/0-omc/src/type.os -o output/intermediate/cci-0-omc/type.oo

echo Linking cci/0-omc
onrampvm output/intermediate/ld-0-global/ld.oe \
    output/intermediate/libc-0-oo/libc.oa \
    output/intermediate/libo-0-oo/libo.oa \
    output/intermediate/cci-0-omc/common.oo \
    output/intermediate/cci-0-omc/compile.oo \
    output/intermediate/cci-0-omc/emit.oo \
    output/intermediate/cci-0-omc/globals.oo \
    output/intermediate/cci-0-omc/lexer.oo \
    output/intermediate/cci-0-omc/locals.oo \
    output/intermediate/cci-0-omc/main.oo \
    output/intermediate/cci-0-omc/parse.oo \
    output/intermediate/cci-0-omc/type.oo \
    -o output/intermediate/cci-0-omc/cci.oe

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
mkdir -p output/intermediate/as-1-compound

echo
echo === Building as/1-compound

echo Assembling as/1-compound emit.os
onrampvm output/intermediate/as-0-basic/as.oe \
    core/as/1-compound/src/emit.os -o output/intermediate/as-1-compound/emit.oo

echo Assembling as/1-compound main.os
onrampvm output/intermediate/as-0-basic/as.oe \
    core/as/1-compound/src/main.os -o output/intermediate/as-1-compound/main.oo

echo Assembling as/1-compound op_arithmetic.os
onrampvm output/intermediate/as-0-basic/as.oe \
    core/as/1-compound/src/op_arithmetic.os -o output/intermediate/as-1-compound/op_arithmetic.oo

echo Assembling as/1-compound op_control.os
onrampvm output/intermediate/as-0-basic/as.oe \
    core/as/1-compound/src/op_control.os -o output/intermediate/as-1-compound/op_control.oo

echo Assembling as/1-compound op_logic.os
onrampvm output/intermediate/as-0-basic/as.oe \
    core/as/1-compound/src/op_logic.os -o output/intermediate/as-1-compound/op_logic.oo

echo Assembling as/1-compound op_memory.os
onrampvm output/intermediate/as-0-basic/as.oe \
    core/as/1-compound/src/op_memory.os -o output/intermediate/as-1-compound/op_memory.oo

echo Assembling as/1-compound opcodes.os
onrampvm output/intermediate/as-0-basic/as.oe \
    core/as/1-compound/src/opcodes.os -o output/intermediate/as-1-compound/opcodes.oo

echo Assembling as/1-compound parse.os
onrampvm output/intermediate/as-0-basic/as.oe \
    core/as/1-compound/src/parse.os -o output/intermediate/as-1-compound/parse.oo

echo Linking as/1-compound
onrampvm output/intermediate/ld-0-global/ld.oe \
    output/intermediate/libc-0-oo/libc.oa \
    output/intermediate/libo-0-oo/libo.oa \
    output/intermediate/as-1-compound/emit.oo \
    output/intermediate/as-1-compound/main.oo \
    output/intermediate/as-1-compound/op_arithmetic.oo \
    output/intermediate/as-1-compound/op_control.oo \
    output/intermediate/as-1-compound/op_logic.oo \
    output/intermediate/as-1-compound/op_memory.oo \
    output/intermediate/as-1-compound/opcodes.oo \
    output/intermediate/as-1-compound/parse.oo \
    -o output/intermediate/as-1-compound/as.oe

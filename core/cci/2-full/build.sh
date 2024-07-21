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
mkdir -p build/intermediate/cci-2-full

echo
echo === Building cci/2-full

echo Compiling cci/2-full block.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/block.c \
    -o build/intermediate/cci-2-full/block.oo

echo Compiling cci/2-full common.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/common.c \
    -o build/intermediate/cci-2-full/common.oo

echo Compiling cci/2-full emit.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/emit.c \
    -o build/intermediate/cci-2-full/emit.oo

echo Compiling cci/2-full enum.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/enum.c \
    -o build/intermediate/cci-2-full/enum.oo

echo Compiling cci/2-full function.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/function.c \
    -o build/intermediate/cci-2-full/function.oo

echo Compiling cci/2-full generate.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/generate.c \
    -o build/intermediate/cci-2-full/generate.oo

echo Compiling cci/2-full generate_ops.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/generate_ops.c \
    -o build/intermediate/cci-2-full/generate_ops.oo

echo Compiling cci/2-full instruction.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/instruction.c \
    -o build/intermediate/cci-2-full/instruction.oo

echo Compiling cci/2-full lexer.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/lexer.c \
    -o build/intermediate/cci-2-full/lexer.oo

echo Compiling cci/2-full llong.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/llong.c \
    -o build/intermediate/cci-2-full/llong.oo

echo Compiling cci/2-full main.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/main.c \
    -o build/intermediate/cci-2-full/main.oo

echo Compiling cci/2-full node.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/node.c \
    -o build/intermediate/cci-2-full/node.oo

echo Compiling cci/2-full options.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/options.c \
    -o build/intermediate/cci-2-full/options.oo

echo Compiling cci/2-full parse_decl.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/parse_decl.c \
    -o build/intermediate/cci-2-full/parse_decl.oo

echo Compiling cci/2-full parse_expr.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/parse_expr.c \
    -o build/intermediate/cci-2-full/parse_expr.oo

echo Compiling cci/2-full parse_stmt.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/parse_stmt.c \
    -o build/intermediate/cci-2-full/parse_stmt.oo

echo Compiling cci/2-full record.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/record.c \
    -o build/intermediate/cci-2-full/record.oo

echo Compiling cci/2-full scope.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/scope.c \
    -o build/intermediate/cci-2-full/scope.oo

echo Compiling cci/2-full strings.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/strings.c \
    -o build/intermediate/cci-2-full/strings.oo

echo Compiling cci/2-full symbol.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/symbol.c \
    -o build/intermediate/cci-2-full/symbol.oo

echo Compiling cci/2-full token.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/token.c \
    -o build/intermediate/cci-2-full/token.oo

echo Compiling cci/2-full type.c
onrampvm build/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/type.c \
    -o build/intermediate/cci-2-full/type.oo

echo Linking cci/2-full
onrampvm build/intermediate/ld-2-full/ld.oe \
    build/intermediate/libc-2-opc/libc.oa \
    build/intermediate/libo-1-opc/libo.oa \
    build/intermediate/cci-2-full/block.oo \
    build/intermediate/cci-2-full/common.oo \
    build/intermediate/cci-2-full/emit.oo \
    build/intermediate/cci-2-full/function.oo \
    build/intermediate/cci-2-full/generate.oo \
    build/intermediate/cci-2-full/generate_ops.oo \
    build/intermediate/cci-2-full/instruction.oo \
    build/intermediate/cci-2-full/lexer.oo \
    build/intermediate/cci-2-full/llong.oo \
    build/intermediate/cci-2-full/main.oo \
    build/intermediate/cci-2-full/node.oo \
    build/intermediate/cci-2-full/options.oo \
    build/intermediate/cci-2-full/parse_decl.oo \
    build/intermediate/cci-2-full/parse_expr.oo \
    build/intermediate/cci-2-full/parse_stmt.oo \
    build/intermediate/cci-2-full/record.oo \
    build/intermediate/cci-2-full/scope.oo \
    build/intermediate/cci-2-full/strings.oo \
    build/intermediate/cci-2-full/symbol.oo \
    build/intermediate/cci-2-full/token.oo \
    build/intermediate/cci-2-full/type.oo \
    -o build/intermediate/cci-2-full/cci.oe

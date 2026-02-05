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
mkdir -p output/intermediate/cci-2-full

echo
echo === Building cci/2-full

echo Compiling cci/2-full arithmetic.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/arithmetic.c \
    -o output/intermediate/cci-2-full/arithmetic.oo

echo Compiling cci/2-full block.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/block.c \
    -o output/intermediate/cci-2-full/block.oo

echo Compiling cci/2-full common.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/common.c \
    -o output/intermediate/cci-2-full/common.oo

echo Compiling cci/2-full emit.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/emit.c \
    -o output/intermediate/cci-2-full/emit.oo

echo Compiling cci/2-full enum.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/enum.c \
    -o output/intermediate/cci-2-full/enum.oo

echo Compiling cci/2-full function.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/function.c \
    -o output/intermediate/cci-2-full/function.oo

echo Compiling cci/2-full generate.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/generate.c \
    -o output/intermediate/cci-2-full/generate.oo

echo Compiling cci/2-full generate_ops.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/generate_ops.c \
    -o output/intermediate/cci-2-full/generate_ops.oo

echo Compiling cci/2-full generate_stmt.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/generate_stmt.c \
    -o output/intermediate/cci-2-full/generate_stmt.oo

echo Compiling cci/2-full instruction.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/instruction.c \
    -o output/intermediate/cci-2-full/instruction.oo

echo Compiling cci/2-full lexer.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/lexer.c \
    -o output/intermediate/cci-2-full/lexer.oo

echo Compiling cci/2-full main.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/main.c \
    -o output/intermediate/cci-2-full/main.oo

echo Compiling cci/2-full node.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/node.c \
    -o output/intermediate/cci-2-full/node.oo

echo Compiling cci/2-full optimize_asm.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/optimize_asm.c \
    -o output/intermediate/cci-2-full/optimize_asm.oo

echo Compiling cci/2-full optimize_tree.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/optimize_tree.c \
    -o output/intermediate/cci-2-full/optimize_tree.oo

echo Compiling cci/2-full options.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/options.c \
    -o output/intermediate/cci-2-full/options.oo

echo Compiling cci/2-full parse_decl.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/parse_decl.c \
    -o output/intermediate/cci-2-full/parse_decl.oo

echo Compiling cci/2-full parse_expr.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/parse_expr.c \
    -o output/intermediate/cci-2-full/parse_expr.oo

echo Compiling cci/2-full parse_init.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/parse_init.c \
    -o output/intermediate/cci-2-full/parse_init.oo

echo Compiling cci/2-full parse_stmt.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/parse_stmt.c \
    -o output/intermediate/cci-2-full/parse_stmt.oo

echo Compiling cci/2-full record.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/record.c \
    -o output/intermediate/cci-2-full/record.oo

echo Compiling cci/2-full scope.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/scope.c \
    -o output/intermediate/cci-2-full/scope.oo

echo Compiling cci/2-full strings.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/strings.c \
    -o output/intermediate/cci-2-full/strings.oo

echo Compiling cci/2-full symbol.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/symbol.c \
    -o output/intermediate/cci-2-full/symbol.oo

echo Compiling cci/2-full token.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/token.c \
    -o output/intermediate/cci-2-full/token.oo

echo Compiling cci/2-full type.c
onrampvm output/intermediate/cc/cc.oe \
    @core/cci/2-full/build-ccargs \
    -c core/cci/2-full/src/type.c \
    -o output/intermediate/cci-2-full/type.oo

echo Linking cci/2-full
onrampvm output/intermediate/ld-2-full/ld.oe \
    -g \
    output/intermediate/libc-2-opc/libc.oa \
    output/intermediate/libo-1-opc/libo.oa \
    output/intermediate/cci-2-full/arithmetic.oo \
    output/intermediate/cci-2-full/block.oo \
    output/intermediate/cci-2-full/common.oo \
    output/intermediate/cci-2-full/emit.oo \
    output/intermediate/cci-2-full/enum.oo \
    output/intermediate/cci-2-full/function.oo \
    output/intermediate/cci-2-full/generate.oo \
    output/intermediate/cci-2-full/generate_ops.oo \
    output/intermediate/cci-2-full/generate_stmt.oo \
    output/intermediate/cci-2-full/instruction.oo \
    output/intermediate/cci-2-full/lexer.oo \
    output/intermediate/cci-2-full/main.oo \
    output/intermediate/cci-2-full/node.oo \
    output/intermediate/cci-2-full/optimize_asm.oo \
    output/intermediate/cci-2-full/optimize_tree.oo \
    output/intermediate/cci-2-full/options.oo \
    output/intermediate/cci-2-full/parse_decl.oo \
    output/intermediate/cci-2-full/parse_expr.oo \
    output/intermediate/cci-2-full/parse_init.oo \
    output/intermediate/cci-2-full/parse_stmt.oo \
    output/intermediate/cci-2-full/record.oo \
    output/intermediate/cci-2-full/scope.oo \
    output/intermediate/cci-2-full/strings.oo \
    output/intermediate/cci-2-full/symbol.oo \
    output/intermediate/cci-2-full/token.oo \
    output/intermediate/cci-2-full/type.oo \
    -o output/intermediate/cci-2-full/cci.oe

#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2024 Fraser Heavy Software
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
mkdir -p build/intermediate/cci-2-full-re

echo
echo === Rebuilding cci/2-full

echo Compiling cci/2-full arithmetic.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/arithmetic.c \
    -o build/intermediate/cci-2-full-re/arithmetic.oo

echo Compiling cci/2-full block.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/block.c \
    -o build/intermediate/cci-2-full-re/block.oo

echo Compiling cci/2-full common.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/common.c \
    -o build/intermediate/cci-2-full-re/common.oo

echo Compiling cci/2-full emit.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/emit.c \
    -o build/intermediate/cci-2-full-re/emit.oo

echo Compiling cci/2-full enum.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/enum.c \
    -o build/intermediate/cci-2-full-re/enum.oo

echo Compiling cci/2-full function.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/function.c \
    -o build/intermediate/cci-2-full-re/function.oo

echo Compiling cci/2-full generate.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/generate.c \
    -o build/intermediate/cci-2-full-re/generate.oo

echo Compiling cci/2-full generate_ops.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/generate_ops.c \
    -o build/intermediate/cci-2-full-re/generate_ops.oo

echo Compiling cci/2-full generate_stmt.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/generate_stmt.c \
    -o build/intermediate/cci-2-full-re/generate_stmt.oo

echo Compiling cci/2-full instruction.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/instruction.c \
    -o build/intermediate/cci-2-full-re/instruction.oo

echo Compiling cci/2-full lexer.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/lexer.c \
    -o build/intermediate/cci-2-full-re/lexer.oo

echo Compiling cci/2-full main.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/main.c \
    -o build/intermediate/cci-2-full-re/main.oo

echo Compiling cci/2-full node.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/node.c \
    -o build/intermediate/cci-2-full-re/node.oo

echo Compiling cci/2-full optimize_asm.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/optimize_asm.c \
    -o build/intermediate/cci-2-full-re/optimize_asm.oo

echo Compiling cci/2-full optimize_tree.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/optimize_tree.c \
    -o build/intermediate/cci-2-full-re/optimize_tree.oo

echo Compiling cci/2-full options.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/options.c \
    -o build/intermediate/cci-2-full-re/options.oo

echo Compiling cci/2-full parse_decl.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/parse_decl.c \
    -o build/intermediate/cci-2-full-re/parse_decl.oo

echo Compiling cci/2-full parse_expr.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/parse_expr.c \
    -o build/intermediate/cci-2-full-re/parse_expr.oo

echo Compiling cci/2-full parse_init.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/parse_init.c \
    -o build/intermediate/cci-2-full-re/parse_init.oo

echo Compiling cci/2-full parse_stmt.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/parse_stmt.c \
    -o build/intermediate/cci-2-full-re/parse_stmt.oo

echo Compiling cci/2-full record.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/record.c \
    -o build/intermediate/cci-2-full-re/record.oo

echo Compiling cci/2-full scope.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/scope.c \
    -o build/intermediate/cci-2-full-re/scope.oo

echo Compiling cci/2-full strings.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/strings.c \
    -o build/intermediate/cci-2-full-re/strings.oo

echo Compiling cci/2-full symbol.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/symbol.c \
    -o build/intermediate/cci-2-full-re/symbol.oo

echo Compiling cci/2-full token.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/token.c \
    -o build/intermediate/cci-2-full-re/token.oo

echo Compiling cci/2-full type.c
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    -c core/cci/2-full/src/type.c \
    -o build/intermediate/cci-2-full-re/type.oo

echo Linking cci/2-full
onrampvm build/output/bin/cc.oe \
    @core/cci/2-full/rebuild-ccargs \
    build/intermediate/libo-1-opc-re/libo.oa \
    build/intermediate/cci-2-full-re/arithmetic.oo \
    build/intermediate/cci-2-full-re/block.oo \
    build/intermediate/cci-2-full-re/common.oo \
    build/intermediate/cci-2-full-re/emit.oo \
    build/intermediate/cci-2-full-re/enum.oo \
    build/intermediate/cci-2-full-re/function.oo \
    build/intermediate/cci-2-full-re/generate.oo \
    build/intermediate/cci-2-full-re/generate_ops.oo \
    build/intermediate/cci-2-full-re/generate_stmt.oo \
    build/intermediate/cci-2-full-re/instruction.oo \
    build/intermediate/cci-2-full-re/lexer.oo \
    build/intermediate/cci-2-full-re/main.oo \
    build/intermediate/cci-2-full-re/node.oo \
    build/intermediate/cci-2-full-re/optimize_asm.oo \
    build/intermediate/cci-2-full-re/optimize_tree.oo \
    build/intermediate/cci-2-full-re/options.oo \
    build/intermediate/cci-2-full-re/parse_decl.oo \
    build/intermediate/cci-2-full-re/parse_expr.oo \
    build/intermediate/cci-2-full-re/parse_init.oo \
    build/intermediate/cci-2-full-re/parse_stmt.oo \
    build/intermediate/cci-2-full-re/record.oo \
    build/intermediate/cci-2-full-re/scope.oo \
    build/intermediate/cci-2-full-re/strings.oo \
    build/intermediate/cci-2-full-re/symbol.oo \
    build/intermediate/cci-2-full-re/token.oo \
    build/intermediate/cci-2-full-re/type.oo \
    -o build/output/bin/cci.oe

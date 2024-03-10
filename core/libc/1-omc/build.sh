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
mkdir -p build/intermediate/libc-1-omc

# TODO it would be nice to build cc before this stage of libc (and even before
# ld/1), we could make this file much shorter (or even get rid of this stage
# libc entirely.)



echo Preprocessing libc/1-omc stdlib_malloc.c
onrampvm build/intermediate/cpp-1-omc/cpp.oe \
    -D__onramp__=1 \
    -D__onramp_cci__=1 -D__onramp_cci_omc__=1 \
    -D__onramp_cpp__=1 -D__onramp_cpp_omc__=1 \
    -D__onramp_libc__=1 -D__onramp_libc_omc__=1 \
    -Drestrict= -D_Noreturn= \
    -Icore/libc/1-omc/include \
    -Icore/libc/0-oo/include \
    -Icore/libo/0-oo/include \
    -include __onramp/__predef.h \
    core/libc/1-omc/src/stdlib_malloc.c -o build/intermediate/libc-1-omc/stdlib_malloc.i

echo Compiling libc/1-omc stdlib_malloc.c
onrampvm build/intermediate/cci-0-omc/cci.oe \
    build/intermediate/libc-1-omc/stdlib_malloc.i -o build/intermediate/libc-1-omc/stdlib_malloc.os

echo Assembling libc/1-omc stdlib_malloc.c
onrampvm build/intermediate/as-1-compound/as.oe \
    build/intermediate/libc-1-omc/stdlib_malloc.os -o build/intermediate/libc-1-omc/stdlib_malloc.oo



echo Preprocessing libc/1-omc stdlib_conv.c
onrampvm build/intermediate/cpp-1-omc/cpp.oe \
    -D__onramp__=1 \
    -D__onramp_cci__=1 -D__onramp_cci_omc__=1 \
    -D__onramp_cpp__=1 -D__onramp_cpp_omc__=1 \
    -D__onramp_libc__=1 -D__onramp_libc_omc__=1 \
    -Drestrict= -D_Noreturn= \
    -Icore/libc/1-omc/include \
    -Icore/libc/0-oo/include \
    -Icore/libo/0-oo/include \
    -include __onramp/__predef.h \
    core/libc/1-omc/src/stdlib_conv.c -o build/intermediate/libc-1-omc/stdlib_conv.i

echo Compiling libc/1-omc stdlib_conv.c
onrampvm build/intermediate/cci-0-omc/cci.oe \
    build/intermediate/libc-1-omc/stdlib_conv.i -o build/intermediate/libc-1-omc/stdlib_conv.os

echo Assembling libc/1-omc stdlib_conv.c
onrampvm build/intermediate/as-1-compound/as.oe \
    build/intermediate/libc-1-omc/stdlib_conv.os -o build/intermediate/libc-1-omc/stdlib_conv.oo



echo Preprocessing libc/1-omc string_omc.c
onrampvm build/intermediate/cpp-1-omc/cpp.oe \
    -D__onramp__=1 \
    -D__onramp_cci__=1 -D__onramp_cci_omc__=1 \
    -D__onramp_cpp__=1 -D__onramp_cpp_omc__=1 \
    -D__onramp_libc__=1 -D__onramp_libc_omc__=1 \
    -Drestrict= -D_Noreturn= \
    -Icore/libc/1-omc/include \
    -Icore/libc/0-oo/include \
    -Icore/libo/0-oo/include \
    -include __onramp/__predef.h \
    core/libc/1-omc/src/string_omc.c -o build/intermediate/libc-1-omc/string_omc.i

echo Compiling libc/1-omc string_omc.c
onrampvm build/intermediate/cci-0-omc/cci.oe \
    build/intermediate/libc-1-omc/string_omc.i -o build/intermediate/libc-1-omc/string_omc.os

echo Assembling libc/1-omc string_omc.c
onrampvm build/intermediate/as-1-compound/as.oe \
    build/intermediate/libc-1-omc/string_omc.os -o build/intermediate/libc-1-omc/string_omc.oo



# The stage 1 libc is an overlay over stage 0. We exclude a few files from
# stage 0 and include all of the files from stage 1.
echo Archiving libc/1-omc
onrampvm build/intermediate/ar-0-cat/ar.oe \
    rc build/intermediate/libc-1-omc/libc.oa \
        core/libc/0-oo/src/start.oo \
        core/libc/0-oo/src/ctype.oo \
        core/libc/0-oo/src/errno.oo \
        core/libc/0-oo/src/spawn.oo \
        core/libc/0-oo/src/stdio.oo \
        core/libc/0-oo/src/stdlib_util.oo \
        core/libc/0-oo/src/string.oo \
        build/intermediate/libc-1-omc/stdlib_malloc.oo \
        build/intermediate/libc-1-omc/stdlib_conv.oo \
        build/intermediate/libc-1-omc/string_omc.oo

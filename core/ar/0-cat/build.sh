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
mkdir -p build/intermediate/ar-0-cat

echo
echo === Building ar/0-cat

# Our libraries don't exist as archives yet (because this archiver doesn't
# exist yet!) We have to link all the libc and libo object files manually.
# Note: start.oo must come first!

echo Linking ar/0-cat
onrampvm build/intermediate/ld-0-global/ld.oe \
    \
    core/libc/0-oo/src/start.oo \
    \
    core/libc/0-oo/src/ctype.oo \
    core/libc/0-oo/src/environ.oo \
    core/libc/0-oo/src/errno.oo \
    core/libc/0-oo/src/malloc.oo \
    core/libc/0-oo/src/malloc_util.oo \
    core/libc/0-oo/src/spawn.oo \
    core/libc/0-oo/src/stdio.oo \
    core/libc/0-oo/src/string.oo \
    \
    core/libo/0-oo/src/libo-error.oo \
    core/libo/0-oo/src/libo-util.oo \
    \
    core/ar/0-cat/ar.oo \
    \
    -o build/intermediate/ar-0-cat/ar.oe

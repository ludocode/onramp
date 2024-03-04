#!/bin/sh

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

# This is an implementation of the Onramp hex program that wraps xxd to perform
# the actual hex conversion. xxd is originally part of Vim, although there are
# other implementations, e.g. in BusyBox/toybox.
#
# Note that xxd ignores invalid characters and allows whitespace between
# characters in a hex byte. Better implementations of the Onramp hex tool
# diagnose such errors.
#
# Consider using the pure POSIX shell implementation in sh/ instead. The pure
# POSIX shell hex tool performs address assertions, detects invalid hex bytes,
# and gives error messages with line numbers.

if [ "$1" = "-o" ]; then
    output="$2"
    input="$3"
else
    input="$1"
    output="$3"
fi

sed 's/[;@#].*//' < "$input" | xxd -r -p > "$output"

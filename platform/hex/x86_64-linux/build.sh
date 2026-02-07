#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2023-2026 Fraser Heavy Software
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


# This script builds the x86_64-linux hex tool.
#
# This hex tool is written in hex which means it requires a hex tool to
# configure it. This makes it somewhat redundant.
#
# It can speed up the build however because the POSIX shell is very slow, so it
# may be quicker to hex this and then use it to hex a VM than it is to hex a VM
# from shell. We only bother to try the POSIX shell tool to build it.


set -e

# Use a shell hex tool if it's configured, otherwise just fall back to the
# shell hex tool. We don't want to add any other dependencies.
if [ -e output/posix/bin/onramphex ]; then
    HEX=output/posix/bin/onramphex
else
    HEX=platform/hex/sh/hex.sh
fi

# build
mkdir -p output/intermediate/hex-x86_64-linux
echo "Hexing x86_64-linux hex.ohx  (with: \`$HEX\`)"
$HEX platform/hex/x86_64-linux/hex.ohx -o output/intermediate/hex-x86_64-linux/hex
chmod +x output/intermediate/hex-x86_64-linux/hex
echo "Wrote: output/intermediate/hex-x86_64-linux/hex"

#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2026 Fraser Heavy Software
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


# This script configures the x86_64-linux hex tool.
#
# This hex tool is written in hex which means it requires a hex tool to
# configure it. This makes it somewhat redundant. It would only really be
# necessary on a Linux kernel that doesn't have a POSIX shell, and since this
# script is written in POSIX shell, it would have to be configured manually.
#
# It can speed up the build however because the POSIX shell is very slow, so it
# is quicker to hex this and then use it to hex a VM than it is to hex a VM
# from shell. We only bother to try the POSIX shell tool to build it.


set -e

if [ "$(uname -m)" != "x86_64" ] || [ "$(uname -s)" != "Linux" ]; then
    echo "$0: This is not x86_64 Linux."
    exit 1
fi

if ! [ -e platform/hex/x86_64-linux/hex.ohx ]; then
    echo "$0: ERROR: platform/hex/x86_64-linux/hex.ohx not found."
    echo "$0: This script must be run in the root of the repository or build directory."
    exit 1
fi

mkdir -p output/posix/share/onramp/platform
platform/hex/sh/hex.sh platform/hex/x86_64-linux/hex.ohx -o output/posix/share/onramp/platform/hex-x86_64-linux
chmod +x output/posix/share/onramp/platform/hex-x86_64-linux
(cd output/posix/bin; ln -sf ../share/onramp/platform/hex-x86_64-linux onramphex)

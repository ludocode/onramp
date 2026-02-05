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


# This script configures an alternate POSIX shell hex tool on POSIX platforms.
#
# We only bother to setup hexsplit.sh under the assumption that it has the
# least bad worst case performance. If it doesn't work the others probably
# won't either.


set -e

if ! [ -e platform/hex/sh-alt/hexsplit.sh ]; then
    echo "$0: ERROR: platform/hex/sh-alt/hexsplit.sh not found."
    echo "$0: This script must be run in the root of the repository or build directory."
    exit 1
fi

mkdir -p output/posix/share/onramp/platform
cp platform/hex/sh-alt/hexsplit.sh output/posix/share/onramp/platform/
(cd output/posix/bin; ln -sf ../share/onramp/platform/hexsplit.sh onramphex)

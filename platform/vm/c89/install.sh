#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2025 Fraser Heavy Software
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


# This script installs the compiled VM into the build output folder as the
# `onrampvm` binary for POSIX systems.


set -e
cd "$(dirname "$0")/../../.."

if ! [ -e build/test/vm-c89/vm ]; then
    echo "$0: ERROR: VM not built yet. Run platform/vm/c89/build.sh" >&1
    exit 1
fi

mkdir -p build/posix/share/onramp/platform
cp build/test/vm-c89/vm build/posix/share/onramp/platform/vm-c89
(cd build/posix/bin; ln -sf ../share/onramp/platform/vm-c89 onrampvm)

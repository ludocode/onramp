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

# This script installs Onramp into ~/.local.

set -e
cd "$(dirname "$0")/../.."

check_path() {
    if ! [ -e $1 ]; then
        echo "ERROR: $1 is missing. Did you build Onramp?"
        exit 1
    fi
}

# Check that a bunch of files exist to make sure the bootstrap process worked
check_path build/posix/bin/onrampvm
check_path build/posix/bin/onrampcc
check_path build/posix/bin/onrampar
check_path build/posix/bin/onramphex
check_path build/posix/share/onramp
check_path build/output/bin/cc.oe
check_path build/output/bin/cci.oe
check_path build/output/bin/ar.oe
check_path build/output/bin/hex.oe
check_path build/output/include/stdlib.h
check_path build/output/lib/libc.oa

# Copy share/
rm -rf $HOME/.local/share/onramp
mkdir -p $HOME/.local/share/onramp
cp -Lr build/posix/share/onramp/* $HOME/.local/share/onramp
echo "Installed ~/.local/share/onramp/"

# Copy bin/
mkdir -p $HOME/.local/bin
cp build/posix/bin/onramphex $HOME/.local/bin/onramphex  ; echo "Installed ~/.local/bin/onramphex"
cp build/posix/bin/onrampvm  $HOME/.local/bin/onrampvm   ; echo "Installed ~/.local/bin/onrampvm"
cp build/posix/bin/onrampcc  $HOME/.local/bin/onrampcc   ; echo "Installed ~/.local/bin/onrampcc"
cp build/posix/bin/onrampar  $HOME/.local/bin/onrampar   ; echo "Installed ~/.local/bin/onrampar"

echo
echo "Done!"
echo
echo "Note: You need ~/.local/bin on your PATH to use the installed Onramp."
echo "Note: Run scripts/posix/uninstall.sh to uninstall."
echo

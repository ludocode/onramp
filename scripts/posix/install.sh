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

# This script installs Onramp into ~/.local.
#
# If --dev is passed, it installs symlinks to the build folder instead of
# copying files.

set -e

check_path() {
    if ! [ -e $1 ]; then
        echo "ERROR: $1 is missing. Did you build Onramp?"
        exit 1
    fi
}

DEV=0
if [ "$1" = "--dev" ]; then
    DEV=1
    shift
fi
if [ "x$1" != "x" ]; then
    echo "ERROR: Unrecognized option: $1"
    exit 1
fi

# Check that a bunch of files exist to make sure the bootstrap process worked
# (Unless --dev is specified, in which case we allow installing links even if
# the compiler is not built yet)
if [ $DEV -ne 1 ]; then
    check_path output/posix/bin/onrampvm
    check_path output/posix/bin/onrampcc
    check_path output/posix/bin/onrampar
    check_path output/posix/bin/onramphex
    check_path output/posix/share/onramp
    check_path output/final/bin/cc*
    check_path output/final/bin/cci*
    check_path output/final/bin/ar*
    check_path output/final/bin/hex*
    check_path output/final/include/stdlib.h
    check_path output/final/lib/libc.oa
fi

# Remove old installation (if any)
rm -rf $HOME/.local/share/onramp
rm -f $HOME/.local/bin/onramphex
rm -f $HOME/.local/bin/onrampvm
rm -f $HOME/.local/bin/onrampcc
rm -f $HOME/.local/bin/onrampar

# Make directories
mkdir -p $HOME/.local/share
mkdir -p $HOME/.local/bin

if [ $DEV -eq 1 ]; then

    # Link share/
    ln -sf "$(pwd)/output/posix/share/onramp" $HOME/.local/share/onramp
    echo "Linked ~/.local/share/onramp/ -> output/posix/share/onramp/"

    # Link bin/
    ln -sf "$(pwd)/output/posix/bin/onramphex" $HOME/.local/bin/onramphex  ; echo "Linked ~/.local/bin/onramphex -> output/posix/bin/onramphex"
    ln -sf "$(pwd)/output/posix/bin/onrampvm"  $HOME/.local/bin/onrampvm   ; echo "Linked ~/.local/bin/onrampvm -> output/posix/bin/onrampvm"
    ln -sf "$(pwd)/output/posix/bin/onrampcc"  $HOME/.local/bin/onrampcc   ; echo "Linked ~/.local/bin/onrampcc -> output/posix/bin/onrampcc"
    ln -sf "$(pwd)/output/posix/bin/onrampar"  $HOME/.local/bin/onrampar   ; echo "Linked ~/.local/bin/onrampar -> output/posix/bin/onrampar"

else

    # Copy share/
    cp -Lr output/posix/share/onramp $HOME/.local/share/onramp
    echo "Installed ~/.local/share/onramp/"

    # Copy bin/
    cp output/posix/bin/onramphex $HOME/.local/bin/onramphex  ; echo "Installed ~/.local/bin/onramphex"
    cp output/posix/bin/onrampvm  $HOME/.local/bin/onrampvm   ; echo "Installed ~/.local/bin/onrampvm"
    cp output/posix/bin/onrampcc  $HOME/.local/bin/onrampcc   ; echo "Installed ~/.local/bin/onrampcc"
    cp output/posix/bin/onrampar  $HOME/.local/bin/onrampar   ; echo "Installed ~/.local/bin/onrampar"

fi

echo
echo "Done!"
echo
echo "Note: You need ~/.local/bin on your PATH to use the installed Onramp."
echo "Note: Run scripts/posix/uninstall.sh to uninstall."
echo

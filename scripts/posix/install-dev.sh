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

# This script symlinks your development build of Onramp into ~/.local.
#
# This is really only useful for developing Onramp. It's meant so you can
# easily use your development build elsewhere without having to re-install it
# or source the environment.

set -e
cd "$(dirname "$0")/../.."

rm -rf $HOME/.local/share/onramp
ln -sf "$(pwd)/build/posix/share/onramp" $HOME/.local/share/onramp
echo "Linked ~/.local/share/onramp/ -> build/posix/share/onramp/"

ln -sf "$(pwd)/build/posix/bin/onramphex" $HOME/.local/bin/onramphex  ; echo "Linked ~/.local/bin/onramphex -> build/posix/bin/onramphex"
ln -sf "$(pwd)/build/posix/bin/onrampvm"  $HOME/.local/bin/onrampvm   ; echo "Linked ~/.local/bin/onrampvm -> build/posix/bin/onrampvm"
ln -sf "$(pwd)/build/posix/bin/onrampcc"  $HOME/.local/bin/onrampcc   ; echo "Linked ~/.local/bin/onrampcc -> build/posix/bin/onrampcc"
ln -sf "$(pwd)/build/posix/bin/onrampar"  $HOME/.local/bin/onrampar   ; echo "Linked ~/.local/bin/onrampar -> build/posix/bin/onrampar"

echo
echo "Done!"
echo
echo "Note: You need ~/.local/bin on your PATH to use the installed Onramp."
echo "Note: Run scripts/posix/uninstall.sh to uninstall."
echo

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

# This script uninstalls Onramp.

set -e
cd "$(dirname "$0")/../.."

echo
echo "Deleting ~/.local/bin/onrampvm"   ; rm -f "$HOME/.local/bin/onrampvm"
echo "Deleting ~/.local/bin/onrampcc"   ; rm -f "$HOME/.local/bin/onrampcc"
echo "Deleting ~/.local/bin/onrampar"   ; rm -f "$HOME/.local/bin/onrampar"
echo "Deleting ~/.local/bin/onramphex"  ; rm -f "$HOME/.local/bin/onramphex"
echo "Deleting ~/.local/share/onramp/"  ; rm -rf "$HOME/.local/share/onramp"
echo
echo "Done."

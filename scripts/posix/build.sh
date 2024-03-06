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



# This is the Onramp automatic build script for POSIX systems. It will choose
# an appropriate hex tool and virtual machine for your system and use them to
# perform the bootstrap process.
#
# See docs/bootstrap-path.md to follow what this script does.



set -e
cd "$(dirname "$0")/../.."



##################################
# Setup
##################################

# Run the POSIX setup script
sh scripts/posix/setup.sh

# If the VM isn't already on our PATH, add it
if ! command -v onrampvm > /dev/null; then
    export PATH="$(pwd)/build/posix/bin:$PATH"
fi



##################################
# Run bootstrap script
##################################

echo
echo "Entering Onramp virtual machine..."
echo

# Now that we have everything we need we can jump inside the VM for the rest
# of the bootstrap process.
# TODO since our sh tool is incomplete we run this script directly.
#onrampvm build/intermediate/sh/sh.oe core/build.sh
sh core/build.sh



##################################
# POSIX-specific finishing touches
##################################

cp platform/cc/posix/onrampcc build/posix/bin
cp platform/cc/posix/onrampar build/posix/bin
cp platform/cc/posix/onramphex build/posix/bin
cp platform/cc/posix/wrap-header build/posix/share/onramp/platform



##################################
# Cleanup
##################################

echo
echo "POSIX build complete."
echo

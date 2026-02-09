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


# This is the old Onramp build script. It is deprecated. This is now just a
# wrapper for the top-level `configure.sh` and the `build.sh` it generates.


set -e
cd "$(dirname "$0")/../.."

usage() {
    echo "This script is deprecated. Use ./configure.sh instead."
    echo "See the setup guide (docs/setup-guide.md) for details."
}

# convert old build options to new configure options
OPTS=""
SETUP=0
while [ $# -ne 0 ]; do
    case "$1" in
        --dev) OPTS="$OPTS --dev"; shift ;;
        --min) OPTS="$OPTS --strict"; shift ;;
        --hex) OPTS="$OPTS --hex $2"; shift; shift ;;
        --vm) OPTS="$OPTS --vm $2"; shift; shift ;;
        --setup) SETUP=1; shift ;;
        --help) usage; exit 0 ;;
        *)
            echo "ERROR: unrecognized option: $1"
            usage
            exit 1
            ;;
    esac
done

./configure.sh $OPTS
if [ $SETUP -eq 0 ]; then
    ./build.sh
fi

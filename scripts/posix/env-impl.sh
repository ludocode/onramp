#!/bin/false

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


# This script is the implementation of the env.sh generated from the
# configure script.
#
# It is meant to be sourced (as is env.sh) so it is not executable.


ONRAMP_ENV_ADD_PATH=output/posix/bin

if command -v realpath >/dev/null 2>&1; then
    # POSIX 2024
    ONRAMP_ENV_ADD_PATH=$(realpath "$ONRAMP_ENV_ADD_PATH")

elif command -v grealpath >/dev/null 2>&1; then
    # macOS brew
    ONRAMP_ENV_ADD_PATH=$(grealpath "$ONRAMP_ENV_ADD_PATH")

elif command -v readlink >/dev/null 2>&1; then
    # also POSIX 2024, but may be an extension earlier than realpath
    ONRAMP_ENV_ADD_PATH=$(readlink -f "$ONRAMP_ENV_ADD_PATH")

elif command -v greadlink >/dev/null 2>&1; then
    # probably useless since macOS brew coreutils also includes grealpath, but
    # maybe some other system has part of GNU coreutils with a g prefix
    ONRAMP_ENV_ADD_PATH=$(greadlink -f "$ONRAMP_ENV_ADD_PATH")

else
    ONRAMP_ENV_ADD_PATH="$(pwd)/$ONRAMP_ENV_ADD_PATH"
fi

# It's nicer if this is idempotent so we go through the trouble of stripping
# the path if it already exists. The path is then added to the front.
ONRAMP_ENV_PATH_STRIPPED=$(echo :$PATH: | sed "s@:@::@g" | sed "s@:$ONRAMP_ENV_ADD_PATH:@:@g" )
export PATH=$(echo $ONRAMP_ENV_ADD_PATH$ONRAMP_ENV_PATH_STRIPPED | sed "s@::*@:@g" | sed 's@:$@@')

echo "Added $ONRAMP_ENV_ADD_PATH to PATH"

# This script is sourced so we scope our vars to ONRAMP and we clean up after
# ourselves.
unset ONRAMP_ENV_ADD_PATH
unset ONRAMP_ENV_PATH_STRIPPED

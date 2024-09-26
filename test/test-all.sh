#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2024 Fraser Heavy Software
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


# This script attempts to run all tests that can be run on the local machine.
# It approximates what the Github CI does, but none of it is in parallel so it
# takes a while to run.


set -e
cd "$(dirname "$0")/.."
scripts/posix/clean.sh

# Run platform-specific tests first to make sure there are no VM bugs before we
# test stuff that depends on a VM.
test/test-platform.sh

# Test a normal build, preferring machine code if available.
# (Unlike the CI we don't specify --min here because we want what we have to be
# testable on platforms that don't have a machine code VM yet.)
scripts/posix/clean.sh
scripts/posix/build.sh
test/test-final.sh

# Test core components with the best available toolchains before testing as
# bootstrapped since they are more likely to find the real source of bugs.
scripts/posix/clean.sh
scripts/posix/build.sh --setup --dev
test/test-core.sh

# Finally we test everything as bootstrapped.
scripts/posix/clean.sh
scripts/posix/build.sh --setup --dev
test/test-bootstrap.sh

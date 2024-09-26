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


# This script tests the final toolchain after it has been bootstrapped.
#
# The POSIX build script (scripts/posix/build.sh) must already have been run.



set -e
cd "$(dirname "$0")/.."

# Put the VM on our PATH
. scripts/posix/env.sh

# Test libc
( cd test/libc/3-full && \
    ../run.sh ../0-oo  full ../../../build/output/lib/libc.oa && \
    ../run.sh ../1-omc full ../../../build/output/lib/libc.oa && \
    ../run.sh ../2-opc full ../../../build/output/lib/libc.oa && \
    ../run.sh .        full ../../../build/output/lib/libc.oa )

# Test libo
true #TODO libo tests don't exist yet

# Test cc
( cd test/cc && ./run.sh . onrampvm ../../../build/output/bin/cc.oe )

# Test ld
( cd test/ld/2-full && ../run.sh . onrampvm ../../../build/output/bin/ld.oe )

# Test as
( cd test/as/2-full && \
    ../run.sh --other-stage ../0-basic onrampvm ../../../build/output/bin/as.oe && \
    ../run.sh --other-stage ../1-compound onrampvm ../../../build/output/bin/as.oe && \
    ../run.sh . onrampvm ../../../build/output/bin/as.oe )

# Test cci
( cd test/cci/2-full && \
    ../run.sh --other-stage ../0-omc full onrampvm ../../../build/output/bin/cci.oe && \
    ../run.sh --other-stage ../1-opc full onrampvm ../../../build/output/bin/cci.oe && \
    ../run.sh               .        full onrampvm ../../../build/output/bin/cci.oe )

# test cpp
# TODO cpp/2 not done yet, only running cpp/1 tests
( cd test/cpp/1-omc && ../run.sh . onrampvm ../../../build/output/bin/cpp.oe )

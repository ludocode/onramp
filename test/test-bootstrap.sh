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

# Performs the Onramp bootstrap process, testing each core component as it is
# bootstrapped.
#
# This script matches core/build.sh except that we run tests after each step.

set -e
cd "$(dirname "$0")/.."

# Put the VM on our PATH
. scripts/posix/env.sh

# We need some of our non-bootstrapped tools built because they are used to
# compile test cases. (For example the libc test cases are written in C even
# for the first stage libc which is written in bytecode. We can't compile them
# with a bootstrapped compiler because it doesn't exist at the time libc/0 is
# built.)
make -C test/cpp/1-omc build
make -C test/cci/1-opc build
make -C test/as/2-full build
make -C test/ld/2-full build

# First we get our linker and libc up
( core/ld/0-global/build.sh && cd test/ld/0-global && ../run.sh . onrampvm ../../../build/intermediate/ld-0-global/ld.oe )
( core/ar/0-cat/build.sh && cd test/ar/0-cat && ../run.sh . onrampvm ../../../build/intermediate/ar-0-cat/ar.oe )
( core/libc/0-oo/build.sh && cd test/libc/0-oo && ../run.sh . ../../../build/intermediate/libc-0-oo/libc.oa -I../../../core/libc/0-oo/include )

# Next we build an assembler
( core/libo/0-oo/build.sh && true ) #TODO libo tests don't exist yet
( core/as/0-basic/build.sh && cd test/as/0-basic && ../run.sh . onrampvm ../../../build/intermediate/as-0-basic/as.oe )
( core/as/1-compound/build.sh && cd test/as/1-compound && \
    ../run.sh . onrampvm ../../../build/intermediate/as-1-compound/as.oe && \
    ../run.sh --other-stage ../0-basic onrampvm ../../../build/intermediate/as-1-compound/as.oe )

# Next build our omC compiler
( core/cpp/0-strip/build.sh && cd test/cpp/0-strip && ../run.sh . onrampvm ../../../build/intermediate/cpp-0-strip/cpp.oe )
( core/cci/0-omc/build.sh && cd test/cci/0-omc && ../run.sh . omc onrampvm ../../../build/intermediate/cci-0-omc/cci.oe )

# Build the rest of the omC toolchain
( core/cpp/1-omc/build.sh && cd test/cpp/1-omc && ../run.sh . onrampvm ../../../build/intermediate/cpp-1-omc/cpp.oe )
( core/ld/1-omc/build.sh && cd test/ld/1-omc && ../run.sh . onrampvm ../../../build/intermediate/ld-1-omc/ld.oe )
( core/libc/1-omc/build.sh && cd test/libc/1-omc && \
    ../run.sh . ../../../build/intermediate/libc-1-omc/libc.oa \
            -I../../../core/libc/1-omc/include \
            -I../../../core/libc/0-oo/include && \
    ../run.sh ../0-oo ../../../build/intermediate/libc-1-omc/libc.oa \
            -I../../../core/libc/1-omc/include \
            -I../../../core/libc/0-oo/include )
( core/cc/build.sh && cd test/cc && ./run.sh . onrampvm ../../../build/intermediate/cc/cc.oe )

# Build the opC toolchain
( core/cci/1-opc/build.sh && cd test/cci/1-opc && \
    ../run.sh . opc onrampvm ../../../build/intermediate/cci-1-opc/cci.oe && \
    ../run.sh --other-stage ../0-omc opc onrampvm ../../../build/intermediate/cci-1-opc/cci.oe )
( core/libc/2-opc/build.sh && cd test/libc/1-omc && \
    ../run.sh . ../../../build/intermediate/libc-2-opc/libc.oa \
            -I../../../core/libc/2-opc/include \
            -I../../../core/libc/1-omc/include \
            -I../../../core/libc/0-oo/include && \
    ../run.sh ../1-omc ../../../build/intermediate/libc-2-opc/libc.oa \
            -I../../../core/libc/2-opc/include \
            -I../../../core/libc/1-omc/include \
            -I../../../core/libc/0-oo/include && \
    ../run.sh ../0-oo ../../../build/intermediate/libc-2-opc/libc.oa \
            -I../../../core/libc/2-opc/include \
            -I../../../core/libc/1-omc/include \
            -I../../../core/libc/0-oo/include )

# Build the full C compiler
( core/libo/1-opc/build.sh && true ) #TODO libo tests don't exist yet
( core/ld/2-full/build.sh && cd test/ld/2-full && ../run.sh . onrampvm ../../../build/intermediate/ld-2-full/ld.oe )

                ( core/cci/2-full/build.sh ) #TODO none of the cci/2 tests pass yet but we still want to build it

                #
                #
                # TODO the rest of this is not bootstrappable yet
                exit 0
                #
                #

( core/cci/2-full/build.sh && cd test/cci/2-full && \
    ../run.sh . onrampvm ../../../build/intermediate/cci-2-full/cci.oe && \
    ../run.sh --other-stage ../1-opc onrampvm ../../../build/intermediate/cci-2-full/cci.oe && \
    ../run.sh --other-stage ../0-omc onrampvm ../../../build/intermediate/cci-2-full/cci.oe )
( core/cpp/2-full/build.sh && cd test/cpp/2-full && ../run.sh . onrampvm ../../../build/intermediate/cpp-2-full/cpp.oe )

# Build the rest of the C toolchain
core/libc/3-full/build.sh
( core/as/2-full/build.sh && cd test/as/2-full && \
    ../run.sh . onrampvm ../../../build/intermediate/as-2-full/as.oe && \
    ../run.sh --other-stage ../1-compound onrampvm ../../../build/intermediate/as-2-full/as.oe && \
    ../run.sh --other-stage ../0-basic onrampvm ../../../build/intermediate/as-2-full/as.oe )

# Rebuild our C toolchain with itself
( core/cc/rebuild.sh && cd test/cc && ../run.sh . onrampvm ../../../build/output/bin/cc.oe )
( core/libc/3-full/rebuild.sh && true ) # TODO test libc
( core/ld/2-full/rebuild.sh && cd test/ld/2-full && ../run.sh . onrampvm ../../../build/output/bin/ld.oe )
( core/as/2-full/rebuild.sh && cd test/as/2-full && \
    ../run.sh . onrampvm ../../../build/output/bin/as.oe && \
    ../run.sh --other-stage ../1-compound onrampvm ../../../build/output/bin/as.oe && \
    ../run.sh --other-stage ../0-basic onrampvm ../../../build/output/bin/as.oe )
( core/cci/2-full/rebuild.sh && cd test/cci/2-full && ../run.sh . onrampvm ../../../build/output/bin/cci.oe )
( core/cpp/2-full/rebuild.sh && cd test/cpp/2-full && ../run.sh . onrampvm ../../../build/output/bin/cpp.oe )

# Build the last few tools we need
platform/hex/c89/build.sh
core/ar/1-unix/build.sh

# TODO rebuild, test rebuilt tools as bootstrapped

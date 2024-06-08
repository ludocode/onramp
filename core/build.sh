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



# This is the portable build script for Onramp. It builds all components.
#
# Like all scripts in `core/`, this script is written for the Onramp
# shell, which is a subset of POSIX shell. We have very little actual shell
# functionality here so it's pretty limited in what it can do but it's enough
# to bootstrap Onramp.
#
# This can also be run in an ordinary POSIX shell for ease of debugging. You'll
# need `onrampvm` on your PATH and you'll need to have built `hex/onramp` first.
# Try the setup scripts in `scripts/` to do this automatically, for example
# `scripts/posix/build.sh`.
#
# See `docs/bootstrap-path.md` to follow what this script does.



set -e

# First we get our linker and libc up
sh core/ld/0-global/build.sh
sh core/ar/0-cat/build.sh
sh core/libc/0-oo/build.sh

# Next we build an assembler
sh core/libo/0-oo/build.sh
sh core/as/0-basic/build.sh
sh core/as/1-compound/build.sh

# Next build our omC compiler
sh core/cpp/0-strip/build.sh
sh core/cci/0-omc/build.sh

# Build the rest of the omC toolchain
sh core/cpp/1-omc/build.sh
sh core/ld/1-omc/build.sh
sh core/libc/1-omc/build.sh
sh core/cc/build.sh

# Build the opC toolchain
sh core/cci/1-opc/build.sh
sh core/libc/2-opc/build.sh

# Build the full C compiler
sh core/libo/1-opc/build.sh
sh core/ld/2-full/build.sh
sh core/cci/2-full/build.sh
## sh core/cpp/2-full/build.sh

## # Build the rest of the C toolchain
## sh core/libc/3-full/build.sh
## sh core/as/2-full/build.sh
##
## # Rebuild our C toolchain with itself
## sh core/cc/rebuild.sh
## sh core/libc/3-full/rebuild.sh
## sh core/ld/2-full/rebuild.sh
## sh core/as/2-full/rebuild.sh
## sh core/cci/2-full/rebuild.sh
## sh core/cpp/2-full/rebuild.sh
##
## # Build the last few tools we need
## sh core/hex/1-c89/build.sh
## sh core/ar/1-unix/build.sh

        # TODO the final stages of the above bootstrap process are not
        # implemented yet. For now we package what we have manually to produce
        # a workable compiler.

        # bin/
        mkdir -p build/output
        mkdir -p build/output/bin
        cp build/intermediate/cc/cc.oe build/output/bin/cc.oe
        cp build/intermediate/cpp-1-omc/cpp.oe build/output/bin/cpp.oe
        cp build/intermediate/cci-1-opc/cci.oe build/output/bin/cci.oe
        cp build/intermediate/as-1-compound/as.oe build/output/bin/as.oe
        cp build/intermediate/ld-2-full/ld.oe build/output/bin/ld.oe
        cp build/intermediate/ar-0-cat/ar.oe build/output/bin/ar.oe
        cp build/intermediate/hex-0-onramp/hex.oe build/output/bin/hex.oe
            #cp build/intermediate/cc/cc.oe.od build/output/bin/cc.oe.od
            #cp build/intermediate/cpp-1-omc/cpp.oe.od build/output/bin/cpp.oe.od
            #cp build/intermediate/cci-0-omc/cci.oe.od build/output/bin/cci.oe.od
            #cp build/intermediate/as-1-compound/as.oe.od build/output/bin/as.oe.od
            #cp build/intermediate/ld-2-full/ld.oe.od build/output/bin/ld.oe.od

        # lib/
        mkdir -p build/output/lib
        cp build/intermediate/libc-2-opc/libc.oa build/output/lib/libc.oa

        # include/
        mkdir -p build/output/include
        mkdir -p build/output/include/__onramp
        mkdir -p build/output/include/sys
        cp core/libc/2-opc/include/__onramp/__mode_t.h build/output/include/__onramp/__mode_t.h
        cp core/libc/2-opc/include/__onramp/__predef.h build/output/include/__onramp/__predef.h
        cp core/libc/2-opc/include/__onramp/__size_t.h build/output/include/__onramp/__size_t.h
        cp core/libc/2-opc/include/__onramp/__va_list.h build/output/include/__onramp/__va_list.h
        cp core/libc/2-opc/include/__onramp/__wchar_t.h build/output/include/__onramp/__wchar_t.h
        cp core/libc/0-oo/include/__onramp/__bool.h build/output/include/__onramp/__bool.h
        cp core/libc/0-oo/include/__onramp/__null.h build/output/include/__onramp/__null.h
        cp core/libc/2-opc/include/sys/stat.h build/output/include/sys/stat.h
        cp core/libc/2-opc/include/sys/time.h build/output/include/sys/time.h
        cp core/libc/2-opc/include/sys/types.h build/output/include/sys/types.h

        cp core/libc/2-opc/include/assert.h build/output/include/assert.h
        cp core/libc/2-opc/include/ctype.h build/output/include/ctype.h
        cp core/libc/0-oo/include/errno.h build/output/include/errno.h
        cp core/libc/2-opc/include/fcntl.h build/output/include/fcntl.h
        cp core/libc/2-opc/include/features.h build/output/include/features.h
        cp core/libc/2-opc/include/inttypes.h build/output/include/inttypes.h
        cp core/libc/2-opc/include/iso646.h build/output/include/iso646.h
        cp core/libc/2-opc/include/limits.h build/output/include/limits.h
        cp core/libc/2-opc/include/malloc.h build/output/include/malloc.h
        cp core/libc/2-opc/include/setjmp.h build/output/include/setjmp.h
        cp core/libc/0-oo/include/spawn.h build/output/include/spawn.h
        cp core/libc/2-opc/include/stdalign.h build/output/include/stdalign.h
        cp core/libc/2-opc/include/stdarg.h build/output/include/stdarg.h
        cp core/libc/0-oo/include/stdbool.h build/output/include/stdbool.h
        cp core/libc/2-opc/include/stddef.h build/output/include/stddef.h
        cp core/libc/2-opc/include/stdint.h build/output/include/stdint.h
        cp core/libc/2-opc/include/stdio.h build/output/include/stdio.h
        cp core/libc/2-opc/include/stdlib.h build/output/include/stdlib.h
        cp core/libc/2-opc/include/stdnoreturn.h build/output/include/stdnoreturn.h
        cp core/libc/2-opc/include/string.h build/output/include/string.h
        cp core/libc/2-opc/include/strings.h build/output/include/strings.h
        cp core/libc/2-opc/include/time.h build/output/include/time.h
        cp core/libc/2-opc/include/unistd.h build/output/include/unistd.h

echo Done.

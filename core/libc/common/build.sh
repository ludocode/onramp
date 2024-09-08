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

# This script copies the libc headers into the build output.
#
# We don't have recursive copy in Onramp shell so we have to copy each file
# individually.

set -e
mkdir -p build/output/include/__onramp
mkdir -p build/output/include/sys

cp core/libc/common/include/__onramp/__arithmetic.h build/output/include/__onramp/__arithmetic.h
cp core/libc/common/include/__onramp/__bool.h build/output/include/__onramp/__bool.h
cp core/libc/common/include/__onramp/__mode_t.h build/output/include/__onramp/__mode_t.h
cp core/libc/common/include/__onramp/__null.h build/output/include/__onramp/__null.h
cp core/libc/common/include/__onramp/__predef.h build/output/include/__onramp/__predef.h
cp core/libc/common/include/__onramp/__size_t.h build/output/include/__onramp/__size_t.h
cp core/libc/common/include/__onramp/__va_list.h build/output/include/__onramp/__va_list.h
cp core/libc/common/include/__onramp/__wchar_t.h build/output/include/__onramp/__wchar_t.h
cp core/libc/common/include/assert.h build/output/include/assert.h
cp core/libc/common/include/ctype.h build/output/include/ctype.h
cp core/libc/common/include/errno.h build/output/include/errno.h
cp core/libc/common/include/fcntl.h build/output/include/fcntl.h
cp core/libc/common/include/features.h build/output/include/features.h
cp core/libc/common/include/inttypes.h build/output/include/inttypes.h
cp core/libc/common/include/iso646.h build/output/include/iso646.h
cp core/libc/common/include/limits.h build/output/include/limits.h
cp core/libc/common/include/malloc.h build/output/include/malloc.h
cp core/libc/common/include/setjmp.h build/output/include/setjmp.h
cp core/libc/common/include/spawn.h build/output/include/spawn.h
cp core/libc/common/include/stdalign.h build/output/include/stdalign.h
cp core/libc/common/include/stdarg.h build/output/include/stdarg.h
cp core/libc/common/include/stdbool.h build/output/include/stdbool.h
cp core/libc/common/include/stddef.h build/output/include/stddef.h
cp core/libc/common/include/stdint.h build/output/include/stdint.h
cp core/libc/common/include/stdio.h build/output/include/stdio.h
cp core/libc/common/include/stdlib.h build/output/include/stdlib.h
cp core/libc/common/include/stdnoreturn.h build/output/include/stdnoreturn.h
cp core/libc/common/include/string.h build/output/include/string.h
cp core/libc/common/include/strings.h build/output/include/strings.h
cp core/libc/common/include/sys/stat.h build/output/include/sys/stat.h
cp core/libc/common/include/sys/time.h build/output/include/sys/time.h
cp core/libc/common/include/sys/types.h build/output/include/sys/types.h
cp core/libc/common/include/time.h build/output/include/time.h
cp core/libc/common/include/unistd.h build/output/include/unistd.h

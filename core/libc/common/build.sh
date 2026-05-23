#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2024-2025 Fraser Heavy Software
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

echo
echo === Copying libc headers

mkdir -p output/final/include/__onramp
mkdir -p output/final/include/sys

echo Copying internal headers
cp core/libc/common/include/__onramp/__arithmetic.h output/final/include/__onramp/__arithmetic.h
cp core/libc/common/include/__onramp/__bool.h output/final/include/__onramp/__bool.h
cp core/libc/common/include/__onramp/__fatal.h output/final/include/__onramp/__fatal.h
cp core/libc/common/include/__onramp/__mode_t.h output/final/include/__onramp/__mode_t.h
cp core/libc/common/include/__onramp/__name_max.h output/final/include/__onramp/__name_max.h
cp core/libc/common/include/__onramp/__null.h output/final/include/__onramp/__null.h
cp core/libc/common/include/__onramp/__pit.h output/final/include/__onramp/__pit.h
cp core/libc/common/include/__onramp/__predef.h output/final/include/__onramp/__predef.h
cp core/libc/common/include/__onramp/__seek.h output/final/include/__onramp/__seek.h
cp core/libc/common/include/__onramp/__size_t.h output/final/include/__onramp/__size_t.h
cp core/libc/common/include/__onramp/__syscalls.h output/final/include/__onramp/__syscalls.h
cp core/libc/common/include/__onramp/__time_t.h output/final/include/__onramp/__time_t.h
cp core/libc/common/include/__onramp/__timespec.h output/final/include/__onramp/__timespec.h
cp core/libc/common/include/__onramp/__useconds_t.h output/final/include/__onramp/__useconds_t.h
cp core/libc/common/include/__onramp/__va_list.h output/final/include/__onramp/__va_list.h
cp core/libc/common/include/__onramp/__wchar_limits.h output/final/include/__onramp/__wchar_limits.h
cp core/libc/common/include/__onramp/__wchar_t.h output/final/include/__onramp/__wchar_t.h
cp core/libc/common/include/__onramp/__wint_t.h output/final/include/__onramp/__wint_t.h

echo Copying standard headers
cp core/libc/common/include/assert.h output/final/include/assert.h
cp core/libc/common/include/ctype.h output/final/include/ctype.h
cp core/libc/common/include/dirent.h output/final/include/dirent.h
cp core/libc/common/include/errno.h output/final/include/errno.h
cp core/libc/common/include/fcntl.h output/final/include/fcntl.h
cp core/libc/common/include/features.h output/final/include/features.h
cp core/libc/common/include/inttypes.h output/final/include/inttypes.h
cp core/libc/common/include/iso646.h output/final/include/iso646.h
cp core/libc/common/include/limits.h output/final/include/limits.h
cp core/libc/common/include/malloc.h output/final/include/malloc.h
cp core/libc/common/include/math.h output/final/include/math.h
cp core/libc/common/include/semaphore.h output/final/include/semaphore.h
cp core/libc/common/include/setjmp.h output/final/include/setjmp.h
cp core/libc/common/include/signal.h output/final/include/signal.h
cp core/libc/common/include/spawn.h output/final/include/spawn.h
cp core/libc/common/include/stdalign.h output/final/include/stdalign.h
cp core/libc/common/include/stdarg.h output/final/include/stdarg.h
cp core/libc/common/include/stdbit.h output/final/include/stdbit.h
cp core/libc/common/include/stdbool.h output/final/include/stdbool.h
cp core/libc/common/include/stddef.h output/final/include/stddef.h
cp core/libc/common/include/stdint.h output/final/include/stdint.h
cp core/libc/common/include/stdio.h output/final/include/stdio.h
cp core/libc/common/include/stdlib.h output/final/include/stdlib.h
cp core/libc/common/include/stdnoreturn.h output/final/include/stdnoreturn.h
cp core/libc/common/include/string.h output/final/include/string.h
cp core/libc/common/include/strings.h output/final/include/strings.h
cp core/libc/common/include/sys/stat.h output/final/include/sys/stat.h
cp core/libc/common/include/sys/syscall.h output/final/include/sys/syscall.h
cp core/libc/common/include/sys/time.h output/final/include/sys/time.h
cp core/libc/common/include/sys/types.h output/final/include/sys/types.h
cp core/libc/common/include/termios.h output/final/include/termios.h
cp core/libc/common/include/time.h output/final/include/time.h
cp core/libc/common/include/uchar.h output/final/include/uchar.h
cp core/libc/common/include/unistd.h output/final/include/unistd.h
cp core/libc/common/include/wchar.h output/final/include/wchar.h
cp core/libc/common/include/wctype.h output/final/include/wctype.h

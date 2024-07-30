/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ONRAMP_LIBC_ONRAMP_BOOL_H_INCLUDED
#define __ONRAMP_LIBC_ONRAMP_BOOL_H_INCLUDED

// omC and opC don't support _Bool so when compiling under these compilers,
// _Bool is int (with a size of 4 bytes.) It's not the same semantics as _Bool
// but it's good enough.
//
// (We use int instead of char to prevent truncating conversions from losing
// upper bits. If we store a high bit in a bool and then test that bool in an
// if statement, it should evaluate as true.)
//
// In the final stage, _Bool is a fundamental type with a size of 1 byte. It's
// effectively a 1-bit integer, except that assigning any non-zero value to it
// makes it 1.
//
// TODO make sure that these definitions of _Bool are ABI-compatible, otherwise
// we wouldn't be able to link cci/1 objects with cci/2 objects. This *should*
// be safe already; we just need to make sure we never have high bits set when
// we pass or return a _Bool.

#ifdef __onramp_cci_omc__
    typedef int _Bool;
#endif
#ifdef __onramp_cci_opc__
    typedef int _Bool;
#endif

#endif

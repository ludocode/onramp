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

#ifndef __ONRAMP_LIBC_CTYPE_H_INCLUDED
#define __ONRAMP_LIBC_CTYPE_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif



// functions in libc/0
int isalnum(int);
int isalpha(int);
int isblank(int);
int isdigit(int);
int isgraph(int);
int islower(int);
int isprint(int);
int isspace(int);
int isupper(int);
int isxdigit(int);



// functions in libc/2

#ifndef __onramp_libc_oo__
#ifndef __onramp_libc_omc__

int isascii(int);
int iscntrl(int);
int ispunct(int);
int tolower(int);
int toupper(int);

#endif
#endif



#endif

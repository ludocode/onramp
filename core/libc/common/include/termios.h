/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Fraser Heavy Software
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

#ifndef __ONRAMP_LIBC_TERMIOS_H_INCLUDED
#define __ONRAMP_LIBC_TERMIOS_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

typedef int tcflag_t;

// TODO this structure is incomplete. We currently only support c_lflag.
struct termios {
    tcflag_t c_lflag;
};

// TODO these are the only flags supported so far. The bits match those of the
// VM pit capabilities field, not that there's any point in doing that.
#define ECHO     (1 << 0)
#define ICANON   (1 << 2)

// TODO we only support TCSANOW.
// - we should support TCSADRAIN but I haven't decided yet whether the VM
//   should allow output buffering, and if it does, it will need a flush syscall
// - we should support TCSAFLUSH which would do the same as TCSADRAIN but, if
//   input is non-blocking, it should also read and discard all pending input
// TODO it's not clear whether these are supposed to be bit flags. We allow it
// just in case.
#define TCSANOW (1 << 0)
//#define TCSADRAIN (1 << 1)
//#define TCSAFLUSH (1 << 2)

int tcgetattr(int __fd, struct termios* __termios);
int tcsetattr(int __fd, int __actions, const struct termios* __termios);

#endif

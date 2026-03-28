/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2025 Fraser Heavy Software
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

#ifndef __ONRAMP_LIBC_FEATURES_H_INCLUDED
#define __ONRAMP_LIBC_FEATURES_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

/*
 * This file contains the Onramp libc implementation of feature test macros.
 *
 * The contents are based on the feature_test_macros(7) Linux man page, as well
 * as black-box testing of what GCC and glibc define under various feature test
 * macros and -std=* options.
 *
 * This means we also try to implicitly define feature test macros wherever the
 * documentation suggests we should. For example if a user defines only
 * _XOPEN_SOURCE to 700, we implicitly define _POSIX_C_SOURCE to 200809L, and
 * therefore implicitly define _ATFILE_SOURCE. We do this in case other user
 * code is testing for these macros.
 *
 * We try to do the same as other Linux compilers and libcs with a couple
 * exceptions:
 *
 * The first is that we compile in a `-std=c*` mode by default, not in a
 * `-std=gnu*` mode. This means we define __STRICT_ANSI__, so none of the other
 * feature test macros (e.g. _DEFAULT_SOURCE) get enabled by default.
 * __STRICT_ANSI__ can be disabled by a `-std=gnu*` mode, by
 * `-fgnu-extensions`, or by calling `onrampgcc` instead of `onrampcc`.
 *
 * The second is that we don't currently #undef any macros that are already
 * defined, even if they are incorrect or incompatible with other defined
 * flags. We just define all symbols in the union of all defined feature flags
 * as best we can.
 */

/*
 * Implementation details:
 *
 * Many of our libc headers need to support compiling with cpp/1 where #if is
 * not available. We therefore parse out the feature test macros and convert
 * them to a set of internal defines that can be tested with #ifdef. These all
 * have prefix `__onramp_ftm_`.
 *
 * For example, if the user defines _POSIX_C_SOURCE to 199506L or greater, we
 * define all of the following:
 *
 *     __onramp_ftm_posix_1990
 *     __onramp_ftm_posix_1992
 *     __onramp_ftm_posix_1993
 *     __onramp_ftm_posix_1995
 *
 * A libc function added in POSIX 1993 would then be wrapped in
 * `#ifdef __onramp_ftm_posix_1993`. This is probably similar to what other
 * libcs do with their __USE macros.
 *
 * When these internal macros are defined, they are always defined to empty
 * (never 0 or 1 or anything else.) They are not intended to be used with #if.
 * They are also meant to be used only in the libc; user code should not use
 * them. Note that this is not the case for the feature test macros: if nothing
 * is defined, _DEFAULT_SOURCE will be defined to 1 and not empty, even though
 * it is never used in a comparison and the man pages suggest it should be
 * defined to empty.
 *
 * We don't bother to check if the compiler is cpp/1 unless we need to wrap a
 * top-level #if statement. Feature test macros are not intended to be used
 * in the early stages of bootstrapping.
 */



/*
 * _GNU_SOURCE
 *
 * This implicitly defines tons of stuff. The man page says it defines
 * _ISOC99_SOURCE and doesn't mention _ISOC11_SOURCE, but apparently it defines
 * all of them, even _ISOC2Y_SOURCE, even when set to -std=c11. It also defines
 * _XOPEN_SOURCE to 700.
 *
 * Even if some of the feature test macros are already incorrectly defined to
 * lower numbers, we always include everything in _GNU_SOURCE.
 */
#ifdef _GNU_SOURCE
    #define __onramp_ftm_gnu

    // _ATFILE_SOURCE
    #define __onramp_ftm_at
    #ifndef _ATFILE_SOURCE
        #define _ATFILE_SOURCE 1
    #endif

    // _LARGEFILE64_SOURCE
    //TODO
    #ifndef _LARGEFILE64_SOURCE
        #define _LARGEFILE64_SOURCE 1
    #endif

    // _ISOC99_SOURCE
    #define __onramp_ftm_c95
    #define __onramp_ftm_c99
    #ifndef _ISOC99_SOURCE
        #define _ISOC99_SOURCE 1
    #endif

    // _ISOC11_SOURCE
    #define __onramp_ftm_c11
    #ifndef _ISOC11_SOURCE
        #define _ISOC11_SOURCE 1
    #endif

    // _ISOC23_SOURCE
    #define __onramp_ftm_c23
    #ifndef _ISOC23_SOURCE
        #define _ISOC23_SOURCE 1
    #endif

    // _ISOC2Y_SOURCE
    #define __onramp_ftm_c2y
    #ifndef _ISOC2Y_SOURCE
        #define _ISOC2Y_SOURCE 1
    #endif

    // _XOPEN_SOURCE_EXTENDED
    #ifndef _XOPEN_SOURCE_EXTENDED
        #define _XOPEN_SOURCE_EXTENDED 1
    #endif

    // _XOPEN_SOURCE
    #define __onramp_ftm_susv2
    #define __onramp_ftm_susv3
    #define __onramp_ftm_susv4
    #define __onramp_ftm_xpg4v2
    #define __onramp_ftm_xsi
    #ifndef _XOPEN_SOURCE
        #define _XOPEN_SOURCE 700
    #endif

    // _POSIX_C_SOURCE
    #define __onramp_ftm_posix_1990
    #define __onramp_ftm_posix_1992
    #define __onramp_ftm_posix_1993
    #define __onramp_ftm_posix_1995
    #define __onramp_ftm_posix_2001
    #define __onramp_ftm_posix_2008
    #ifndef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 200809L
    #endif

    // _POSIX_SOURCE
    #ifndef _POSIX_SOURCE
        #define _POSIX_SOURCE 1
    #endif

    // _DEFAULT_SOURCE
    #define __onramp_ftm_default
    #ifndef _DEFAULT_SOURCE
        #define _DEFAULT_SOURCE 1
    #endif
#endif



/*
 * _XOPEN_SOURCE
 *
 * We check for this before _POSIX_C_SOURCE because this defines _POSIX_SOURCE
 * and _POSIX_C_SOURCE if neither are defined.
 *
 * If _POSIX_C_SOURCE is defined to a lower version of POSIX than is required
 * by the value of _XOPEN_SOURCE, the additional POSIX symbols will be defined.
 * The value of _POSIX_C_SOURCE will not be redefined and will effectively be
 * ignored in this case.
 */

#ifdef _XOPEN_SOURCE

    // 700
    #if _XOPEN_SOURCE >= 700
        #define __onramp_ftm_susv4
        #define __onramp_ftm_xsi

        // 700 includes POSIX 2008.
        #define __onramp_ftm_posix_2008
        #if !defined(_POSIX_SOURCE) && !defined(_POSIX_C_SOURCE)
            #define _POSIX_SOURCE 1
            #define _POSIX_C_SOURCE 200809L
        #endif
    #endif

    // 600
    #if _XOPEN_SOURCE >= 600
        #define __onramp_ftm_susv3

        // 600 includes POSIX 2001, and defines _POSIX_C_SOURCE if not already
        // defined.
        #define __onramp_ftm_posix_2001
        #if (!defined(_POSIX_SOURCE) && !defined(_POSIX_C_SOURCE))
            #define _POSIX_SOURCE 1
            #define _POSIX_C_SOURCE 200112L
        #endif
    #endif

    // 500
    #if _XOPEN_SOURCE >= 500 || defined(_XOPEN_SOURCE_EXTENDED)
        #define __onramp_ftm_susv2
        #define __onramp_ftm_xpg4v2

        // 500 includes POSIX 1995.
        #define __onramp_ftm_posix_1993
        #define __onramp_ftm_posix_1995
        #if (!defined(_POSIX_SOURCE) && !defined(_POSIX_C_SOURCE))
            #define _POSIX_SOURCE 1
            #define _POSIX_C_SOURCE 199506L
        #endif
    #endif

    // Any value of _XOPEN_SOURCE activates POSIX 1992 but, for _XOPEN_SOURCE
    // less than 500, we only define _POSIX_SOURCE and _POSIX_C_SOURCE if
    // __STRICT_ANSI__ is not defined.
    #define __onramp_ftm_posix_1990
    #define __onramp_ftm_posix_1992
    #if !defined(__STRICT_ANSI__) && \
            !defined(_POSIX_SOURCE) && !defined(_POSIX_C_SOURCE)
        #define _POSIX_SOURCE 1
        #define _POSIX_C_SOURCE 2
    #endif

    // Any value also includes XPG4.
    #define __onramp_ftm_xpg4

#endif



/*
 * _POSIX_C_SOURCE
 *
 * We also handle _POSIX_SOURCE here. It is implicitly defined if
 * _POSIX_C_SOURCE is.
 */

#ifdef _POSIX_C_SOURCE
    #if _POSIX_C_SOURCE >= 1
        #define __onramp_ftm_posix_1990
    #endif
    #if _POSIX_C_SOURCE >= 2
        #define __onramp_ftm_posix_1992
    #endif
    #if _POSIX_C_SOURCE >= 199309L
        #define __onramp_ftm_posix_1993
    #endif
    #if _POSIX_C_SOURCE >= 199506L
        #define __onramp_ftm_posix_1995
    #endif
    #if _POSIX_C_SOURCE >= 200112L
        #define __onramp_ftm_posix_2001
    #endif
    #if _POSIX_C_SOURCE >= 200809L
        #define __onramp_ftm_posix_2008
        #ifndef _ATFILE_SOURCE
            #define _ATFILE_SOURCE 1
        #endif
    #endif
#endif

#ifndef _POSIX_SOURCE
    #ifdef _POSIX_C_SOURCE
        #define _POSIX_SOURCE 1
    #endif
#endif



/*
 * __STDC_VERSION__
 *
 * This isn't defined by the user, but by the driver based on the -std=
 * command-line option.
 */

#ifdef __STDC_VERSION__
    #if __STDC_VERSION__ >= 199409L
        #define __onramp_ftm_c95
    #endif
    #if __STDC_VERSION__ >= 199901L
        #define __onramp_ftm_c99
    #endif
    #if __STDC_VERSION__ >= 201112L
        #define __onramp_ftm_c11
    #endif
    #if __STDC_VERSION__ >= 201710L
        #define __onramp_ftm_c17 // TODO probably useless
    #endif
    #if __STDC_VERSION__ >= 202311L
        #define __onramp_ftm_c23
    #endif
    #if __STDC_VERSION__ > 202311L
        #define __onramp_ftm_c2y
    #endif
#endif



/*
 * _ISOC99_SOURCE
 *
 * Also _ISOC9X_SOURCE for backwards compatibility. Neither implicitly defines
 * the other, and neither is implicitly defined by -std=c99.
 */

#ifndef __onramp_cpp_omc__
    #if defined(_ISOC99_SOURCE) || defined(_ISOC9X_SOURCE)
        #define __onramp_ftm_c95
        #define __onramp_ftm_c99
    #endif
#endif



/*
 * _ISOC11_SOURCE
 *
 * Note that this does not implicitly define _ISOC99_SOURCE, and this is not
 * implicitly defined by -std=c11.
 *
 * There apparently was never an _ISOC1X_SOURCE.
 */

#ifdef _ISOC11_SOURCE
    #define __onramp_ftm_c95
    #define __onramp_ftm_c99
    #define __onramp_ftm_c11
#endif



/*
 * _ISOC23_SOURCE
 *
 * Also _ISOC2X_SOURCE.
 */

#ifndef __onramp_cpp_omc__
    #if defined(_ISOC23_SOURCE) || defined(_ISOC2X_SOURCE)
        #define __onramp_ftm_c95
        #define __onramp_ftm_c99
        #define __onramp_ftm_c11
        #define __onramp_ftm_c23
    #endif
#endif



/*
 * _ISOC2Y_SOURCE
 */

#ifdef _ISOC2Y_SOURCE
    #define __onramp_ftm_c95
    #define __onramp_ftm_c99
    #define __onramp_ftm_c11
    #define __onramp_ftm_c23
    #define __onramp_ftm_c2y
#endif



/* TODO
_LARGEFILE64_SOURCE
_LARGEFILE_SOURCE
_FILE_OFFSET_BITS
_TIME_BITS
_THREAD_SAFE
_REENTRANT //deprecated
*/



/*
 * _BSD_SOURCE
 *
 * In glibc this is equivalent to _DEFAULT_SOURCE except for a deprecation
 * warning. We don't emit the warning.
 */
#ifdef _BSD_SOURCE
    #define __onramp_ftm_bsd
    #define __onramp_ftm_default
    #ifndef _DEFAULT_SOURCE
        #define _DEFAULT_SOURCE 1
    #endif
#endif



/*
 * _SVID_SOURCE
 *
 * This is the same as _BSD_SOURCE in glibc. We also skip the warning.
 */
#ifdef _SVID_SOURCE
    #define __onramp_ftm_svid
    #define __onramp_ftm_default
    #ifndef _DEFAULT_SOURCE
        #define _DEFAULT_SOURCE 1
    #endif
#endif



/*
 * _AT_SOURCE
 *
 * Enables functions with "at" prefix such as openat(). (These are just
 * emulated on Onramp.)
 */
#ifdef _ATFILE_SOURCE
    #define __onramp_ftm_at
#endif



/*
 * _DEFAULT_SOURCE
 *
 * Apparently BSD and SVID are deprecated in favour of this so we define them
 * implicitly if they aren't already.
 */
#ifdef _DEFAULT_SOURCE
    #define __onramp_ftm_default
    #ifndef _BSD_SOURCE
        #define _BSD_SOURCE 1
    #endif
    #ifndef _SVID_SOURCE
        #define _SVID_SOURCE 1
    #endif
#endif



/*
 * _FORTIFY_SOURCE
 */

#ifdef _FORTIFY_SOURCE
    #ifndef __onramp_cpp_omc__
        #if _FORTIFY_SOURCE >= 1
            #define __onramp_ftm_fortify_1
        #endif
        #if _FORTIFY_SOURCE >= 2
            #define __onramp_ftm_fortify_2
        #endif
        #if _FORTIFY_SOURCE >= 3
            #define __onramp_ftm_fortify_3
        #endif
    #endif
#endif



/*
 * This turns on most libc features without expressing a preference
 * for BSD vs. System V.
 *
 * A few macros define this including:
 * - _DEFAULT_SOURCE
 * - _BSD_SOURCE
 * - _GNU_SOURCE
 */
#ifdef __onramp_ftm_default
    #define __onramp_ftm_at

    // We enable C99 features but not later ones to match glibc. Almost all C11
    // and later symbols are either in POSIX 2008 or they are in new header
    // files so they can be used anyway (as long as they don't depend on new
    // compiler features.)
    #define __onramp_ftm_c95
    #define __onramp_ftm_c99

    #define __onramp_ftm_posix_1990
    #define __onramp_ftm_posix_1992
    #define __onramp_ftm_posix_1993
    #define __onramp_ftm_posix_1995
    #define __onramp_ftm_posix_2001
    #define __onramp_ftm_posix_2008
    #define __onramp_ftm_susv2
    #define __onramp_ftm_susv3
    #define __onramp_ftm_susv4
    #define __onramp_ftm_xpg4
    #define __onramp_ftm_xpg4v2
    #ifndef __onramp_cpp_omc__
        #undef __onramp_ftm_default
    #endif
#endif



#endif // __ONRAMP_LIBC_FEATURES_H_INCLUDED

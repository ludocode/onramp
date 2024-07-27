#ifndef PORTABILITY_H_INCLUDED
#define PORTABILITY_H_INCLUDED

/*
 * This header helps us test our C code in native C compilers. It is
 * force-included by the test Makefiles.
 */

/*
 * Clang warns about the use of bitwise `|` on boolean value because it wants
 * us to use `||`, but `||` does not exist in omC.
 *
 * We need to silence this warning in omC source files. We do want the warning
 * on in other C source files though. The test Makefiles of programs written in
 * omC define __onramp_omc__.
 */
#ifdef __onramp_omc__
    #ifdef __has_warning
        #if __has_warning("-Wbitwise-instead-of-logical")
            #pragma GCC diagnostic ignored "-Wbitwise-instead-of-logical"
        #endif
    #endif
#endif

#endif

/*
 * MIT No Attribution
 *
 * Copyright (c) 2022 Fraser Heavy Software
 * Copyright (c) 2022-2023 Fraser Heavy Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * This is an amalgamated Ghost header.
 *
 * See the official Ghost source:
 *     https://github.com/ludocode/ghost
 */

/*
 * The following leaf headers were amalgamated into this file:
 *
 *     ghost/debug/ghost_debugbreak.h
 *     ghost/debug/ghost_static_assert.h
 *     ghost/header/c/ghost_stdint_h.h
 *     ghost/header/c/ghost_stdio_h.h
 *     ghost/header/c/ghost_stdlib_h.h
 *     ghost/header/c/ghost_string_h.h
 *     ghost/language/ghost_always_inline.h
 *     ghost/language/ghost_array_count.h
 *     ghost/language/ghost_const_cast.h
 *     ghost/language/ghost_noinline.h
 *     ghost/language/ghost_noreturn.h
 *     ghost/language/ghost_null.h
 *     ghost/language/ghost_unreachable.h
 *     ghost/malloc/ghost_alloc_array.h
 *     ghost/malloc/ghost_alloc_zero.h
 *     ghost/malloc/ghost_strdup.h
 *     ghost/math/min/ghost_min_u32.h
 *     ghost/math/rotr/ghost_rotr_u32.h
 *     ghost/serialization/load/ghost_load_le_u32.h
 *     ghost/serialization/store/ghost_store_le_u32.h
 *     ghost/string/ghost_strlcat.h
 *     ghost/string/ghost_strlcpy.h
 */

#ifndef VM_GHOST_AMALGAMATED_H_INCLUDED
#define VM_GHOST_AMALGAMATED_H_INCLUDED

#define VM_GHOST_IMPL_AMALGAMATED 1


/******************************
 * ghost/core/ghost_version.h
 ******************************/

#if defined(VM_GHOST_VERSION) || defined(vm_ghost_has_VM_GHOST_VERSION)
    #error "VM_GHOST_VERSION cannot be overridden."
#endif

#define VM_GHOST_VERSION 1
#define vm_ghost_has_VM_GHOST_VERSION 1

/*****************************
 * ghost/core/ghost_config.h
 *****************************/

#ifdef VM_GHOST_CONFIG_H
    #include VM_GHOST_CONFIG_H
#endif

#ifndef VM_GHOST_EXPERIMENTAL
    #define VM_GHOST_EXPERIMENTAL 0
#endif

#ifndef VM_GHOST_DOCUMENTATION
    #define VM_GHOST_DOCUMENTATION 0
#endif

#ifndef VM_GHOST_IMPL_AMALGAMATED
    #define VM_GHOST_IMPL_AMALGAMATED 0
#endif

/* TODO maybe get rid of this entirely, just use VM_GHOST_EXPERIMENTAL */
#ifndef VM_GHOST_IMPL_ENABLE_C23
    #define VM_GHOST_IMPL_ENABLE_C23 VM_GHOST_EXPERIMENTAL
#endif

#ifndef VM_GHOST_ENABLE_CXX23
    #define VM_GHOST_ENABLE_CXX23 VM_GHOST_EXPERIMENTAL
#endif

/**************************
 * ghost/core/ghost_has.h
 **************************/

#ifndef vm_ghost_has_vm_ghost_has
    #ifdef vm_ghost_has
        #define vm_ghost_has_vm_ghost_has 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_has
    #define vm_ghost_has(identifier) vm_ghost_has_##identifier
    #define vm_ghost_has_vm_ghost_has 1
#endif

/*************************************
 * ghost/preprocessor/ghost_concat.h
 *************************************/

#ifndef vm_ghost_has_VM_GHOST_CONCAT
    #ifndef VM_GHOST_CONCAT
        #define VM_GHOST_CONCAT(x, y) VM_GHOST_CONCAT_IMPL(x, y)
        #define VM_GHOST_CONCAT_IMPL(x, y) x##y
    #endif
    #define vm_ghost_has_VM_GHOST_CONCAT 1
#endif

/**************************************
 * ghost/preprocessor/ghost_counter.h
 **************************************/

#ifndef vm_ghost_has_VM_GHOST_COUNTER
    #ifdef VM_GHOST_COUNTER
        #define vm_ghost_has_VM_GHOST_COUNTER 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_COUNTER
    #if defined(__GNUC__) && !defined(__PCC__)
        #if defined(__clang__) || defined(__TINYC__)
            #define VM_GHOST_COUNTER __COUNTER__
            #define vm_ghost_has_VM_GHOST_COUNTER 1
        #elif __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
            #define VM_GHOST_COUNTER __COUNTER__
            #define vm_ghost_has_VM_GHOST_COUNTER 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_COUNTER
    #ifdef _MSC_VER
        #if _MSC_VER >= 1900
            #define VM_GHOST_COUNTER __COUNTER__
            #define vm_ghost_has_VM_GHOST_COUNTER 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_COUNTER
    #define VM_GHOST_COUNTER __LINE__
    #define vm_ghost_has_VM_GHOST_COUNTER 1
#endif

/******************************
 * ghost/detect/ghost_cproc.h
 ******************************/

#ifndef vm_ghost_has_VM_GHOST_CPROC
    #ifdef VM_GHOST_CPROC
        #define vm_ghost_has_VM_GHOST_CPROC 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_CPROC
    #if defined(__cproc__) || defined(__CPROC__)
        #define VM_GHOST_CPROC 1
        #define vm_ghost_has_VM_GHOST_CPROC 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_CPROC
    #if !defined(__GNUC__) && !defined(__GNUC_MINOR__) && defined(__extension__) && \
            (defined(__GNUC_PATCHLEVEL__) || defined(__GNUC_STDC_INLINE__) || defined(__GNUC_GNU_INLINE__))
        #define VM_GHOST_CPROC 1
        #define vm_ghost_has_VM_GHOST_CPROC 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_CPROC
    #define VM_GHOST_CPROC 0
    #define vm_ghost_has_VM_GHOST_CPROC 1
#endif

/******************************************
 * ghost/preprocessor/ghost_has_builtin.h
 ******************************************/

#ifndef vm_ghost_has_vm_ghost_has_builtin
    #ifdef vm_ghost_has_builtin
        #error "vm_ghost_has_builtin cannot be defined. Define vm_ghost_has_vm_ghost_has_builtin to 1 or 0 to override Ghost's __has_builtin detection."
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_has_builtin
    #ifndef __has_builtin
        #define vm_ghost_has_vm_ghost_has_builtin 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_has_builtin
    #ifdef __LCC__
        #define vm_ghost_has_vm_ghost_has_builtin 0
    #else
        #if VM_GHOST_CPROC
            #define vm_ghost_has_vm_ghost_has_builtin 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_has_builtin
    #if __has_builtin(__builtin_vm_ghost_nonexistent_builtin)
        #define vm_ghost_has_vm_ghost_has_builtin 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_has_builtin
    #if \
            !__has_builtin(__builtin__Exit) && \
            !__has_builtin(__builtin_add_overflow) && \
            !__has_builtin(__builtin_alloca) && \
            !__has_builtin(__builtin_bcmp) && \
            !__has_builtin(__builtin_bit_cast) && \
            !__has_builtin(__builtin_bswap16) && \
            !__has_builtin(__builtin_bswap32) && \
            !__has_builtin(__builtin_bzero) && \
            !__has_builtin(__builtin_ceil) && \
            !__has_builtin(__builtin_choose_expr) && \
            !__has_builtin(__builtin_clear_padding) && \
            !__has_builtin(__builtin_constant_p) && \
            !__has_builtin(__builtin_ctz) && \
            !__has_builtin(__builtin_debugtrap) && \
            !__has_builtin(__builtin_expect) && \
            !__has_builtin(__builtin_fabs) && \
            !__has_builtin(__builtin_ffs) && \
            !__has_builtin(__builtin_floor) && \
            !__has_builtin(__builtin_free) && \
            !__has_builtin(__builtin_has_attribute) && \
            !__has_builtin(__builtin_huge_val) && \
            !__has_builtin(__builtin_inf) && \
            !__has_builtin(__builtin_isalnum) && \
            !__has_builtin(__builtin_isfinite) && \
            !__has_builtin(__builtin_isgreater) && \
            !__has_builtin(__builtin_isgreaterequal) && \
            !__has_builtin(__builtin_isinf) && \
            !__has_builtin(__builtin_isless) && \
            !__has_builtin(__builtin_islessequal) && \
            !__has_builtin(__builtin_islessgreater) && \
            !__has_builtin(__builtin_islower) && \
            !__has_builtin(__builtin_isnan) && \
            !__has_builtin(__builtin_isprint) && \
            !__has_builtin(__builtin_ispunct) && \
            !__has_builtin(__builtin_isspace) && \
            !__has_builtin(__builtin_isunordered) && \
            !__has_builtin(__builtin_isupper) && \
            !__has_builtin(__builtin_launder) && \
            !__has_builtin(__builtin_lround) && \
            !__has_builtin(__builtin_malloc) && \
            !__has_builtin(__builtin_memcmp) && \
            !__has_builtin(__builtin_memcpy) && \
            !__has_builtin(__builtin_memmove) && \
            !__has_builtin(__builtin_memset) && \
            !__has_builtin(__builtin_mul_overflow) && \
            !__has_builtin(__builtin_nan) && \
            !__has_builtin(__builtin_object_size) && \
            !__has_builtin(__builtin_offsetof) && \
            !__has_builtin(__builtin_popcount) && \
            !__has_builtin(__builtin_realloc) && \
            !__has_builtin(__builtin_rotateleft16) && \
            !__has_builtin(__builtin_rotateleft32) && \
            !__has_builtin(__builtin_round) && \
            !__has_builtin(__builtin_snprintf) && \
            !__has_builtin(__builtin_sprintf) && \
            !__has_builtin(__builtin_sqrt) && \
            !__has_builtin(__builtin_strcat) && \
            !__has_builtin(__builtin_strcmp) && \
            !__has_builtin(__builtin_strcpy) && \
            !__has_builtin(__builtin_strlen) && \
            !__has_builtin(__builtin_strncpy) && \
            !__has_builtin(__builtin_strnlen) && \
            !__has_builtin(__builtin_sub_overflow) && \
            !__has_builtin(__builtin_trap) && \
            !__has_builtin(__builtin_types_compatible_p) && \
            !__has_builtin(__builtin_unreachable) && \
            !__has_builtin(__builtin_va_list) && \
            !__has_builtin(__builtin_va_arg) && \
            !__has_builtin(__builtin_va_copy)
        #define vm_ghost_has_vm_ghost_has_builtin 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_has_builtin
    #define vm_ghost_has_vm_ghost_has_builtin 1
#endif

/******************************************
 * ghost/preprocessor/ghost_has_include.h
 ******************************************/

#ifdef vm_ghost_has_include
    #error "vm_ghost_has_include cannot be defined. Define vm_ghost_has_vm_ghost_has_include to 1 or 0 to override Ghost's __has_include detection."
#endif

#ifndef vm_ghost_has_vm_ghost_has_include
    #ifndef __has_include
        #define vm_ghost_has_vm_ghost_has_include 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_has_include
    #if VM_GHOST_IMPL_AMALGAMATED
        #if !__has_include(<stddef.h>)
            #define vm_ghost_has_vm_ghost_has_include 0
        #endif
    #else
        #if !__has_include("ghost/vm_ghost_core.h")
            #define vm_ghost_has_vm_ghost_has_include 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_has_include
    #if __has_include("ghost/impl/nonexistent/vm_ghost_impl_nonexistent_header.h")
        #define vm_ghost_has_vm_ghost_has_include 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_has_include
    #define vm_ghost_has_vm_ghost_has_include 1
#endif

/***********************************************
 * ghost/preprocessor/ghost_msvc_traditional.h
 ***********************************************/

#ifndef vm_ghost_has_VM_GHOST_MSVC_TRADITIONAL
    #ifdef VM_GHOST_MSVC_TRADITIONAL
        #define vm_ghost_has_VM_GHOST_MSVC_TRADITIONAL 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MSVC_TRADITIONAL
    #ifdef _MSC_VER
        #ifdef _MSVC_TRADITIONAL
            #if _MSVC_TRADITIONAL
                #define VM_GHOST_MSVC_TRADITIONAL 1
            #else
                #define VM_GHOST_MSVC_TRADITIONAL 0
            #endif
        #else
            #define VM_GHOST_MSVC_TRADITIONAL 1
        #endif
        #define vm_ghost_has_VM_GHOST_MSVC_TRADITIONAL 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MSVC_TRADITIONAL
    #define VM_GHOST_MSVC_TRADITIONAL 0
    #define vm_ghost_has_VM_GHOST_MSVC_TRADITIONAL 1
#endif

/************************************
 * ghost/language/ghost_cplusplus.h
 ************************************/

#ifndef vm_ghost_has_vm_ghost_cplusplus
    #ifdef vm_ghost_cplusplus
        #define vm_ghost_has_vm_ghost_cplusplus 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cplusplus
    #ifndef __cplusplus
        #define vm_ghost_has_vm_ghost_cplusplus 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cplusplus
    #ifdef _MSVC_LANG
        #define vm_ghost_cplusplus _MSVC_LANG
        #define vm_ghost_has_vm_ghost_cplusplus 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cplusplus
    #define vm_ghost_cplusplus __cplusplus
    #define vm_ghost_has_vm_ghost_cplusplus 1
#endif

/****************************
 * ghost/detect/ghost_gcc.h
 ****************************/

#ifndef vm_ghost_has_VM_GHOST_GCC
    #ifdef VM_GHOST_GCC
        #define vm_ghost_has_VM_GHOST_GCC 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_GCC

    #if defined(__GNUC__) && \
            !defined(_MSC_VER)  && \
            !defined(__CPARSER__) && \
            !VM_GHOST_CPROC && \
            !defined(__DMC__)  && \
            !defined(__INTEL_COMPILER) && \
            !defined(__KEFIRCC__) && \
            !defined(__LCC__) && \
            !defined(__PCC__)  && \
            !defined(__PGI) && !defined(__NVCOMPILER)  && \
            !defined(__chibicc__) && \
            !defined(__clang__) && \
            !defined(__lacc__)
        #define VM_GHOST_GCC 1
    #else
        #define VM_GHOST_GCC 0
    #endif
    #define vm_ghost_has_VM_GHOST_GCC 1
#endif

/*****************************************
 * ghost/preprocessor/ghost_pp_va_args.h
 *****************************************/

#ifndef vm_ghost_has_VM_GHOST_PP_VA_ARGS
    #ifdef VM_GHOST_PP_VA_ARGS
        #define vm_ghost_has_VM_GHOST_PP_VA_ARGS 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_PP_VA_ARGS
    #ifdef __STDC_VERSION__
        #if __STDC_VERSION__ >= 199901L
            #define VM_GHOST_PP_VA_ARGS 1
            #define vm_ghost_has_VM_GHOST_PP_VA_ARGS 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_PP_VA_ARGS
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 201103L
            #define VM_GHOST_PP_VA_ARGS 1
            #define vm_ghost_has_VM_GHOST_PP_VA_ARGS 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_PP_VA_ARGS
    #ifdef __GNUC__
        #if VM_GHOST_GCC
            #if __GNUC__ >= 5
                #ifdef __STRICT_ANSI__
                    #if __STRICT_ANSI__
                        #define VM_GHOST_PP_VA_ARGS 0
                    #else
                        #define VM_GHOST_PP_VA_ARGS 1
                    #endif
                #else
                    #define VM_GHOST_PP_VA_ARGS 1
                #endif
                #define vm_ghost_has_VM_GHOST_PP_VA_ARGS 1
            #endif
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_PP_VA_ARGS
    #if defined(_MSC_VER)
        #define VM_GHOST_PP_VA_ARGS 1
    #else
        #define VM_GHOST_PP_VA_ARGS 0
    #endif
    #define vm_ghost_has_VM_GHOST_PP_VA_ARGS 1
#endif

/*************************************
 * ghost/preprocessor/ghost_expand.h
 *************************************/

#ifndef vm_ghost_has_VM_GHOST_EXPAND
    #ifdef VM_GHOST_EXPAND
        #define vm_ghost_has_VM_GHOST_EXPAND 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_EXPAND
    #if VM_GHOST_PP_VA_ARGS
        #define VM_GHOST_EXPAND(...) __VA_ARGS__
    #else
        #define VM_GHOST_EXPAND(x) x
    #endif
    #define vm_ghost_has_VM_GHOST_EXPAND 1
#endif

/**************************************
 * ghost/preprocessor/ghost_nothing.h
 **************************************/

#ifndef vm_ghost_has_VM_GHOST_NOTHING
    #ifndef VM_GHOST_NOTHING
        #define VM_GHOST_NOTHING 
    #endif
    #define vm_ghost_has_VM_GHOST_NOTHING 1
#endif

/*******************************************
 * ghost/preprocessor/ghost_omit_va_args.h
 *******************************************/

#ifndef vm_ghost_has_VM_GHOST_OMIT_VA_ARGS
    #ifdef VM_GHOST_OMIT_VA_ARGS
        #define vm_ghost_has_VM_GHOST_OMIT_VA_ARGS 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_OMIT_VA_ARGS
    #if !VM_GHOST_PP_VA_ARGS
        #define VM_GHOST_OMIT_VA_ARGS 0
        #define vm_ghost_has_VM_GHOST_OMIT_VA_ARGS 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_OMIT_VA_ARGS
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 202002L
            #define VM_GHOST_OMIT_VA_ARGS 1
            #define vm_ghost_has_VM_GHOST_OMIT_VA_ARGS 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_OMIT_VA_ARGS
    #ifdef _MSC_VER
        #define VM_GHOST_OMIT_VA_ARGS 1
        #define vm_ghost_has_VM_GHOST_OMIT_VA_ARGS 1
    #endif

#endif

#ifndef vm_ghost_has_VM_GHOST_OMIT_VA_ARGS
    #if VM_GHOST_GCC
        #if __GNUC__ <= 7
            #define VM_GHOST_OMIT_VA_ARGS 0
        #elif defined __STRICT_ANSI__
            #if __STRICT_ANSI__ != 0
                #define VM_GHOST_OMIT_VA_ARGS 0
            #else
                #define VM_GHOST_OMIT_VA_ARGS 1
            #endif
        #else
            #define VM_GHOST_OMIT_VA_ARGS 1
        #endif
        #define vm_ghost_has_VM_GHOST_OMIT_VA_ARGS 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_OMIT_VA_ARGS
    #ifdef __TINYC__
        #define VM_GHOST_OMIT_VA_ARGS 1
        #define vm_ghost_has_VM_GHOST_OMIT_VA_ARGS 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_OMIT_VA_ARGS
    #define VM_GHOST_OMIT_VA_ARGS 0
    #define vm_ghost_has_VM_GHOST_OMIT_VA_ARGS 1
#endif

/********************************************
 * ghost/language/ghost_extern_c_push_pop.h
 ********************************************/

#ifndef vm_ghost_has_VM_GHOST_EXTERN_C_PUSH
    #ifdef VM_GHOST_EXTERN_C_PUSH
        #define vm_ghost_has_VM_GHOST_EXTERN_C_PUSH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_EXTERN_C_PUSH
    #ifdef __cplusplus
        #define VM_GHOST_EXTERN_C_PUSH extern "C" {
    #else
        #define VM_GHOST_EXTERN_C_PUSH 
    #endif
    #define vm_ghost_has_VM_GHOST_EXTERN_C_PUSH 1
#endif

#ifndef vm_ghost_has_VM_GHOST_EXTERN_C_POP
    #ifdef VM_GHOST_EXTERN_C_POP
        #define vm_ghost_has_VM_GHOST_EXTERN_C_POP 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_EXTERN_C_POP
    #ifdef __cplusplus
        #define VM_GHOST_EXTERN_C_POP }
    #else
        #define VM_GHOST_EXTERN_C_POP 
    #endif
    #define vm_ghost_has_VM_GHOST_EXTERN_C_POP 1
#endif

/******************************************
 * ghost/language/ghost_hidden_push_pop.h
 ******************************************/

#ifndef vm_ghost_has_VM_GHOST_HIDDEN_PUSH
    #ifdef VM_GHOST_HIDDEN_PUSH
        #define vm_ghost_has_VM_GHOST_HIDDEN_PUSH 1
    #endif
#endif
#ifndef vm_ghost_has_VM_GHOST_HIDDEN_POP
    #ifdef VM_GHOST_HIDDEN_POP
        #define vm_ghost_has_VM_GHOST_HIDDEN_POP 1
    #endif
#endif

#if defined(__GNUC__) && !defined(__CPARSER__)
    #ifdef VM_GHOST_IMPL_HIDDEN_PUSH_PRAGMA
        #error
    #endif

    #if VM_GHOST_GCC
        #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
            #define VM_GHOST_IMPL_HIDDEN_PUSH_PRAGMA
        #endif
    #else
        #define VM_GHOST_IMPL_HIDDEN_PUSH_PRAGMA
    #endif

    #ifdef VM_GHOST_IMPL_HIDDEN_PUSH_PRAGMA
        #undef VM_GHOST_IMPL_HIDDEN_PUSH_PRAGMA
        #ifndef vm_ghost_has_VM_GHOST_HIDDEN_PUSH
            #define VM_GHOST_HIDDEN_PUSH _Pragma("GCC visibility push(hidden)")
            #define vm_ghost_has_VM_GHOST_HIDDEN_PUSH 1
        #endif
        #ifndef vm_ghost_has_VM_GHOST_HIDDEN_POP
            #define VM_GHOST_HIDDEN_POP _Pragma("GCC visibility pop")
            #define vm_ghost_has_VM_GHOST_HIDDEN_POP 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_HIDDEN_PUSH
    #define VM_GHOST_HIDDEN_PUSH 
    #define vm_ghost_has_VM_GHOST_HIDDEN_PUSH 1
#endif
#ifndef vm_ghost_has_VM_GHOST_HIDDEN_POP
    #define VM_GHOST_HIDDEN_POP 
    #define vm_ghost_has_VM_GHOST_HIDDEN_POP 1
#endif

/*********************************
 * ghost/language/ghost_inline.h
 *********************************/

#ifndef vm_ghost_has_vm_ghost_inline
    #ifdef vm_ghost_inline
        #define vm_ghost_has_vm_ghost_inline 1
    #endif
#endif

#if 0
#ifndef vm_ghost_has_vm_ghost_inline
    #ifdef __TINYC__
        #define vm_ghost_has_vm_ghost_inline 0
    #endif
#endif
#endif

#ifndef vm_ghost_has_vm_ghost_inline
    #ifdef __STDC_VERSION__
        #if __STDC_VERSION__ >= 199901L
            #define vm_ghost_inline inline
            #define vm_ghost_has_vm_ghost_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_inline
    #ifdef cplusplus
        #define vm_ghost_inline inline
        #define vm_ghost_has_vm_ghost_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_inline
    #ifdef __GNUC__
        #define vm_ghost_inline __inline__
        #define vm_ghost_has_vm_ghost_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_inline
    #if defined(_MSC_VER)
        #if _MSC_VER >= 1900
            #define vm_ghost_inline __inline
            #define vm_ghost_has_vm_ghost_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_inline
    #define vm_ghost_has_vm_ghost_inline 0
#endif

/***************************************
 * ghost/language/ghost_stdc_version.h
 ***************************************/

#ifndef vm_ghost_has_VM_GHOST_STDC_VERSION
    #ifdef VM_GHOST_STDC_VERSION
        #define vm_ghost_has_VM_GHOST_STDC_VERSION 1
    #elif defined(__STDC_VERSION__)
        #define VM_GHOST_STDC_VERSION __STDC_VERSION__
        #define vm_ghost_has_VM_GHOST_STDC_VERSION 1
    #else
        #define vm_ghost_has_VM_GHOST_STDC_VERSION 0
    #endif
#endif

/********************************
 * ghost/impl/ghost_impl_defs.h
 ********************************/

#ifndef VM_GHOST_MANUAL_DEFS
    #define VM_GHOST_MANUAL_DEFS 0
#endif

#if VM_GHOST_MANUAL_DEFS
    #ifndef VM_GHOST_EMIT_DEFS
        #define VM_GHOST_EMIT_DEFS 0
    #endif
#else
    #ifdef VM_GHOST_EMIT_DEFS
        #error "You cannot pre-define VM_GHOST_EMIT_DEFS unless VM_GHOST_MANUAL_DEFS is enabled."
    #endif
    #define VM_GHOST_EMIT_DEFS 1
#endif

#if VM_GHOST_PP_VA_ARGS
    #if VM_GHOST_EMIT_DEFS
        #define VM_GHOST_IMPL_DEF(...) __VA_ARGS__
    #else
        #define VM_GHOST_IMPL_DEF(...) ;
    #endif
#else
    #if VM_GHOST_EMIT_DEFS
        #define VM_GHOST_IMPL_DEF(x) x
    #else
        #define VM_GHOST_IMPL_DEF(x) ;
    #endif
#endif

/*******************************************
 * ghost/silence/ghost_silence_long_long.h
 *******************************************/

#ifndef vm_ghost_has_VM_GHOST_SILENCE_LONG_LONG
    #ifdef VM_GHOST_SILENCE_LONG_LONG
        #define vm_ghost_has_VM_GHOST_SILENCE_LONG_LONG 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_LONG_LONG
    #if defined(__GNUC__) && !defined(__CPARSER__)
        #if VM_GHOST_GCC
            #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
                #define VM_GHOST_SILENCE_LONG_LONG _Pragma("GCC diagnostic ignored \"-Wlong-long\"")
                #define vm_ghost_has_VM_GHOST_SILENCE_LONG_LONG 1
            #endif
        #else
            #define VM_GHOST_SILENCE_LONG_LONG _Pragma("GCC diagnostic ignored \"-Wlong-long\"")
            #define vm_ghost_has_VM_GHOST_SILENCE_LONG_LONG 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_LONG_LONG
    #define VM_GHOST_SILENCE_LONG_LONG 
    #define vm_ghost_has_VM_GHOST_SILENCE_LONG_LONG 1
#endif

/***************************************************************
 * ghost/silence/ghost_silence_missing_function_declarations.h
 ***************************************************************/

#ifndef vm_ghost_has_VM_GHOST_SILENCE_MISSING_FUNCTION_DECLARATIONS
    #ifdef VM_GHOST_SILENCE_MISSING_FUNCTION_DECLARATIONS
        #define vm_ghost_has_VM_GHOST_SILENCE_MISSING_FUNCTION_DECLARATIONS 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_MISSING_FUNCTION_DECLARATIONS
    #if defined(__GNUC__) && !defined(__CPARSER__)
        #if defined(__clang__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
            #if defined(__cplusplus) && !defined(__clang__)
                #define VM_GHOST_SILENCE_MISSING_FUNCTION_DECLARATIONS \
                        _Pragma("GCC diagnostic ignored \"-Wmissing-declarations\"")
            #else
                #define VM_GHOST_SILENCE_MISSING_FUNCTION_DECLARATIONS \
                        _Pragma("GCC diagnostic ignored \"-Wmissing-declarations\"") \
                        _Pragma("GCC diagnostic ignored \"-Wmissing-prototypes\"")
            #endif
            #define vm_ghost_has_VM_GHOST_SILENCE_MISSING_FUNCTION_DECLARATIONS 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_MISSING_FUNCTION_DECLARATIONS
    #define VM_GHOST_SILENCE_MISSING_FUNCTION_DECLARATIONS 
    #define vm_ghost_has_VM_GHOST_SILENCE_MISSING_FUNCTION_DECLARATIONS 1
#endif

/***************************************************************
 * ghost/silence/ghost_silence_missing_variable_declarations.h
 ***************************************************************/

#ifndef vm_ghost_has_VM_GHOST_SILENCE_MISSING_VARIABLE_DECLARATIONS
    #ifdef VM_GHOST_SILENCE_MISSING_VARIABLE_DECLARATIONS
        #define vm_ghost_has_VM_GHOST_SILENCE_MISSING_VARIABLE_DECLARATIONS 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_MISSING_VARIABLE_DECLARATIONS
    #if defined(__clang__)
        #define VM_GHOST_SILENCE_MISSING_VARIABLE_DECLARATIONS \
                _Pragma("GCC diagnostic ignored \"-Wmissing-variable-declarations\"")
        #define vm_ghost_has_VM_GHOST_SILENCE_MISSING_VARIABLE_DECLARATIONS 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_MISSING_VARIABLE_DECLARATIONS
    #define VM_GHOST_SILENCE_MISSING_VARIABLE_DECLARATIONS 
    #define vm_ghost_has_VM_GHOST_SILENCE_MISSING_VARIABLE_DECLARATIONS 1
#endif

/******************************************************
 * ghost/silence/ghost_silence_missing_declarations.h
 ******************************************************/

#ifndef vm_ghost_has_VM_GHOST_SILENCE_MISSING_DECLARATIONS
    #ifdef VM_GHOST_SILENCE_MISSING_DECLARATIONS
        #define vm_ghost_has_VM_GHOST_SILENCE_MISSING_DECLARATIONS 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_MISSING_DECLARATIONS
    #define VM_GHOST_SILENCE_MISSING_DECLARATIONS \
        VM_GHOST_SILENCE_MISSING_FUNCTION_DECLARATIONS \
        VM_GHOST_SILENCE_MISSING_VARIABLE_DECLARATIONS
    #define vm_ghost_has_VM_GHOST_SILENCE_MISSING_DECLARATIONS 1
#endif

/******************************************
 * ghost/silence/ghost_silence_push_pop.h
 ******************************************/

#ifndef vm_ghost_has_VM_GHOST_SILENCE_PUSH
    #ifdef VM_GHOST_SILENCE_PUSH
        #define vm_ghost_has_VM_GHOST_SILENCE_PUSH 1
    #endif
#endif
#ifndef vm_ghost_has_VM_GHOST_SILENCE_POP
    #ifdef VM_GHOST_SILENCE_POP
        #define vm_ghost_has_VM_GHOST_SILENCE_POP 1
    #endif
#endif

#if defined(__GNUC__) && !defined(__CPARSER__)
    #if defined(__clang__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
        #ifndef vm_ghost_has_VM_GHOST_SILENCE_PUSH
            #define VM_GHOST_SILENCE_PUSH _Pragma("GCC diagnostic push")
            #define vm_ghost_has_VM_GHOST_SILENCE_PUSH 1
        #endif
        #ifndef vm_ghost_has_VM_GHOST_SILENCE_POP
            #define VM_GHOST_SILENCE_POP _Pragma("GCC diagnostic pop")
            #define vm_ghost_has_VM_GHOST_SILENCE_POP 1
        #endif
    #endif
#endif

#ifdef _MSC_VER
    #ifndef vm_ghost_has_VM_GHOST_SILENCE_PUSH
        #define VM_GHOST_SILENCE_PUSH __pragma(warning(push))
        #define vm_ghost_has_VM_GHOST_SILENCE_PUSH 1
    #endif
    #ifndef vm_ghost_has_VM_GHOST_SILENCE_POP
        #define VM_GHOST_SILENCE_POP __pragma(warning(pop))
        #define vm_ghost_has_VM_GHOST_SILENCE_POP 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_PUSH
    #define vm_ghost_has_VM_GHOST_SILENCE_PUSH 0
#endif
#ifndef vm_ghost_has_VM_GHOST_SILENCE_POP
    #define vm_ghost_has_VM_GHOST_SILENCE_POP 0
#endif

/*************************************************
 * ghost/silence/ghost_silence_unused_function.h
 *************************************************/

#ifndef vm_ghost_has_VM_GHOST_SILENCE_UNUSED_FUNCTION
    #ifdef VM_GHOST_SILENCE_UNUSED_FUNCTION
        #define vm_ghost_has_VM_GHOST_SILENCE_UNUSED_FUNCTION 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_UNUSED_FUNCTION
    #if defined(__GNUC__) && !defined(__CPARSER__)
        #if defined(__clang__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
            #define VM_GHOST_SILENCE_UNUSED_FUNCTION \
                    _Pragma("GCC diagnostic ignored \"-Wunused-function\"")
            #define vm_ghost_has_VM_GHOST_SILENCE_UNUSED_FUNCTION 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_UNUSED_FUNCTION
    #define VM_GHOST_SILENCE_UNUSED_FUNCTION 
    #define vm_ghost_has_VM_GHOST_SILENCE_UNUSED_FUNCTION 1
#endif

/********************************************
 * ghost/impl/ghost_impl_function_wrapper.h
 ********************************************/

#if defined(__GNUC__)
    #define VM_GHOST_IMPL_FUNCTION_SILENCE_MISSING_DECLARATIONS \
        VM_GHOST_SILENCE_MISSING_DECLARATIONS
#endif

#if defined(VM_GHOST_IMPL_FUNCTION_SILENCE_MISSING_DECLARATIONS)
    #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
        #define VM_GHOST_IMPL_FUNCTION_SILENCE_PUSH VM_GHOST_SILENCE_PUSH
        #define VM_GHOST_IMPL_FUNCTION_SILENCE_POP VM_GHOST_SILENCE_POP
    #endif
#endif

#ifndef VM_GHOST_IMPL_FUNCTION_SILENCE_PUSH
    #define VM_GHOST_IMPL_FUNCTION_SILENCE_PUSH 
#endif
#ifndef VM_GHOST_IMPL_FUNCTION_SILENCE_POP
    #define VM_GHOST_IMPL_FUNCTION_SILENCE_POP 
#endif
#ifndef VM_GHOST_IMPL_FUNCTION_SILENCE_MISSING_DECLARATIONS
    #define VM_GHOST_IMPL_FUNCTION_SILENCE_MISSING_DECLARATIONS 
#endif

#define VM_GHOST_IMPL_FUNCTION_OPEN \
    VM_GHOST_EXTERN_C_PUSH \
    VM_GHOST_HIDDEN_PUSH \
    VM_GHOST_IMPL_FUNCTION_SILENCE_PUSH \
    VM_GHOST_IMPL_FUNCTION_SILENCE_MISSING_DECLARATIONS \
    VM_GHOST_SILENCE_LONG_LONG \
    VM_GHOST_SILENCE_UNUSED_FUNCTION

#define VM_GHOST_IMPL_FUNCTION_CLOSE \
    VM_GHOST_IMPL_FUNCTION_SILENCE_POP \
    VM_GHOST_HIDDEN_POP \
    VM_GHOST_EXTERN_C_POP

#define VM_GHOST_IMPL_CXX_FUNCTION_OPEN \
    VM_GHOST_HIDDEN_PUSH \
    VM_GHOST_IMPL_FUNCTION_SILENCE_PUSH \
    VM_GHOST_IMPL_FUNCTION_SILENCE_MISSING_DECLARATIONS

#define VM_GHOST_IMPL_CXX_FUNCTION_CLOSE \
    VM_GHOST_IMPL_FUNCTION_SILENCE_POP \
    VM_GHOST_HIDDEN_POP

#ifndef VM_GHOST_STATIC_DEFS
    #define VM_GHOST_STATIC_DEFS 0
#endif
#if VM_GHOST_STATIC_DEFS && VM_GHOST_MANUAL_DEFS
    #error
#endif

#ifdef _MSC_VER
__pragma(warning(disable:4505)) 
#endif

/************************************************************
 * ghost/impl/language/ghost_impl_attribute_namespace_gnu.h
 ************************************************************/

/* TODO this version check won't work on e.g. Apple Clang and other variants */
#ifndef VM_GHOST_IMPL_ATTRIBUTE_NAMESPACE_GNU
    #if defined(__clang__)
        #if __clang_major__ < 8
            #define VM_GHOST_IMPL_ATTRIBUTE_NAMESPACE_GNU gnu
        #else
            #define VM_GHOST_IMPL_ATTRIBUTE_NAMESPACE_GNU __gnu__
        #endif
    #endif
#endif

#ifndef VM_GHOST_IMPL_ATTRIBUTE_NAMESPACE_GNU
    #if VM_GHOST_GCC
        #if __GNUC__ < 8
            #define VM_GHOST_IMPL_ATTRIBUTE_NAMESPACE_GNU gnu
        #else
            #define VM_GHOST_IMPL_ATTRIBUTE_NAMESPACE_GNU __gnu__
        #endif
    #endif
#endif

#ifndef VM_GHOST_IMPL_ATTRIBUTE_NAMESPACE_GNU
    #ifdef __GNUC__
        #define VM_GHOST_IMPL_ATTRIBUTE_NAMESPACE_GNU __gnu__
    #endif
#endif

/*******************************************************
 * ghost/impl/language/ghost_impl_standard_attribute.h
 *******************************************************/

#ifdef VM_GHOST_IMPL_STANDARD_ATTRIBUTE
    #error
#endif

#ifndef VM_GHOST_IMPL_STANDARD_ATTRIBUTE
    #ifdef __GNUC__
        #if VM_GHOST_GCC
            #if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8)
                #define VM_GHOST_IMPL_STANDARD_ATTRIBUTE 0
            #endif
        #endif
    #endif
#endif

#ifndef VM_GHOST_IMPL_STANDARD_ATTRIBUTE
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 201103L
            #define VM_GHOST_IMPL_STANDARD_ATTRIBUTE 1
        #endif
    #endif
#endif

#ifndef VM_GHOST_IMPL_STANDARD_ATTRIBUTE
    #ifdef __STDC_VERSION__
        #if VM_GHOST_STDC_VERSION >= 202000L && !VM_GHOST_IMPL_ENABLE_C23
            #define VM_GHOST_IMPL_STANDARD_ATTRIBUTE 1
        #endif
    #endif
#endif

#ifndef VM_GHOST_IMPL_STANDARD_ATTRIBUTE
    #define VM_GHOST_IMPL_STANDARD_ATTRIBUTE 0
#endif

/****************************************
 * ghost/language/ghost_always_inline.h
 ****************************************/

#ifndef vm_ghost_has_vm_ghost_always_inline
    #ifdef vm_ghost_always_inline
        #define vm_ghost_has_vm_ghost_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_always_inline
    #if defined(__GNUC__) && !defined(__PGI)
        #if VM_GHOST_IMPL_STANDARD_ATTRIBUTE
            #define vm_ghost_always_inline [[VM_GHOST_IMPL_ATTRIBUTE_NAMESPACE_GNU::__always_inline__]] inline
            #define vm_ghost_has_vm_ghost_always_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_always_inline
    #ifdef _MSC_VER
        #define vm_ghost_always_inline __forceinline
        #define vm_ghost_has_vm_ghost_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_always_inline
    #if defined(__GNUC__)
        #define vm_ghost_always_inline vm_ghost_inline __attribute__((__always_inline__))
        #define vm_ghost_has_vm_ghost_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_always_inline
    #if vm_ghost_has(vm_ghost_inline)
        #define vm_ghost_always_inline vm_ghost_inline
        #define vm_ghost_has_vm_ghost_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_always_inline
    #define vm_ghost_has_vm_ghost_always_inline 0
#endif

/*********************************************
 * ghost/language/ghost_emit_always_inline.h
 *********************************************/

#ifndef vm_ghost_has_vm_ghost_emit_always_inline
    #ifdef vm_ghost_emit_always_inline
        #define vm_ghost_has_vm_ghost_emit_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_always_inline
    #if !vm_ghost_has(vm_ghost_always_inline)
        #define vm_ghost_has_vm_ghost_emit_always_inline 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_always_inline
    #ifdef __cplusplus
        #define vm_ghost_emit_always_inline vm_ghost_always_inline
        #define vm_ghost_has_vm_ghost_emit_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_always_inline
    #ifdef _MSC_VER
        #if _MSC_VER >= 1900
            #define vm_ghost_emit_always_inline vm_ghost_always_inline
            #define vm_ghost_has_vm_ghost_emit_always_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_always_inline
    #if defined(__GNUC__) && (defined(__GNUC_GNU_INLINE__) || \
            (!defined(__GNUC_STDC_INLINE__) && !defined(__GNUC_GNU_INLINE__)))
        #define vm_ghost_emit_always_inline vm_ghost_always_inline
        #define vm_ghost_has_vm_ghost_emit_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_always_inline
    #if defined(__STDC_VERSION__)
        #if VM_GHOST_STDC_VERSION >= 199901L
            #define vm_ghost_emit_always_inline extern vm_ghost_always_inline
            #define vm_ghost_has_vm_ghost_emit_always_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_always_inline
    #define vm_ghost_has_vm_ghost_emit_always_inline 0
#endif

/***************************************
 * ghost/language/ghost_maybe_unused.h
 ***************************************/

#ifndef vm_ghost_has_vm_ghost_maybe_unused
    #if defined(__cplusplus)
        #if vm_ghost_cplusplus >= 201703L
            #define vm_ghost_maybe_unused [[maybe_unused]]
            #define vm_ghost_has_vm_ghost_maybe_unused 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_maybe_unused
    #if defined(__GNUC__)
        #define vm_ghost_maybe_unused __attribute__((__unused__))
        #define vm_ghost_has_vm_ghost_maybe_unused 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_maybe_unused
    #define vm_ghost_maybe_unused 
    #define vm_ghost_has_vm_ghost_maybe_unused 1
#endif

/***********************************************
 * ghost/language/ghost_header_always_inline.h
 ***********************************************/

#ifndef vm_ghost_has_vm_ghost_header_always_inline
    #ifdef vm_ghost_header_always_inline
        #define vm_ghost_has_vm_ghost_header_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_always_inline
    #if defined(__cplusplus)
        #define vm_ghost_header_always_inline vm_ghost_always_inline
        #define vm_ghost_has_vm_ghost_header_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_always_inline
    #if defined(_MSC_VER)
        #if _MSC_VER >= 1900
            #define vm_ghost_header_always_inline vm_ghost_always_inline
            #define vm_ghost_has_vm_ghost_header_always_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_always_inline
    #if vm_ghost_has(vm_ghost_always_inline)
        #define vm_ghost_header_always_inline vm_ghost_always_inline static
        #define vm_ghost_has_vm_ghost_header_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_always_inline
    #define vm_ghost_header_always_inline vm_ghost_maybe_unused static
    #define vm_ghost_has_vm_ghost_header_always_inline 1
#endif

/*********************************
 * ghost/language/ghost_hosted.h
 *********************************/

#ifndef vm_ghost_has_VM_GHOST_HOSTED
    #ifdef VM_GHOST_HOSTED
        #define vm_ghost_has_VM_GHOST_HOSTED 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_HOSTED
    #ifdef __DMC__
        #define VM_GHOST_HOSTED 1
        #define vm_ghost_has_VM_GHOST_HOSTED 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_HOSTED
    #ifdef __STDC_HOSTED__
        #if __STDC_HOSTED__
            #define VM_GHOST_HOSTED 1
        #else
            #define VM_GHOST_HOSTED 0
        #endif
        #define vm_ghost_has_VM_GHOST_HOSTED 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_HOSTED
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<stdio.h>)
            #define VM_GHOST_HOSTED 1
        #else
            #define VM_GHOST_HOSTED 0
        #endif
        #define vm_ghost_has_VM_GHOST_HOSTED 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_HOSTED
    #define VM_GHOST_HOSTED 1
    #define vm_ghost_has_VM_GHOST_HOSTED 1
#endif

/***********************************************
 * ghost/language/ghost_noemit_always_inline.h
 ***********************************************/

#ifndef vm_ghost_has_vm_ghost_noemit_always_inline
    #ifdef vm_ghost_noemit_always_inline
        #define vm_ghost_has_vm_ghost_noemit_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_always_inline
    #if !vm_ghost_has(vm_ghost_always_inline)
        #define vm_ghost_has_vm_ghost_emit_always_inline 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noemit_always_inline
    #ifdef __cplusplus
        #define vm_ghost_noemit_always_inline vm_ghost_always_inline
        #define vm_ghost_has_vm_ghost_noemit_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noemit_always_inline
    #ifdef _MSC_VER
        #if _MSC_VER >= 1900
            #define vm_ghost_noemit_always_inline vm_ghost_always_inline
            #define vm_ghost_has_vm_ghost_noemit_always_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noemit_always_inline
    #if defined(__GNUC__) && (defined(__GNUC_GNU_INLINE__) || \
            (!defined(__GNUC_STDC_INLINE__) && !defined(__GNUC_GNU_INLINE__)))
        #define vm_ghost_noemit_always_inline extern vm_ghost_always_inline
        #define vm_ghost_has_vm_ghost_noemit_always_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noemit_always_inline
    #if defined(__STDC_VERSION__)
        #if VM_GHOST_STDC_VERSION >= 199901L
            #define vm_ghost_noemit_always_inline vm_ghost_always_inline
            #define vm_ghost_has_vm_ghost_noemit_always_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noemit_always_inline
    #define vm_ghost_has_vm_ghost_noemit_always_inline 0
#endif

/*****************************************
 * ghost/impl/ghost_impl_always_inline.h
 *****************************************/

#if VM_GHOST_STATIC_DEFS
    #if vm_ghost_has_vm_ghost_always_inline
        #define vm_ghost_impl_always_inline vm_ghost_always_inline static
    #else
        #define vm_ghost_impl_always_inline static
    #endif
#elif VM_GHOST_MANUAL_DEFS
    #if VM_GHOST_EMIT_DEFS
        #define vm_ghost_impl_always_inline vm_ghost_emit_always_inline
    #else
        #define vm_ghost_impl_always_inline vm_ghost_noemit_always_inline
    #endif
#else
    #define vm_ghost_impl_always_inline vm_ghost_header_always_inline
#endif

/***********************************************
 * ghost/impl/header/ghost_impl_cosmopolitan.h
 ***********************************************/

#ifdef __COSMOPOLITAN__

    #ifndef vm_ghost_has_vm_ghost_errno_h
        #define vm_ghost_has_vm_ghost_errno_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_inttypes_h
        #define vm_ghost_has_vm_ghost_inttypes_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_limits_h
        #define vm_ghost_has_vm_ghost_limits_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_math_h
        #define vm_ghost_has_vm_ghost_math_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_signal_h
        #define vm_ghost_has_vm_ghost_signal_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_stdarg_h
        #define vm_ghost_has_vm_ghost_stdarg_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_stdbool_h
        #define vm_ghost_has_vm_ghost_stdbool_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_stddef_h
        #define vm_ghost_has_vm_ghost_stddef_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_stdint_h
        #define vm_ghost_has_vm_ghost_stdint_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_stdio_h
        #define vm_ghost_has_vm_ghost_stdio_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_stdlib_h
        #define vm_ghost_has_vm_ghost_stdlib_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_string_h
        #define vm_ghost_has_vm_ghost_string_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_time_h
        #define vm_ghost_has_vm_ghost_time_h 1
    #endif

    #ifndef vm_ghost_has_vm_ghost_pthread_h
        #define vm_ghost_has_vm_ghost_pthread_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_strings_h
        #define vm_ghost_has_vm_ghost_strings_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_sys_types_h
        #define vm_ghost_has_vm_ghost_sys_types_h 1
    #endif
    #ifndef vm_ghost_has_vm_ghost_unistd_h
        #define vm_ghost_has_vm_ghost_unistd_h 1
    #endif

#endif

/**********************************
 * ghost/header/c/ghost_errno_h.h
 **********************************/

#ifndef vm_ghost_has_vm_ghost_errno_h
    #ifdef VM_GHOST_ERRNO_H
        #include VM_GHOST_ERRNO_H
        #define vm_ghost_has_vm_ghost_errno_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_errno_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<errno.h>)
            #include <errno.h>
            #define vm_ghost_has_vm_ghost_errno_h 1
        #else
            #define vm_ghost_has_vm_ghost_errno_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_errno_h
    #if VM_GHOST_HOSTED
        #include <errno.h>
        #define vm_ghost_has_vm_ghost_errno_h 1
    #else
        #define vm_ghost_has_vm_ghost_errno_h 0
    #endif
#endif

#ifdef __cplusplus
    #if vm_ghost_has_vm_ghost_errno_h
        #if vm_ghost_has(vm_ghost_has_include)
            #if __has_include(<cerrno>)
                #include <cerrno>
            #endif
        #else
            #include <cerrno>
        #endif
    #endif
#endif

/***********************************
 * ghost/header/c/ghost_limits_h.h
 ***********************************/

#ifndef vm_ghost_has_vm_ghost_limits_h
    #ifdef VM_GHOST_LIMITS_H
        #include VM_GHOST_LIMITS_H
        #define vm_ghost_has_vm_ghost_limits_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_limits_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<limits.h>)
            #include <limits.h>
            #define vm_ghost_has_vm_ghost_limits_h 1
        #else
            #define vm_ghost_has_vm_ghost_limits_h 0
        #endif
    #else
        #include <limits.h>
        #define vm_ghost_has_vm_ghost_limits_h 1
    #endif
#endif

#ifdef __cplusplus
    #if vm_ghost_has_vm_ghost_limits_h
        #if vm_ghost_has(vm_ghost_has_include)
            #if __has_include(<climits>)
                #include <climits>
            #endif
        #else
            #include <climits>
        #endif
    #endif
#endif

/***********************************
 * ghost/header/c/ghost_signal_h.h
 ***********************************/

#ifndef vm_ghost_has_vm_ghost_signal_h
    #if !VM_GHOST_HOSTED
        #define vm_ghost_has_vm_ghost_signal_h 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_signal_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<signal.h>)
            #include <signal.h>
            #define vm_ghost_has_vm_ghost_signal_h 1
        #else
            #define vm_ghost_has_vm_ghost_signal_h 0
        #endif
    #else
        #include <signal.h>
        #define vm_ghost_has_vm_ghost_signal_h 1
    #endif
#endif

#ifdef __cplusplus
    #if vm_ghost_has_vm_ghost_signal_h
        #if vm_ghost_has(vm_ghost_has_include)
            #if __has_include(<csignal>)
                #include <csignal>
            #endif
        #else
            #include <csignal>
        #endif
    #endif
#endif

/************************************
 * ghost/header/c/ghost_stdbool_h.h
 ************************************/

#ifndef vm_ghost_has_vm_ghost_stdbool_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<stdbool.h>)
            #include <stdbool.h>
            #define vm_ghost_has_vm_ghost_stdbool_h 1
        #else
            #define vm_ghost_has_vm_ghost_stdbool_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdbool_h
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 201103L
            #include <stdbool.h>
            #define vm_ghost_has_vm_ghost_stdbool_h 1
        #endif
    #elif defined(__STDC_VERSION__)
        #if VM_GHOST_STDC_VERSION >= 199901L
            #include <stdbool.h>
            #define vm_ghost_has_vm_ghost_stdbool_h 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdbool_h
    #ifdef _MSC_VER
        #if _MSC_VER < 1800
            #define vm_ghost_has_vm_ghost_stdbool_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdbool_h
    #include <stdbool.h>
    #define vm_ghost_has_vm_ghost_stdbool_h 1
#endif

/**************************************
 * ghost/header/c/ghost_stdckdint_h.h
 **************************************/

#ifndef vm_ghost_has_vm_ghost_stdckdint_h
    #ifdef VM_GHOST_STDCKDINT_H
        #include VM_GHOST_STDCKDINT_H
        #define vm_ghost_has_vm_ghost_stdckdint_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdckdint_h
    #ifdef __STDC_VERSION__
        #if VM_GHOST_STDC_VERSION < 202000L || !VM_GHOST_IMPL_ENABLE_C23
            #define vm_ghost_has_vm_ghost_stdckdint_h 0
        #endif
    #else
        #define vm_ghost_has_vm_ghost_stdckdint_h 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdckdint_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<stdckdint.h>)
            #include <stdckdint.h>
            #define vm_ghost_has_vm_ghost_stdckdint_h 1
        #else
            #define vm_ghost_has_vm_ghost_stdckdint_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdckdint_h
    #if VM_GHOST_HOSTED
        #include <stdckdint.h>
        #define vm_ghost_has_vm_ghost_stdckdint_h 1
    #else
        #define vm_ghost_has_vm_ghost_stdckdint_h 0
    #endif
#endif

/***********************************
 * ghost/header/c/ghost_stddef_h.h
 ***********************************/

#ifndef vm_ghost_has_vm_ghost_stddef_h
    #ifdef VM_GHOST_STDDEF_H
        #include VM_GHOST_STDDEF_H
        #define vm_ghost_has_vm_ghost_stddef_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stddef_h

    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<stddef.h>)
            #include <stddef.h>
            #define vm_ghost_has_vm_ghost_stddef_h 1
        #else
            #define vm_ghost_has_vm_ghost_stddef_h 0
        #endif

    #else
        #include <stddef.h>
        #define vm_ghost_has_vm_ghost_stddef_h 1
    #endif
#endif

#ifdef __cplusplus
    #if vm_ghost_has_vm_ghost_stddef_h
        #if vm_ghost_has(vm_ghost_has_include)
            #if __has_include(<cstddef>)
                #include <cstddef>
            #endif
        #else
            #include <cstddef>
        #endif
    #endif
#endif

/**************************************
 * ghost/header/cxx/ghost_cxx_bit_h.h
 **************************************/

#ifndef vm_ghost_has_vm_ghost_cxx_bit_h
    #ifndef __cplusplus
        #define vm_ghost_has_vm_ghost_cxx_bit_h 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cxx_bit_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<bit>)
            #include <bit>
            #define vm_ghost_has_vm_ghost_cxx_bit_h 1
        #else
            #define vm_ghost_has_vm_ghost_cxx_bit_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cxx_bit_h
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 202002L
            #include <bit>
            #define vm_ghost_has_vm_ghost_cxx_bit_h 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cxx_bit_h
    #define vm_ghost_has_vm_ghost_cxx_bit_h 0
#endif

/**********************************************
 * ghost/header/cxx/ghost_cxx_type_traits_h.h
 **********************************************/

#ifndef vm_ghost_has_vm_ghost_cxx_type_traits_h
    #ifndef __cplusplus
        #define vm_ghost_has_vm_ghost_cxx_type_traits_h 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cxx_type_traits_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<type_traits>)
            #include <type_traits>
            #define vm_ghost_has_vm_ghost_cxx_type_traits_h 1
        #else
            #define vm_ghost_has_vm_ghost_cxx_type_traits_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cxx_type_traits_h
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 201103L
            #include <type_traits>
            #define vm_ghost_has_vm_ghost_cxx_type_traits_h 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cxx_type_traits_h
    #define vm_ghost_has_vm_ghost_cxx_type_traits_h 0
#endif

/******************************************
 * ghost/header/cxx/ghost_cxx_version_h.h
 ******************************************/

#ifndef vm_ghost_has_vm_ghost_cxx_version_h
    #ifndef __cplusplus
        #define vm_ghost_has_vm_ghost_cxx_version_h 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cxx_version_h
    #ifdef __cplusplus
        #if vm_ghost_has(vm_ghost_has_include)
            #if __has_include(<version>)
                #include <version>
                #define vm_ghost_has_vm_ghost_cxx_version_h 1
            #endif
        #else
            #if vm_ghost_cplusplus >= 202002L
                #include <version>
                #define vm_ghost_has_vm_ghost_cxx_version_h 1
            #endif
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cxx_version_h
    #ifdef __cplusplus
        #if vm_ghost_has(vm_ghost_has_include)
            #if __has_include(<ciso646>)
                #include <ciso646>
                #define vm_ghost_has_vm_ghost_cxx_version_h 1
            #endif
        #else
            #include <ciso646>
            #define vm_ghost_has_vm_ghost_cxx_version_h 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cxx_version_h
    #define vm_ghost_has_vm_ghost_cxx_version_h 0
#endif

/*******************************
 * ghost/language/ghost_cold.h
 *******************************/

#ifndef vm_ghost_has_vm_ghost_cold
    #ifdef vm_ghost_cold
        #define vm_ghost_has_vm_ghost_cold 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cold
    #if defined(__clang__)
        #if VM_GHOST_IMPL_STANDARD_ATTRIBUTE
            #define vm_ghost_cold [[VM_GHOST_IMPL_ATTRIBUTE_NAMESPACE_GNU::__cold__]]
            #define vm_ghost_has_vm_ghost_cold 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cold
    #if defined(__GNUC__) && !defined(__CPARSER__) && !defined(__PCC__)
        #if VM_GHOST_GCC
            #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
                #define vm_ghost_cold __attribute__((__cold__))
                #define vm_ghost_has_vm_ghost_cold 1
            #endif
        #else
            #define vm_ghost_cold __attribute__((__cold__))
            #define vm_ghost_has_vm_ghost_cold 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_cold
    #define vm_ghost_cold 
    #define vm_ghost_has_vm_ghost_cold 1
#endif

/**************************************
 * ghost/language/ghost_static_cast.h
 **************************************/

#ifndef vm_ghost_has_vm_ghost_static_cast
    #ifdef vm_ghost_static_cast
        #define vm_ghost_has_vm_ghost_static_cast 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_static_cast
    #ifdef __cplusplus
        #define vm_ghost_static_cast(Type, value) (static_cast<Type>(value))
    #else
        #define vm_ghost_static_cast(Type, value) ((Type)(value))
    #endif
    #define vm_ghost_has_vm_ghost_static_cast 1
#endif

/**********************************
 * ghost/language/ghost_discard.h
 **********************************/

#ifndef vm_ghost_has_vm_ghost_discard
    #ifndef vm_ghost_discard
        #define vm_ghost_discard(x) vm_ghost_static_cast(void, (x))
    #endif
    #define vm_ghost_has_vm_ghost_discard 1
#endif

/************************************
 * ghost/type/size_t/ghost_size_t.h
 ************************************/

#if VM_GHOST_DOCUMENTATION
typedef size_t vm_ghost_size_t;
#define vm_ghost_has_vm_ghost_size_t 1
#endif

#ifndef vm_ghost_has_vm_ghost_size_t
    #ifdef vm_ghost_size_t
        #define vm_ghost_has_vm_ghost_size_t 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_size_t
    typedef size_t vm_ghost_size_t;
    #define vm_ghost_has_vm_ghost_size_t 1
#endif

/***********************************
 * ghost/header/c/ghost_stdlib_h.h
 ***********************************/

#ifndef vm_ghost_has_vm_ghost_stdlib_h
    #ifdef VM_GHOST_STDLIB_H
        #include VM_GHOST_STDLIB_H
        #define vm_ghost_has_vm_ghost_stdlib_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdlib_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<stdlib.h>)
            #include <stdlib.h>
            #define vm_ghost_has_vm_ghost_stdlib_h 1
        #else
            #define vm_ghost_has_vm_ghost_stdlib_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdlib_h
    #if VM_GHOST_HOSTED
        #include <stdlib.h>
        #define vm_ghost_has_vm_ghost_stdlib_h 1
    #else
        #define vm_ghost_has_vm_ghost_stdlib_h 0
    #endif
#endif

#ifdef __cplusplus
    #if vm_ghost_has_vm_ghost_stdlib_h
        #if vm_ghost_has(vm_ghost_has_include)
            #if __has_include(<cstdlib>)
                #include <cstdlib>
            #endif
        #else
            #include <cstdlib>
        #endif
    #endif
#endif

/*****************************
 * ghost/debug/ghost_abort.h
 *****************************/

#ifndef vm_ghost_has_vm_ghost_abort
    #ifdef vm_ghost_abort
        #define vm_ghost_has_vm_ghost_abort 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_abort
    #if VM_GHOST_HOSTED
        #if vm_ghost_has(vm_ghost_stdlib_h)
            #ifdef __cplusplus
                #define vm_ghost_abort() ::abort()
            #else
                #define vm_ghost_abort() abort()
            #endif
            #define vm_ghost_has_vm_ghost_abort 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_abort
    #if vm_ghost_has(vm_ghost_has_builtin)
        #if __has_builtin(__builtin_trap)
            #define vm_ghost_abort __builtin_trap
            #define vm_ghost_has_vm_ghost_abort 1
        #endif
    #else
        #if VM_GHOST_GCC
            #define vm_ghost_abort __builtin_trap
            #define vm_ghost_has_vm_ghost_abort 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_abort
    #define vm_ghost_has_vm_ghost_abort 0
#endif

/*****************************
 * ghost/debug/ghost_debug.h
 *****************************/

#ifndef vm_ghost_has_VM_GHOST_DEBUG
    #ifdef VM_GHOST_DEBUG
        #define vm_ghost_has_VM_GHOST_DEBUG 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_DEBUG
    #if defined(DEBUG) || defined(_DEBUG)
        #define VM_GHOST_DEBUG 1
    #else
        #define VM_GHOST_DEBUG 0
    #endif
    #define vm_ghost_has_VM_GHOST_DEBUG 1
#endif

/***********************************
 * ghost/header/c/ghost_stdint_h.h
 ***********************************/

#ifndef vm_ghost_has_vm_ghost_stdint_h
    #ifdef VM_GHOST_STDINT_H
        #include VM_GHOST_STDINT_H
        #define vm_ghost_has_vm_ghost_stdint_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdint_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<stdint.h>)
            #include <stdint.h>
            #define vm_ghost_has_vm_ghost_stdint_h 1
        #else
            #define vm_ghost_has_vm_ghost_stdint_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdint_h
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 201103L
            #include <stdint.h>
            #define vm_ghost_has_vm_ghost_stdint_h 1
        #endif
    #elif defined(__STDC_VERSION__)
        #if VM_GHOST_STDC_VERSION >= 199901L
            #include <stdint.h>
            #define vm_ghost_has_vm_ghost_stdint_h 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdint_h
    #ifdef _MSC_VER
        #if _MSC_VER < 1900 
            #define vm_ghost_has_vm_ghost_stdint_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdint_h
    #include <stdint.h>
    #define vm_ghost_has_vm_ghost_stdint_h 1
#endif

#ifdef __cplusplus
    #if vm_ghost_has_vm_ghost_stdint_h
        #if vm_ghost_has(vm_ghost_has_include)
            #if __has_include(<cstdint>)
                #include <cstdint>
            #endif
        #else
            #include <cstdint>
        #endif
    #endif
#endif

/**********************************
 * ghost/header/c/ghost_stdio_h.h
 **********************************/

#ifndef vm_ghost_has_vm_ghost_stdio_h
    #ifdef VM_GHOST_STDIO_H
        #include VM_GHOST_STDIO_H
        #define vm_ghost_has_vm_ghost_stdio_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdio_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<stdio.h>)
            #include <stdio.h>
            #define vm_ghost_has_vm_ghost_stdio_h 1
        #else
            #define vm_ghost_has_vm_ghost_stdio_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_stdio_h
    #if VM_GHOST_HOSTED
        #include <stdio.h>
        #define vm_ghost_has_vm_ghost_stdio_h 1
    #else
        #define vm_ghost_has_vm_ghost_stdio_h 0
    #endif
#endif

#ifdef __cplusplus
    #if vm_ghost_has_vm_ghost_stdio_h
        #if vm_ghost_has(vm_ghost_has_include)
            #if __has_include(<cstdio>)
                #include <cstdio>
            #endif
        #else
            #include <cstdio>
        #endif
    #endif
#endif

/***********************************
 * ghost/header/c/ghost_string_h.h
 ***********************************/

#ifndef vm_ghost_has_vm_ghost_string_h
    #ifdef VM_GHOST_STRING_H
        #include VM_GHOST_STRING_H
        #define vm_ghost_has_vm_ghost_string_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_string_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<string.h>)
            #include <string.h>
            #define vm_ghost_has_vm_ghost_string_h 1
        #else
            #define vm_ghost_has_vm_ghost_string_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_string_h
    #if VM_GHOST_HOSTED
        #include <string.h>
        #define vm_ghost_has_vm_ghost_string_h 1
    #else
        #define vm_ghost_has_vm_ghost_string_h 0
    #endif
#endif

#ifdef __cplusplus
    #if vm_ghost_has_vm_ghost_string_h
        #if vm_ghost_has(vm_ghost_has_include)
            #if __has_include(<cstring>)
                #include <cstring>
            #endif
        #else
            #include <cstring>
        #endif
    #endif
#endif

/**************************************
 * ghost/language/ghost_array_count.h
 **************************************/

#ifndef vm_ghost_has_vm_ghost_array_count
    #ifdef vm_ghost_array_count
        #define vm_ghost_has_vm_ghost_array_count 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_array_count
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 201103L
            template <typename vm_ghost_impl_T, vm_ghost_size_t vm_ghost_impl_N>
            static constexpr vm_ghost_size_t vm_ghost_impl_array_count(vm_ghost_impl_T (&)[vm_ghost_impl_N]) {
                return vm_ghost_impl_N;
            }
            #define vm_ghost_array_count(x) vm_ghost_impl_array_count(x)
        #else
            template <typename vm_ghost_impl_T, vm_ghost_size_t vm_ghost_impl_N>
            char (&vm_ghost_impl_array_count(vm_ghost_impl_T (&vm_ghost_impl_array)[vm_ghost_impl_N]))[vm_ghost_impl_N];
            #define vm_ghost_array_count(x) sizeof(vm_ghost_impl_array_count(x))
        #endif
        #define vm_ghost_has_vm_ghost_array_count 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_array_count
    #ifdef __GNUC__
        #define vm_ghost_array_count(x) \
            ( \
                 \
            (sizeof(x) / sizeof((x)[0])))
        #define vm_ghost_has_vm_ghost_array_count 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_array_count
    #define vm_ghost_array_count(x) \
        ( \
             \
        sizeof(x) / sizeof(0[(x)]))
    #define vm_ghost_has_vm_ghost_array_count 1
#endif

/***********************************
 * ghost/language/ghost_noinline.h
 ***********************************/

#ifndef vm_ghost_has_vm_ghost_noinline
    #ifdef vm_ghost_noinline
        #define vm_ghost_has_vm_ghost_noinline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noinline
    #if defined(_MSC_VER)
        #if defined(__cplusplus) || _MSC_VER >= 1900
            #define vm_ghost_noinline __declspec(noinline)
            #define vm_ghost_has_vm_ghost_noinline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noinline
    #if defined(__GNUC__)
        #define vm_ghost_noinline __attribute__((__noinline__))
        #define vm_ghost_has_vm_ghost_noinline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noinline
    #define vm_ghost_noinline 
    #define vm_ghost_has_vm_ghost_noinline 1
#endif

/***********************************
 * ghost/language/ghost_noreturn.h
 ***********************************/

#ifndef vm_ghost_has_vm_ghost_noreturn
    #ifdef __STDC_VERSION__
        #if (VM_GHOST_STDC_VERSION > 202000L && VM_GHOST_IMPL_ENABLE_C23)
            #define vm_ghost_noreturn [[__noreturn__]]
            #define vm_ghost_has_vm_ghost_noreturn 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noreturn
    #if defined(__STDC_VERSION__) && !defined(__PGI)
        #if VM_GHOST_STDC_VERSION >= 201112L
            #define vm_ghost_noreturn _Noreturn
            #define vm_ghost_has_vm_ghost_noreturn 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noreturn
    #ifdef __cplusplus
        #if VM_GHOST_IMPL_STANDARD_ATTRIBUTE
            #define vm_ghost_noreturn [[noreturn]]
            #define vm_ghost_has_vm_ghost_noreturn 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noreturn
    #if defined(_MSC_VER)
        #define vm_ghost_noreturn __declspec(noreturn)
        #define vm_ghost_has_vm_ghost_noreturn 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noreturn
    #if defined(__GNUC__) || defined(__PGI)
        #define vm_ghost_noreturn __attribute__((__noreturn__))
        #define vm_ghost_has_vm_ghost_noreturn 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noreturn
    #define vm_ghost_noreturn 
    #define vm_ghost_has_vm_ghost_noreturn 1
#endif

/***************************************
 * ghost/language/ghost_noreturn_opt.h
 ***************************************/

#ifndef vm_ghost_has_vm_ghost_noreturn_opt
    #ifndef vm_ghost_noreturn_opt
        #if vm_ghost_has(vm_ghost_noreturn)
            #define vm_ghost_noreturn_opt vm_ghost_noreturn
        #else
            #define vm_ghost_noreturn_opt 
        #endif
    #endif
    #define vm_ghost_has_vm_ghost_noreturn_opt 1
#endif

/*****************************
 * ghost/detect/ghost_unix.h
 *****************************/

#ifndef vm_ghost_has_VM_GHOST_UNIX
    #ifdef VM_GHOST_UNIX
        #define vm_ghost_has_VM_GHOST_UNIX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UNIX
    #if defined(__unix__) || defined (__unix) || \
            (defined(__APPLE__) && defined(__MACH__)) || \
            defined(__hpux) || \
            (defined(__sun) && defined(__SVR4))
        #define VM_GHOST_UNIX 1
    #else
        #define VM_GHOST_UNIX 0
    #endif
    #define vm_ghost_has_VM_GHOST_UNIX 1
#endif

/***************************************
 * ghost/header/posix/ghost_unistd_h.h
 ***************************************/

#ifndef vm_ghost_has_vm_ghost_unistd_h
    #ifdef VM_GHOST_UNISTD_H
        #include VM_GHOST_UNISTD_H
        #define vm_ghost_has_vm_ghost_unistd_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_unistd_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<unistd.h>)
            #include <unistd.h>
            #define vm_ghost_has_vm_ghost_unistd_h 1
        #else
            #define vm_ghost_has_vm_ghost_unistd_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_unistd_h
    #if VM_GHOST_UNIX
        #include <unistd.h>
        #define vm_ghost_has_vm_ghost_unistd_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_unistd_h
    #if defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)
        #include <unistd.h>
        #define vm_ghost_has_vm_ghost_unistd_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_unistd_h
    #define vm_ghost_has_vm_ghost_unistd_h 0
#endif

/************************************
 * ghost/language/ghost_asm_label.h
 ************************************/

#ifndef vm_ghost_has_vm_ghost_asm_label
    #ifdef vm_ghost_asm_label
        #define vm_ghost_has_vm_ghost_asm_label 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_asm_label
    #if defined(__GNUC__) || defined(__TINYC__)
        #define vm_ghost_asm_label(name) __asm__(name)
        #define vm_ghost_has_vm_ghost_asm_label 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_asm_label
    #if VM_GHOST_CPROC
        #define vm_ghost_asm_label(name) __asm__(name)
        #define vm_ghost_has_vm_ghost_asm_label 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_asm_label
    #define vm_ghost_has_vm_ghost_asm_label 0
#endif

/************************************
 * ghost/language/ghost_dllimport.h
 ************************************/

#ifndef vm_ghost_has_vm_ghost_dllimport
    #ifdef vm_ghost_dllimport
        #define vm_ghost_has_vm_ghost_dllimport 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_dllimport
    #if defined(_MSC_VER)
        #define vm_ghost_dllimport __declspec(dllimport)
    #elif defined(__MINGW32__)
        #define vm_ghost_dllimport __attribute__((__visibility__("default"))) __attribute__((__dllimport__))
    #elif defined(__GNUC__)
        #define vm_ghost_dllimport __attribute__((__visibility__("default")))
    #else
        #define vm_ghost_dllimport 
    #endif
    #define vm_ghost_has_vm_ghost_dllimport 1
#endif

/*********************************
 * ghost/language/ghost_expect.h
 *********************************/

#ifndef vm_ghost_has_vm_ghost_expect
    #ifdef vm_ghost_expect
        #define vm_ghost_has_vm_ghost_expect 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_expect
    #if vm_ghost_has(vm_ghost_has_builtin)
        #if __has_builtin(__builtin_expect)
            #define vm_ghost_expect(x, v) (__builtin_expect((x), (v)))
            #define vm_ghost_has_vm_ghost_expect 1
        #endif
    #else
        #if VM_GHOST_GCC
            #define vm_ghost_expect(x, v) (__builtin_expect((x), (v)))
            #define vm_ghost_has_vm_ghost_expect 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_expect
    #define vm_ghost_expect(x, v) (x)
    #define vm_ghost_has_vm_ghost_expect 1
#endif

/**************************************
 * ghost/language/ghost_expect_true.h
 **************************************/

#ifndef vm_ghost_has_vm_ghost_expect_true
    #ifdef vm_ghost_expect_true
        #define vm_ghost_has_vm_ghost_expect_true 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_expect_true
    #define vm_ghost_expect_true(x) (!vm_ghost_expect(!(x), 0))
    #define vm_ghost_has_vm_ghost_expect_true 1
#endif

/***********************************
 * ghost/language/ghost_extern_c.h
 ***********************************/

#ifndef vm_ghost_has_vm_ghost_extern_c
    #ifdef vm_ghost_extern_c
        #define vm_ghost_has_vm_ghost_extern_c 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_extern_c
    #ifdef __cplusplus
        #define vm_ghost_extern_c extern "C"
    #else
        #define vm_ghost_extern_c extern
    #endif
    #define vm_ghost_has_vm_ghost_extern_c 1
#endif

/**********************************
 * ghost/language/ghost_generic.h
 **********************************/

#ifndef vm_ghost_has_vm_ghost_generic
    #if defined(__clang__) && defined(__has_feature)
        #if __has_feature(c_generic_selections)
            #define vm_ghost_generic _Generic
            #define vm_ghost_has_vm_ghost_generic 1
        #else
            #define vm_ghost_has_vm_ghost_generic 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_generic
    #ifdef __STDC_VERSION__
        #if VM_GHOST_STDC_VERSION >= 201112L
            #if VM_GHOST_GCC
                #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)
                    #define vm_ghost_generic _Generic
                    #define vm_ghost_has_vm_ghost_generic 1
                #endif
            #else
                #define vm_ghost_generic _Generic
                #define vm_ghost_has_vm_ghost_generic 1
            #endif
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_generic
    #ifndef __cplusplus
        #if VM_GHOST_GCC
            #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)
                #define vm_ghost_generic __extension__ _Generic
                #define vm_ghost_has_vm_ghost_generic 1
            #endif
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_generic
    #define vm_ghost_has_vm_ghost_generic 0
#endif

/*******************************
 * ghost/language/ghost_weak.h
 *******************************/

#ifndef vm_ghost_has_vm_ghost_weak
    #ifdef vm_ghost_weak
        #define vm_ghost_has_vm_ghost_weak 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_weak
    #ifdef __PCC__
        #define vm_ghost_has_vm_ghost_weak 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_weak
    #ifdef __GXX_WEAK__
        #if !__GXX_WEAK__
            #define vm_ghost_has_vm_ghost_weak 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_weak
    #if defined(__GNUC__) && !defined(__CPARSER__)
        #define vm_ghost_weak __attribute__((__weak__))
        #define vm_ghost_has_vm_ghost_weak 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_weak
    #define vm_ghost_has_vm_ghost_weak 0
#endif

/******************************************
 * ghost/language/ghost_header_noinline.h
 ******************************************/

#ifndef vm_ghost_has_vm_ghost_header_noinline
    #ifdef vm_ghost_header_noinline
        #define vm_ghost_has_vm_ghost_header_noinline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_noinline
    #if (defined(__cplusplus) || defined(_MSC_VER)) && (defined(__clang__) || !defined(__GNUC__))
        #if defined(__cplusplus) || _MSC_VER >= 1900
            #define vm_ghost_header_noinline vm_ghost_noinline inline
            #define vm_ghost_has_vm_ghost_header_noinline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_noinline
    #if vm_ghost_has(vm_ghost_weak)
        #define vm_ghost_header_noinline vm_ghost_weak vm_ghost_noinline
        #define vm_ghost_has_vm_ghost_header_noinline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_noinline
    #define vm_ghost_header_noinline vm_ghost_noinline vm_ghost_maybe_unused static
    #define vm_ghost_has_vm_ghost_header_noinline 1
#endif

/*********************************
 * ghost/language/ghost_typeof.h
 *********************************/

#ifndef vm_ghost_has_vm_ghost_typeof
    #ifdef vm_ghost_typeof
        #define vm_ghost_has_vm_ghost_typeof 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_typeof
    #ifdef __STDC_VERSION__
        #if VM_GHOST_STDC_VERSION > 202000L && VM_GHOST_IMPL_ENABLE_C23
            #define vm_ghost_typeof(T) typeof(T)
            #define vm_ghost_has_vm_ghost_typeof 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_typeof
    #ifdef __GNUC__
        #define vm_ghost_typeof(T) __typeof__(T)
        #define vm_ghost_has_vm_ghost_typeof 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_typeof
    #ifdef __cplusplus
        #if __cplusplus >= 201103L
            #if vm_ghost_has(vm_ghost_cxx_type_traits_h)
                #define vm_ghost_typeof(T) std::remove_reference<decltype(T)>::type
                #define vm_ghost_has_vm_ghost_typeof 1
            #endif
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_typeof
    #define vm_ghost_has_vm_ghost_typeof 0
#endif

/********************************
 * ghost/type/char/ghost_char.h
 ********************************/

#ifdef vm_ghost_has_vm_ghost_char
    #if !vm_ghost_has_vm_ghost_char
        #error "vm_ghost_has_vm_ghost_char cannot be defined to 0. char must exist."
    #endif
#else
    #define vm_ghost_has_vm_ghost_char 1
#endif

/**********************************
 * ghost/type/llong/ghost_llong.h
 **********************************/

#if VM_GHOST_DOCUMENTATION

typedef long long vm_ghost_llong;
#define vm_ghost_has_vm_ghost_llong 1
#endif

#ifndef vm_ghost_has_vm_ghost_llong
    #ifdef vm_ghost_llong
        #define vm_ghost_has_vm_ghost_llong 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_llong
    #if defined(__STDC_VERSION__)
        #if VM_GHOST_STDC_VERSION >= 199901L
            typedef long long vm_ghost_llong;
            #define vm_ghost_has_vm_ghost_llong 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_llong
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 201103L
            typedef long long vm_ghost_llong;
            #define vm_ghost_has_vm_ghost_llong 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_llong
    #ifdef _MSC_VER
        typedef long long vm_ghost_llong;
        #define vm_ghost_has_vm_ghost_llong 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_llong
    #ifdef __GNUC__
        __extension__ typedef long long vm_ghost_llong;
        #define vm_ghost_has_vm_ghost_llong 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_llong
    #define vm_ghost_has_vm_ghost_llong 0
#endif

/************************************
 * ghost/impl/ghost_impl_libc_asm.h
 ************************************/

#ifdef VM_GHOST_IMPL_LIBC_ASM
    #error
#endif
#ifndef vm_ghost_impl_libc_asm
    #if vm_ghost_has(vm_ghost_asm_label)
        #define VM_GHOST_IMPL_LIBC_ASM 1
        #ifdef __APPLE__
            #define vm_ghost_impl_libc_asm(label) vm_ghost_asm_label("_" label)
        #else
            #define vm_ghost_impl_libc_asm(label) vm_ghost_asm_label(label)
        #endif
    #else
        #define VM_GHOST_IMPL_LIBC_ASM 0
    #endif
#endif

/*****************************************
 * ghost/impl/ghost_impl_libc_noexcept.h
 *****************************************/

#ifndef vm_ghost_impl_libc_noexcept
    #if defined __GLIBC__
        #ifdef __cplusplus
            #if vm_ghost_cplusplus >= 201103L
                #define vm_ghost_impl_libc_noexcept(label) noexcept(true) label
            #else
                #define vm_ghost_impl_libc_noexcept(label) label throw()
            #endif
        #else
            #define vm_ghost_impl_libc_noexcept(label) label __attribute__((__nothrow__))
        #endif
    #endif
#endif

#ifndef vm_ghost_impl_libc_noexcept
    #define vm_ghost_impl_libc_noexcept(label) label
#endif

/************************************
 * ghost/impl/ghost_impl_noinline.h
 ************************************/

#if VM_GHOST_STATIC_DEFS
    #define vm_ghost_impl_noinline vm_ghost_maybe_unused vm_ghost_noinline static
#elif VM_GHOST_MANUAL_DEFS
    #define vm_ghost_impl_noinline vm_ghost_noinline
#else
    #define vm_ghost_impl_noinline vm_ghost_header_noinline
#endif

/******************************
 * ghost/io/ghost_flockfile.h
 ******************************/

#ifndef vm_ghost_has_vm_ghost_flockfile
    #ifdef vm_ghost_flockfile
        #define vm_ghost_has_vm_ghost_flockfile 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_flockfile
    #if !vm_ghost_has(vm_ghost_unistd_h)
        #define vm_ghost_has_vm_ghost_flockfile 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_flockfile
    #if !vm_ghost_has(vm_ghost_stdio_h)
        #define vm_ghost_has_vm_ghost_flockfile 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_flockfile
    #if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
            defined(__DragonFly__) || defined(__APPLE__) || defined(__linux__) || \
            defined(__EMSCRIPTEN__)
        #if VM_GHOST_IMPL_LIBC_ASM
            vm_ghost_extern_c
            vm_ghost_dllimport
            void vm_ghost_flockfile(FILE*)
                vm_ghost_impl_libc_noexcept(vm_ghost_impl_libc_asm("flockfile"));
        #else
            VM_GHOST_IMPL_FUNCTION_OPEN
            vm_ghost_impl_always_inline
            void vm_ghost_flockfile(FILE* vm_ghost_file) {
                vm_ghost_dllimport
                extern void flockfile(FILE*)
                    vm_ghost_impl_libc_noexcept(VM_GHOST_NOTHING);
                flockfile(vm_ghost_file);
            }
            VM_GHOST_IMPL_FUNCTION_CLOSE
        #endif
        #define vm_ghost_has_vm_ghost_flockfile 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_flockfile
    #ifdef _POSIX_VERSION
        #if _POSIX_VERSION >= 199506L
            #ifdef __cplusplus
                #define vm_ghost_flockfile ::flockfile
            #else
                #define vm_ghost_flockfile flockfile
            #endif
            #define vm_ghost_has_vm_ghost_flockfile 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_flockfile
    #define vm_ghost_has_vm_ghost_flockfile 0
#endif

/*********************************
 * ghost/io/ghost_ftrylockfile.h
 *********************************/

#ifndef vm_ghost_has_vm_ghost_ftrylockfile
    #ifdef vm_ghost_ftrylockfile
        #define vm_ghost_has_vm_ghost_ftrylockfile 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_ftrylockfile
    #if !vm_ghost_has(vm_ghost_stdio_h)
        #define vm_ghost_has_vm_ghost_ftrylockfile 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_ftrylockfile
    #if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
            defined(__DragonFly__) || defined(__APPLE__) || defined(__linux__) || \
            defined(__EMSCRIPTEN__)
        #if VM_GHOST_IMPL_LIBC_ASM
            vm_ghost_extern_c
            vm_ghost_dllimport
            int vm_ghost_ftrylockfile(FILE*)
                vm_ghost_impl_libc_noexcept(vm_ghost_impl_libc_asm("ftrylockfile"));
        #else
            VM_GHOST_IMPL_FUNCTION_OPEN
            vm_ghost_impl_always_inline
            int vm_ghost_ftrylockfile(FILE* vm_ghost_file) {
                vm_ghost_dllimport
                extern int ftrylockfile(FILE*)
                    vm_ghost_impl_libc_noexcept(VM_GHOST_NOTHING);
                return ftrylockfile(vm_ghost_file);
            }
            VM_GHOST_IMPL_FUNCTION_CLOSE
        #endif
        #define vm_ghost_has_vm_ghost_ftrylockfile 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_ftrylockfile
    #ifdef _POSIX_VERSION
        #if _POSIX_VERSION >= 199506L
            #ifdef __cplusplus
                #define vm_ghost_ftrylockfile ::ftrylockfile
            #else
                #define vm_ghost_ftrylockfile ftrylockfile
            #endif
            #define vm_ghost_has_vm_ghost_ftrylockfile 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_ftrylockfile
    #define vm_ghost_has_vm_ghost_ftrylockfile 0
#endif

/*******************************
 * ghost/language/ghost_null.h
 *******************************/

#ifndef vm_ghost_has_vm_ghost_null
    #ifdef vm_ghost_null
        #define vm_ghost_has_vm_ghost_null 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_null
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 201103L
            #define vm_ghost_null nullptr
        #else
            #define vm_ghost_null NULL
        #endif
    #else
        #define vm_ghost_null ((void*)0)
    #endif
    #define vm_ghost_has_vm_ghost_null 1
#endif

/**********************************
 * ghost/debug/ghost_debugbreak.h
 **********************************/

#ifndef vm_ghost_has_vm_ghost_debugbreak
    #ifdef vm_ghost_debugbreak
        #define vm_ghost_has_vm_ghost_debugbreak 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_debugbreak
    #ifdef _MSC_VER
        #include <intrin.h>
        #define vm_ghost_debugbreak() __debugbreak()
        #define vm_ghost_has_vm_ghost_debugbreak 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_debugbreak
    #if vm_ghost_has(vm_ghost_has_builtin)
        #if __has_builtin(__builtin_debugtrap)
            #define vm_ghost_debugbreak() __builtin_debugtrap()
            #define vm_ghost_has_vm_ghost_debugbreak 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_debugbreak
    #if defined(__GNUC__) && !defined(__chibicc__)

        #ifndef VM_GHOST_DEBUGBREAK_IMPL_ASM
            #if defined(__i386__) || defined(__x86_64__)
                #define VM_GHOST_DEBUGBREAK_IMPL_ASM "int3"
            #elif defined(__aarch64__)
                #define VM_GHOST_DEBUGBREAK_IMPL_ASM ".inst 0xd4200000"
            #elif defined(__thumb__)
                #define VM_GHOST_DEBUGBREAK_IMPL_ASM ".inst 0xde01"
            #elif defined(__arm__)
                #define VM_GHOST_DEBUGBREAK_IMPL_ASM ".inst 0xe7f001f0"
            #endif
        #endif

        #ifdef VM_GHOST_DEBUGBREAK_IMPL_ASM
            VM_GHOST_IMPL_FUNCTION_OPEN
            vm_ghost_impl_always_inline
            void vm_ghost_debugbreak(void) {
                __asm__ __volatile__ (VM_GHOST_DEBUGBREAK_IMPL_ASM);
            }
            VM_GHOST_IMPL_FUNCTION_CLOSE
            #define vm_ghost_has_vm_ghost_debugbreak 1
            #undef VM_GHOST_DEBUGBREAK_IMPL_ASM
        #endif

    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_debugbreak
    #ifdef SIGTRAP
        #ifdef __cplusplus
            #define vm_ghost_debugbreak() ::raise(SIGTRAP)
        #else
            #define vm_ghost_debugbreak() raise(SIGTRAP)
        #endif
        #define vm_ghost_has_vm_ghost_debugbreak 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_debugbreak
    #if vm_ghost_has(vm_ghost_abort)
        #define vm_ghost_debugbreak vm_ghost_abort
        #define vm_ghost_has_vm_ghost_debugbreak 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_debugbreak
    #define vm_ghost_has_vm_ghost_debugbreak 0
#endif

/***********************************
 * ghost/debug/ghost_assert_fail.h
 ***********************************/

#if VM_GHOST_DOCUMENTATION
[[noreturn]]
void vm_ghost_assert_fail(
        const char*  assertion,
        const char*  message,
        const char* file,
        int line,
        const char*  function);
#endif

#ifndef vm_ghost_has_vm_ghost_assert_fail
    #ifdef vm_ghost_assert_fail
        #define vm_ghost_has_vm_ghost_assert_fail 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_assert_fail
    #if vm_ghost_has_vm_ghost_debugbreak || vm_ghost_has_vm_ghost_abort
        #ifdef VM_GHOST_IMPL_ASSERT_FAIL_LINKAGE
            #error
        #endif
        #if vm_ghost_has_vm_ghost_stdio_h
            #define VM_GHOST_IMPL_ASSERT_FAIL_LINKAGE vm_ghost_cold vm_ghost_impl_noinline
        #else
            #define VM_GHOST_IMPL_ASSERT_FAIL_LINKAGE vm_ghost_impl_always_inline
        #endif

        VM_GHOST_IMPL_FUNCTION_OPEN
        vm_ghost_noreturn_opt
        VM_GHOST_IMPL_ASSERT_FAIL_LINKAGE
        void vm_ghost_assert_fail(
                const char*  assertion,
                const char*  message,
                const char* file, int line, const char* function)
        #undef VM_GHOST_IMPL_ASSERT_FAIL_LINKAGE
        #if !VM_GHOST_EMIT_DEFS
        ;
        #else
        {
            #if vm_ghost_has(vm_ghost_stdio_h)
                if (file == vm_ghost_null || file[0] == '\000')
                    file = "(unknown file)";
                if (function == vm_ghost_null || function[0] == '\000')
                    function = "(unknown function)";

                fflush(stdout);

                #if vm_ghost_has(vm_ghost_flockfile)
                    vm_ghost_flockfile(stderr);
                #endif
                #if vm_ghost_has(vm_ghost_ftrylockfile)
                    if (0 == vm_ghost_ftrylockfile(stdout)) {
                        fflush(stdout);
                    }
                #endif

                if (assertion != vm_ghost_null && assertion[0] != '\000') {
                    if (message != vm_ghost_null && message[0] != '\000') {
                        fprintf(stderr, "\nAssertion failed at %s:%d in %s:\n    %s\n    %s\n",
                                file, line, function, assertion, message);
                    } else {
                        fprintf(stderr, "\nAssertion failed at %s:%d in %s:\n    %s\n",
                                file, line, function, assertion);
                    }
                } else {
                    if (message != vm_ghost_null && message[0] != '\000') {
                        fprintf(stderr, "\nFatal error at %s:%d in %s:\n    %s\n",
                                file, line, function, message);
                    } else {
                        fprintf(stderr, "\nFatal error at %s:%d in %s\n",
                                file, line, function);
                    }
                }

            #else
                vm_ghost_discard(assertion);
                vm_ghost_discard(message);
                vm_ghost_discard(file);
                vm_ghost_discard(line);
                vm_ghost_discard(function);
            #endif

            #if VM_GHOST_DEBUG || !vm_ghost_has(vm_ghost_abort)
                vm_ghost_debugbreak();
            #endif

            #if vm_ghost_has(vm_ghost_abort)
                vm_ghost_abort();
            #endif
        }
        #endif
        VM_GHOST_IMPL_FUNCTION_CLOSE
        #define vm_ghost_has_vm_ghost_assert_fail 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_assert_fail
    #define vm_ghost_has_vm_ghost_assert_fail 0
#endif

/***************************************
 * ghost/debug/ghost_pretty_function.h
 ***************************************/

#ifndef vm_ghost_has_VM_GHOST_PRETTY_FUNCTION
    #ifdef VM_GHOST_PRETTY_FUNCTION
        #define vm_ghost_has_VM_GHOST_PRETTY_FUNCTION 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_PRETTY_FUNCTION
    #ifdef __GNUC__
        #ifdef __cplusplus
            #define VM_GHOST_PRETTY_FUNCTION vm_ghost_static_cast(const char*, __PRETTY_FUNCTION__)
            #define vm_ghost_has_VM_GHOST_PRETTY_FUNCTION 1
        #endif
    #elif defined(_MSC_VER)
        #define VM_GHOST_PRETTY_FUNCTION vm_ghost_static_cast(const char*, __FUNCSIG__)
        #define vm_ghost_has_VM_GHOST_PRETTY_FUNCTION 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_PRETTY_FUNCTION
    #ifdef __cpp_lib_source_location
        #if __cpp_lib_source_location >= 201907L
            #include <source_location>
            #define VM_GHOST_PRETTY_FUNCTION (::std::source_location::current().function_name())
            #define vm_ghost_has_VM_GHOST_PRETTY_FUNCTION 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_PRETTY_FUNCTION
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 201103L
            #define VM_GHOST_PRETTY_FUNCTION vm_ghost_static_cast(const char*, __func__)
            #define vm_ghost_has_VM_GHOST_PRETTY_FUNCTION 1
        #endif
    #elif defined(__STDC_VERSION__)
        #if VM_GHOST_STDC_VERSION >= 199901L
            #define VM_GHOST_PRETTY_FUNCTION vm_ghost_static_cast(const char*, __func__)
            #define vm_ghost_has_VM_GHOST_PRETTY_FUNCTION 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_PRETTY_FUNCTION
    #ifdef __CPARSER__
        #define VM_GHOST_PRETTY_FUNCTION vm_ghost_static_cast(const char*, __FUNCTION__)
        #define vm_ghost_has_VM_GHOST_PRETTY_FUNCTION 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_PRETTY_FUNCTION
    #ifdef __GNUC__
        #define VM_GHOST_PRETTY_FUNCTION vm_ghost_static_cast(const char*, __extension__ __func__)
        #define vm_ghost_has_VM_GHOST_PRETTY_FUNCTION 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_PRETTY_FUNCTION
    #define vm_ghost_has_VM_GHOST_PRETTY_FUNCTION 0
#endif

/*****************************
 * ghost/debug/ghost_fatal.h
 *****************************/

#ifndef vm_ghost_has_vm_ghost_fatal
    #ifdef vm_ghost_fatal
        #define vm_ghost_has_vm_ghost_fatal 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_fatal
    #if vm_ghost_has(vm_ghost_assert_fail)
        #ifdef VM_GHOST_IMPL_FATAL_PRETTY_FUNCTION
            #error "Cannot pre-define VM_GHOST_IMPL_FATAL_PRETTY_FUNCTION."
        #endif
        #if vm_ghost_has(VM_GHOST_PRETTY_FUNCTION)
            #define VM_GHOST_IMPL_FATAL_PRETTY_FUNCTION VM_GHOST_PRETTY_FUNCTION
        #else
            #define VM_GHOST_IMPL_FATAL_PRETTY_FUNCTION vm_ghost_null
        #endif

        #if VM_GHOST_PP_VA_ARGS
            #define vm_ghost_fatal(...) vm_ghost_fatal_impl(__VA_ARGS__, "", 0)
            #define vm_ghost_fatal_impl(msg, ...) \
                vm_ghost_assert_fail(vm_ghost_null, "" msg, __FILE__, __LINE__, VM_GHOST_IMPL_FATAL_PRETTY_FUNCTION)
        #else
            #define vm_ghost_fatal(msg) \
                vm_ghost_assert_fail(vm_ghost_null, "" msg, __FILE__, __LINE__, VM_GHOST_IMPL_FATAL_PRETTY_FUNCTION)
        #endif

        #define vm_ghost_has_vm_ghost_fatal 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_fatal
    #define vm_ghost_has_vm_ghost_fatal 0
#endif

/**************************************
 * ghost/language/ghost_unreachable.h
 **************************************/

#ifndef vm_ghost_has_vm_ghost_unreachable
    #ifdef _MSC_VER
        /* Take variadic arguments to avoid "not enough arguments" warning from
         * traditional preprocessor */
        #define vm_ghost_unreachable(...) __assume(0)
        #define vm_ghost_has_vm_ghost_unreachable 1
    #elif defined(__PGI)
        #define vm_ghost_unreachable(dummy) __builtin_unreachable()
        #define vm_ghost_has_vm_ghost_unreachable 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_unreachable
    #ifdef __TINYC__
        #define vm_ghost_unreachable(dummy) \
            do { \
                vm_ghost_fatal("unreachable code path reached"); \
                 \
                return dummy; \
            } while (0)
        #define vm_ghost_has_vm_ghost_unreachable 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_unreachable
    #ifdef __LCC__
        #define vm_ghost_unreachable(dummy) return dummy
        #define vm_ghost_has_vm_ghost_unreachable 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_unreachable
    #ifdef VM_GHOST_DEBUG
        #if vm_ghost_has(vm_ghost_fatal)
            #define vm_ghost_unreachable(dummy) vm_ghost_fatal("unreachable code path reached")
            #define vm_ghost_has_vm_ghost_unreachable 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_unreachable
    #ifdef __STDC_VERSION__
        #if VM_GHOST_STDC_VERSION > 202000L && VM_GHOST_IMPL_ENABLE_C23
            #ifdef unreachable
                #define vm_ghost_unreachable(dummy) unreachable()
                #define vm_ghost_has_vm_ghost_unreachable 1
            #endif
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_unreachable
    #if vm_ghost_has(vm_ghost_has_builtin)
        #if __has_builtin(__builtin_unreachable)
            #define vm_ghost_unreachable(dummy) __builtin_unreachable()
            #define vm_ghost_has_vm_ghost_unreachable 1
        #endif
    #else
        #if VM_GHOST_GCC
            #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
                #define vm_ghost_unreachable(dummy) __builtin_unreachable()
                #define vm_ghost_has_vm_ghost_unreachable 1
            #endif
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_unreachable
    #if vm_ghost_has(vm_ghost_fatal)
        #define vm_ghost_unreachable(dummy) vm_ghost_fatal("unreachable code path reached")
        #define vm_ghost_has_vm_ghost_unreachable 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_unreachable
    #define vm_ghost_unreachable(dummy) return dummy;
    #define vm_ghost_has_vm_ghost_unreachable 1
#endif

/*************************************
 * ghost/debug/ghost_static_assert.h
 *************************************/

#ifndef vm_ghost_has_vm_ghost_static_assert
    #ifdef vm_ghost_static_assert
        #define vm_ghost_has_vm_ghost_static_assert 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_static_assert
    #ifdef __STDC_VERSION__
        #if VM_GHOST_STDC_VERSION > 202000L && VM_GHOST_IMPL_ENABLE_C23
            #define vm_ghost_static_assert _Static_assert
            #define vm_ghost_has_vm_ghost_static_assert 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_static_assert
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 201703L
            #define vm_ghost_static_assert static_assert
            #define vm_ghost_has_vm_ghost_static_assert 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_static_assert
    #ifdef vm_ghost_impl_static_assert_with_msg
        #error "vm_ghost_impl_static_assert_with_msg cannot be pre-defined."
    #endif

    #ifndef vm_ghost_impl_static_assert_with_msg
        #ifdef __STDC_VERSION__
            #if VM_GHOST_STDC_VERSION >= 201112L && !defined(__chibicc__)
                #define vm_ghost_impl_static_assert_with_msg(expr, msg, ...) _Static_assert(expr, msg)
            #endif
        #endif
    #endif

    #ifndef vm_ghost_impl_static_assert_with_msg
        #ifdef __cplusplus
            #if vm_ghost_cplusplus >= 201103L
                #define vm_ghost_impl_static_assert_with_msg(expr, msg, ...) static_assert(expr, msg)
            #endif
        #endif
    #endif

    #ifndef vm_ghost_impl_static_assert_with_msg
        #if defined(_MSC_VER) && defined(__cplusplus)
            #if _MSC_VER >= 1600
                #define vm_ghost_impl_static_assert_with_msg(expr, msg, ...) static_assert(expr, msg)
            #endif
        #endif
    #endif

    #ifdef vm_ghost_impl_static_assert_with_msg
        #if VM_GHOST_MSVC_TRADITIONAL
            #define vm_ghost_static_assert(...) VM_GHOST_EXPAND(vm_ghost_impl_static_assert_with_msg(__VA_ARGS__, "(no message)", 0))
        #else
            #define vm_ghost_static_assert(...) vm_ghost_impl_static_assert_with_msg(__VA_ARGS__, "(no message)", 0)
        #endif
        #define vm_ghost_has_vm_ghost_static_assert 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_static_assert
    #if VM_GHOST_PP_VA_ARGS
        #if VM_GHOST_MSVC_TRADITIONAL
            #define vm_ghost_static_assert(...) VM_GHOST_EXPAND(vm_ghost_static_assert_impl(__VA_ARGS__, 0))
        #else
            #define vm_ghost_static_assert(...) vm_ghost_static_assert_impl(__VA_ARGS__, 0)
        #endif
        #ifdef __CPARSER__
            #define vm_ghost_static_assert_impl(expr, ...) \
                    extern void VM_GHOST_CONCAT(VM_GHOST_STATIC_ASSERT_FAILED_, VM_GHOST_COUNTER)(char[1 - 2*!(expr)])
        #else
            #define vm_ghost_static_assert_impl(expr, ...) \
                    extern void VM_GHOST_STATIC_ASSERT_FAILED(char[1 - 2*!(expr)])
        #endif
    #else
        #define vm_ghost_static_assert(expr, message) \
                extern void VM_GHOST_STATIC_ASSERT_FAILED(char[1 - 2*!(expr)])
    #endif
    #define vm_ghost_has_vm_ghost_static_assert 1

    #ifdef VM_GHOST_IMPL_VM_GHOST_STATIC_ASSERT_IS_SYNTHETIC
        #error "VM_GHOST_IMPL_VM_GHOST_STATIC_ASSERT_IS_SYNTHETIC cannot be pre-defined."
    #endif
    #define VM_GHOST_IMPL_VM_GHOST_STATIC_ASSERT_IS_SYNTHETIC
#endif

/******************************************
 * ghost/debug/ghost_static_assert_expr.h
 ******************************************/

#ifndef vm_ghost_has_vm_ghost_static_assert_expr
    #ifdef vm_ghost_static_assert_expr
        #define vm_ghost_has_vm_ghost_static_assert_expr 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_static_assert_expr
    #ifndef VM_GHOST_IMPL_VM_GHOST_STATIC_ASSERT_IS_SYNTHETIC
        #ifdef __GNUC__
            #define vm_ghost_static_assert_expr(...) ( __extension__ ({ \
                        vm_ghost_static_assert(__VA_ARGS__); \
                        vm_ghost_discard(0); \
                    }))
            #define vm_ghost_has_vm_ghost_static_assert_expr 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_static_assert_expr
    #ifndef VM_GHOST_IMPL_VM_GHOST_STATIC_ASSERT_IS_SYNTHETIC
        #ifdef __cplusplus
            #if vm_ghost_cplusplus >= 201103L
                #define vm_ghost_static_assert_expr(...) ([&]->void{vm_ghost_static_assert(__VA_ARGS__);}())
                #define vm_ghost_has_vm_ghost_static_assert_expr 1
            #endif
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_static_assert_expr
    #if VM_GHOST_PP_VA_ARGS
        #define vm_ghost_static_assert_expr_impl(expr, ...) \
            vm_ghost_discard(sizeof(struct { int vm_ghost_impl_unused: 1; int: -!(expr); }))
        #define vm_ghost_static_assert_expr(...) vm_ghost_static_assert_expr_impl(__VA_ARGS__, 0)
    #else
        #define vm_ghost_static_assert_expr(expr, message) \
            vm_ghost_discard(sizeof(struct { int vm_ghost_impl_unused: 1; int: -!(expr); }))
    #endif
    #define vm_ghost_has_vm_ghost_static_assert_expr 1
#endif

/*******************************
 * ghost/detect/ghost_x86_64.h
 *******************************/

#ifndef vm_ghost_has_VM_GHOST_X86_64
    #ifdef VM_GHOST_X86_64
        #define vm_ghost_has_VM_GHOST_X86_64 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_X86_64
    #if defined(__amd64__) || defined(__amd64) || \
            defined(__x86_64__) || defined(__x86_64) || \
            defined(_M_X64) || defined(_M_AMD64)
        #define VM_GHOST_X86_64 1
    #else
        #define VM_GHOST_X86_64 0
    #endif
    #define vm_ghost_has_VM_GHOST_X86_64 1
#endif

/*******************************
 * ghost/detect/ghost_x86_32.h
 *******************************/

#ifndef vm_ghost_has_VM_GHOST_X86_32
    #ifdef VM_GHOST_X86_32
        #define vm_ghost_has_VM_GHOST_X86_32 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_X86_32
    #if VM_GHOST_X86_64
        #define VM_GHOST_X86_32 0
        #define vm_ghost_has_VM_GHOST_X86_32 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_X86_32
    #if defined(i386) || defined(__i386) || defined(__i386__) || \
            defined(__i486__) || defined(__i586__) || defined(__i686__) || \
            defined(__IA32__) || \
            (defined(_M_IX86) && !defined(_M_I86)) || \
            defined(_M_I386) || \
            defined(__THW_INTEL__) || defined(__INTEL__) || \
            defined(__386) || defined(__386__) || \
            defined(__I86__) || \
            defined(__X86_32__) || defined(_X86_32_)
        #define VM_GHOST_X86_32 1
    #else
        #define VM_GHOST_X86_32 0
    #endif
    #define vm_ghost_has_VM_GHOST_X86_32 1
#endif

/***************************************
 * ghost/impl/ghost_impl_assumptions.h
 ***************************************/

#ifndef VM_GHOST_IMPL_ASSUME_8_BIT_CHAR
    #define VM_GHOST_IMPL_ASSUME_8_BIT_CHAR 1
#endif

#ifndef VM_GHOST_IMPL_ASSUME_TWOS_COMPLEMENT
    #define VM_GHOST_IMPL_ASSUME_TWOS_COMPLEMENT 1
#endif

#ifndef VM_GHOST_IMPL_ASSUME_SIGNED_UNSIGNED_SAME_WIDTH
    #define VM_GHOST_IMPL_ASSUME_SIGNED_UNSIGNED_SAME_WIDTH 1
#endif

#ifndef VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
    #define VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS 1
#endif

/**************************************
 * ghost/type/char/ghost_char_width.h
 **************************************/

#ifndef vm_ghost_has_VM_GHOST_CHAR_WIDTH
    #ifdef VM_GHOST_CHAR_WIDTH
        #define vm_ghost_has_VM_GHOST_CHAR_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_CHAR_WIDTH
    #if !vm_ghost_has(vm_ghost_char)
        #define vm_ghost_has_VM_GHOST_CHAR_WIDTH 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_CHAR_WIDTH
    #ifdef CHAR_BIT
        #define VM_GHOST_CHAR_WIDTH CHAR_BIT
        #define vm_ghost_has_VM_GHOST_CHAR_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_CHAR_WIDTH
    #ifdef __CHAR_BIT__
        #define VM_GHOST_CHAR_WIDTH __CHAR_BIT__
        #define vm_ghost_has_VM_GHOST_CHAR_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_CHAR_WIDTH
    #if !VM_GHOST_IMPL_ASSUME_8_BIT_CHAR
        #error "VM_GHOST_CHAR_WIDTH (a.k.a. CHAR_BIT) is required."
    #endif
    #define VM_GHOST_CHAR_WIDTH 8
    #define vm_ghost_has_VM_GHOST_CHAR_WIDTH 1
#endif

/************************************
 * ghost/type/llong/ghost_llong_c.h
 ************************************/

#ifndef vm_ghost_has_VM_GHOST_LLONG_C
    #ifdef VM_GHOST_LLONG_C
        #define vm_ghost_has_VM_GHOST_LLONG_C 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_C
    #if vm_ghost_has(vm_ghost_llong)
        #define VM_GHOST_LLONG_C(x) x ## LL
        #define vm_ghost_has_VM_GHOST_LLONG_C 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_C
    #define vm_ghost_has_VM_GHOST_LLONG_C 0
#endif

/****************************************
 * ghost/type/llong/ghost_llong_width.h
 ****************************************/

#ifndef vm_ghost_has_VM_GHOST_LLONG_WIDTH
    #ifdef VM_GHOST_LLONG_WIDTH
        #define vm_ghost_has_VM_GHOST_LLONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_WIDTH
    #if !vm_ghost_has(vm_ghost_llong)
        #define vm_ghost_has_VM_GHOST_LLONG_WIDTH 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_WIDTH
    #ifdef LLONG_WIDTH
        #define VM_GHOST_LLONG_WIDTH LLONG_WIDTH
        #define vm_ghost_has_VM_GHOST_LLONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_WIDTH
    #ifdef __LONG_LONG_WIDTH__
        #define VM_GHOST_LLONG_WIDTH __LONG_LONG_WIDTH__
        #define vm_ghost_has_VM_GHOST_LLONG_WIDTH 1
    #elif defined(__SIZEOF_LONG_LONG__)
        #define VM_GHOST_LLONG_WIDTH (__SIZEOF_LONG_LONG__ * VM_GHOST_CHAR_WIDTH)
        #define vm_ghost_has_VM_GHOST_LLONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_WIDTH
    #if defined(_WIN32) || \
            defined(__LP64__) || defined(_LP64) || \
            defined(__ILP32__) || defined(_ILP32) || \
            defined(__x86_64__) || defined(__i386__) || \
            defined(__arm__) || defined(__aarch64__) || \
            defined(__riscv) || defined(__riscv__) || \
            defined(__wasm) || defined(__wasm__)
        #define VM_GHOST_LLONG_WIDTH 64
        #define vm_ghost_has_VM_GHOST_LLONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_WIDTH
    #define vm_ghost_has_VM_GHOST_LLONG_WIDTH 0
#endif

#if vm_ghost_has(VM_GHOST_LLONG_WIDTH)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        vm_ghost_static_assert(sizeof(vm_ghost_llong) * VM_GHOST_CHAR_WIDTH == VM_GHOST_LLONG_WIDTH,
                "VM_GHOST_LLONG_WIDTH is incorrect");
    #endif
#endif

/**************************************
 * ghost/type/llong/ghost_llong_max.h
 **************************************/

#ifndef vm_ghost_has_VM_GHOST_LLONG_MAX
    #ifdef VM_GHOST_LLONG_MAX
        #define vm_ghost_has_VM_GHOST_LLONG_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_MAX
    #if !vm_ghost_has(vm_ghost_llong)
        #define vm_ghost_has_VM_GHOST_LLONG_MAX 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_MAX
    #ifdef LLONG_MAX
        #define VM_GHOST_LLONG_MAX LLONG_MAX
        #define vm_ghost_has_VM_GHOST_LLONG_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_MAX
    #ifdef __LONG_LONG_MAX__
        #define VM_GHOST_LLONG_MAX __LONG_LONG_MAX__
        #define vm_ghost_has_VM_GHOST_LLONG_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_MAX
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        #if vm_ghost_has(VM_GHOST_LLONG_WIDTH)
            #if VM_GHOST_LLONG_WIDTH == 64
                #define VM_GHOST_LLONG_MAX 9223372036854775807LL
            #elif VM_GHOST_LLONG_WIDTH == 32
                #define VM_GHOST_LLONG_MAX 2147483647LL
            #elif VM_GHOST_LLONG_WIDTH == 16
                #define VM_GHOST_LLONG_MAX 32767LL
            #elif VM_GHOST_LLONG_WIDTH == 8
                #define VM_GHOST_LLONG_MAX 127LL
            #else
                #define VM_GHOST_LLONG_MAX ((((1LL << (VM_GHOST_LLONG_WIDTH - 2)) - 1LL) << 1) + 1LL)
            #endif
            #define vm_ghost_has_VM_GHOST_LLONG_MAX 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_MAX
    #define vm_ghost_has_VM_GHOST_LLONG_MAX 0
#endif

#if vm_ghost_has(VM_GHOST_LLONG_MAX)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
            VM_GHOST_SILENCE_PUSH
            VM_GHOST_SILENCE_LONG_LONG
        #endif

        vm_ghost_static_assert(VM_GHOST_LLONG_MAX ==
                    ((((1LL << (sizeof(vm_ghost_llong) * VM_GHOST_CHAR_WIDTH - 2)) - 1LL) << 1) + 1LL),
                "VM_GHOST_LLONG_MAX is incorrect");

        #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
            VM_GHOST_SILENCE_POP
        #endif
    #endif
#endif

/**************************************
 * ghost/type/llong/ghost_llong_min.h
 **************************************/

#ifndef vm_ghost_has_VM_GHOST_LLONG_MIN
    #ifdef VM_GHOST_LLONG_MIN
        #define vm_ghost_has_VM_GHOST_LLONG_MIN 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_MIN
    #if !vm_ghost_has(vm_ghost_llong)
        #define vm_ghost_has_VM_GHOST_LLONG_MIN 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_MIN
    #ifdef LLONG_MIN
        #define VM_GHOST_LLONG_MIN LLONG_MIN
        #define vm_ghost_has_VM_GHOST_LLONG_MIN 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_MIN
    #if VM_GHOST_IMPL_ASSUME_TWOS_COMPLEMENT
        #if vm_ghost_has(VM_GHOST_LLONG_MAX)
            #define VM_GHOST_LLONG_MIN (-1 - VM_GHOST_LLONG_MAX)
            #define vm_ghost_has_VM_GHOST_LLONG_MIN 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LLONG_MIN
    #define vm_ghost_has_VM_GHOST_LLONG_MIN 0
#endif

#if vm_ghost_has(VM_GHOST_LLONG_MIN)
    #if VM_GHOST_IMPL_ASSUME_TWOS_COMPLEMENT
        #if vm_ghost_has(VM_GHOST_LLONG_MAX)
            #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
                VM_GHOST_SILENCE_PUSH
                VM_GHOST_SILENCE_LONG_LONG
            #endif

            vm_ghost_static_assert(VM_GHOST_LLONG_MIN == -1LL - VM_GHOST_LLONG_MAX,
                    "VM_GHOST_LLONG_MIN is incorrect");

            #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
                VM_GHOST_SILENCE_POP
            #endif
        #endif
    #endif
#endif

/******************************
 * ghost/type/int/ghost_int.h
 ******************************/

#ifdef vm_ghost_has_vm_ghost_int
    #if !vm_ghost_has_vm_ghost_int
        #error "vm_ghost_has_vm_ghost_int cannot be defined to 0. int must exist."
    #endif
#else
    #define vm_ghost_has_vm_ghost_int 1
#endif

/************************************
 * ghost/type/int/ghost_int_width.h
 ************************************/

#ifndef vm_ghost_has_VM_GHOST_INT_WIDTH
    #ifdef VM_GHOST_INT_WIDTH
        #define vm_ghost_has_VM_GHOST_INT_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INT_WIDTH
    #if !vm_ghost_has(vm_ghost_int)
        #define vm_ghost_has_VM_GHOST_INT_WIDTH 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INT_WIDTH
    #ifdef INT_WIDTH
        #define VM_GHOST_INT_WIDTH INT_WIDTH
        #define vm_ghost_has_VM_GHOST_INT_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INT_WIDTH
    #ifdef __INT_WIDTH__
        #define VM_GHOST_INT_WIDTH __INT_WIDTH__
        #define vm_ghost_has_VM_GHOST_INT_WIDTH 1
    #elif defined(__SIZEOF_INT__)
        #define VM_GHOST_INT_WIDTH (__SIZEOF_INT__ * VM_GHOST_CHAR_WIDTH)
        #define vm_ghost_has_VM_GHOST_INT_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INT_WIDTH
    #if defined(_WIN32) || \
            defined(__LP64__) || defined(_LP64) || \
            defined(__ILP32__) || defined(_ILP32) || \
            defined(__x86_64__) || defined(__i386__) || \
            defined(__arm__) || defined(__aarch64__) || \
            defined(__riscv) || defined(__riscv__) || \
            defined(__wasm) || defined(__wasm__)
        #define VM_GHOST_INT_WIDTH 32
        #define vm_ghost_has_VM_GHOST_INT_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INT_WIDTH
    #define vm_ghost_has_VM_GHOST_INT_WIDTH 0
#endif

#if vm_ghost_has(VM_GHOST_INT_WIDTH)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        vm_ghost_static_assert(sizeof(int) * VM_GHOST_CHAR_WIDTH == VM_GHOST_INT_WIDTH,
                "VM_GHOST_INT_WIDTH is incorrect");
    #endif
#endif

/********************************
 * ghost/type/uint/ghost_uint.h
 ********************************/

#ifdef vm_ghost_has_vm_ghost_uint
    #if !vm_ghost_has_vm_ghost_uint
        #error "vm_ghost_has_vm_ghost_uint cannot be defined to 0. unsigned int must exist."
    #endif
#else
    #define vm_ghost_has_vm_ghost_uint 1
#endif

/**********************************
 * ghost/type/uint/ghost_uint_c.h
 **********************************/

#ifndef vm_ghost_has_VM_GHOST_UINT_C
    #ifndef VM_GHOST_UINT_C
        #define VM_GHOST_UINT_C(x) x ## U
    #endif
    #define vm_ghost_has_VM_GHOST_UINT_C 1
#endif

/**************************************
 * ghost/type/uint/ghost_uint_width.h
 **************************************/

#ifndef vm_ghost_has_VM_GHOST_UINT_WIDTH
    #ifdef VM_GHOST_UINT_WIDTH
        #define vm_ghost_has_VM_GHOST_UINT_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT_WIDTH
    #if !vm_ghost_has(vm_ghost_uint)
        #define vm_ghost_has_VM_GHOST_UINT_WIDTH 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT_WIDTH
    #ifdef UINT_WIDTH
        #define VM_GHOST_UINT_WIDTH UINT_WIDTH
        #define vm_ghost_has_VM_GHOST_UINT_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT_WIDTH
    #if VM_GHOST_IMPL_ASSUME_SIGNED_UNSIGNED_SAME_WIDTH
        #if vm_ghost_has(VM_GHOST_INT_WIDTH)
            #define VM_GHOST_UINT_WIDTH VM_GHOST_INT_WIDTH
            #define vm_ghost_has_VM_GHOST_UINT_WIDTH 1
        #endif
    #endif
#endif

#if vm_ghost_has(VM_GHOST_UINT_WIDTH)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        vm_ghost_static_assert(sizeof(unsigned int) * VM_GHOST_CHAR_WIDTH == VM_GHOST_UINT_WIDTH,
                "VM_GHOST_UINT_WIDTH is incorrect");
    #endif
#endif

/*********************************************************
 * ghost/silence/ghost_silence_cast_truncates_constant.h
 *********************************************************/

#ifndef vm_ghost_has_VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT
    #ifdef VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT
        #define vm_ghost_has_VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT
    #ifdef _MSC_VER
        #if _MSC_VER < 1920 && defined(__cplusplus)
            #define VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT \
                    __pragma(warning(disable:4309)) \
                    __pragma(warning(disable:4310))
        #else
            #define VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT \
                    __pragma(warning(disable:4310))
        #endif
        #define vm_ghost_has_VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT
    #define VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT 
    #define vm_ghost_has_VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT 1
#endif

/***************************************************
 * ghost/silence/ghost_silence_constant_overflow.h
 ***************************************************/

#ifndef vm_ghost_has_VM_GHOST_SILENCE_CONSTANT_OVERFLOW
    #ifdef VM_GHOST_SILENCE_CONSTANT_OVERFLOW
        #define vm_ghost_has_VM_GHOST_SILENCE_CONSTANT_OVERFLOW 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_CONSTANT_OVERFLOW
    #ifdef _MSC_VER
        #define VM_GHOST_SILENCE_CONSTANT_OVERFLOW __pragma(warning(disable:4307))
        #define vm_ghost_has_VM_GHOST_SILENCE_CONSTANT_OVERFLOW 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_CONSTANT_OVERFLOW
    #if defined(__GNUC__) && !defined(__CPARSER__)
        #if VM_GHOST_GCC
            #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
                #define VM_GHOST_SILENCE_CONSTANT_OVERFLOW _Pragma("GCC diagnostic ignored \"-Woverflow\"")
                #define vm_ghost_has_VM_GHOST_SILENCE_CONSTANT_OVERFLOW 1
            #endif
        #else
            #define VM_GHOST_SILENCE_CONSTANT_OVERFLOW _Pragma("GCC diagnostic ignored \"-Woverflow\"")
            #define vm_ghost_has_VM_GHOST_SILENCE_CONSTANT_OVERFLOW 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SILENCE_CONSTANT_OVERFLOW
    #define VM_GHOST_SILENCE_CONSTANT_OVERFLOW 
    #define vm_ghost_has_VM_GHOST_SILENCE_CONSTANT_OVERFLOW 1
#endif

/************************************
 * ghost/type/uint/ghost_uint_max.h
 ************************************/

#ifndef vm_ghost_has_VM_GHOST_UINT_MAX
    #ifdef VM_GHOST_UINT_MAX
        #define vm_ghost_has_VM_GHOST_UINT_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT_MAX
    #if !vm_ghost_has(vm_ghost_uint)
        #define vm_ghost_has_VM_GHOST_UINT_MAX 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT_MAX
    #ifdef UINT_MAX
        #define VM_GHOST_UINT_MAX UINT_MAX
        #define vm_ghost_has_VM_GHOST_UINT_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT_MAX
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        #if vm_ghost_has(VM_GHOST_UINT_WIDTH)
            #if VM_GHOST_UINT_WIDTH == 64
                #define VM_GHOST_UINT_MAX 18446744073709551615U
            #elif VM_GHOST_UINT_WIDTH == 32
                #define VM_GHOST_UINT_MAX 4294967295U
            #elif VM_GHOST_UINT_WIDTH == 16
                #define VM_GHOST_UINT_MAX 65535U
            #elif VM_GHOST_UINT_WIDTH == 8
                #define VM_GHOST_UINT_MAX 255U
            #else
                #define VM_GHOST_UINT_MAX ((((1U << (VM_GHOST_UINT_WIDTH - 1)) - 1U) << 1) + 1U)
            #endif
            #define vm_ghost_has_VM_GHOST_UINT_MAX 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT_MAX
    #define vm_ghost_has_VM_GHOST_UINT_MAX 0
#endif

#if vm_ghost_has(VM_GHOST_UINT_MAX)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        vm_ghost_static_assert(VM_GHOST_UINT_MAX ==
                    ((((1U << (sizeof(unsigned int) * VM_GHOST_CHAR_WIDTH - 1)) - 1U) << 1) + 1U),
                "VM_GHOST_UINT_MAX is incorrect");
    #endif
#endif

#if vm_ghost_has(VM_GHOST_UINT_MAX)
    #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
        VM_GHOST_SILENCE_PUSH
        VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT
        VM_GHOST_SILENCE_CONSTANT_OVERFLOW
    #endif

    vm_ghost_static_assert(0 == vm_ghost_static_cast(unsigned int, VM_GHOST_UINT_MAX + 1U),
            "VM_GHOST_UINT_MAX is incorrect");

    #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
        VM_GHOST_SILENCE_POP
    #endif
#endif

/********************************
 * ghost/type/long/ghost_long.h
 ********************************/

#ifndef vm_ghost_has_vm_ghost_long
    #define vm_ghost_has_vm_ghost_long 1
#endif

/**********************************
 * ghost/type/long/ghost_long_c.h
 **********************************/

#ifndef vm_ghost_has_VM_GHOST_LONG_C
    #ifndef VM_GHOST_LONG_C
        #define VM_GHOST_LONG_C(x) x ## L
    #endif
    #define vm_ghost_has_VM_GHOST_LONG_C 1
#endif

/**************************************
 * ghost/type/long/ghost_long_width.h
 **************************************/

#ifndef vm_ghost_has_VM_GHOST_LONG_WIDTH
    #ifdef VM_GHOST_LONG_WIDTH
        #define vm_ghost_has_VM_GHOST_LONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_WIDTH
    #if !vm_ghost_has(vm_ghost_long)
        #define vm_ghost_has_VM_GHOST_LONG_WIDTH 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_WIDTH
    #ifdef LONG_WIDTH
        #define VM_GHOST_LONG_WIDTH LONG_WIDTH
        #define vm_ghost_has_VM_GHOST_LONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_WIDTH
    #ifdef __LONG_WIDTH__
        #define VM_GHOST_LONG_WIDTH __LONG_WIDTH__
        #define vm_ghost_has_VM_GHOST_LONG_WIDTH 1
    #elif defined(__SIZEOF_LONG__)
        #define VM_GHOST_LONG_WIDTH (__SIZEOF_LONG__ * VM_GHOST_CHAR_WIDTH)
        #define vm_ghost_has_VM_GHOST_LONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_WIDTH
    #if defined(__ILP32__) || defined(_ILP32)
        #define VM_GHOST_LONG_WIDTH 32
        #define vm_ghost_has_VM_GHOST_LONG_WIDTH 1
    #elif defined(__LP64__) || defined(_LP64)
        #define VM_GHOST_LONG_WIDTH 64
        #define vm_ghost_has_VM_GHOST_LONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_WIDTH
    #if defined(_WIN32)
        #define VM_GHOST_LONG_WIDTH 32
        #define vm_ghost_has_VM_GHOST_LONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_WIDTH
    #ifdef __riscv_xlen
        #if __riscv_xlen == 64
            #define VM_GHOST_LONG_WIDTH 64
            #define vm_ghost_has_VM_GHOST_LONG_WIDTH 1
        #elif __riscv_xlen == 32
            #define VM_GHOST_LONG_WIDTH 32
            #define vm_ghost_has_VM_GHOST_LONG_WIDTH 1
        #endif
    #elif \
            (defined(__i386__) && !defined(__x86_64__)) || \
            (defined(__arm__) && !defined(__aarch64__))
        #define VM_GHOST_LONG_WIDTH 32
        #define vm_ghost_has_VM_GHOST_LONG_WIDTH 1
    #elif \
            defined(__x86_64__) || \
            defined(__aarch64__)
        #define VM_GHOST_LONG_WIDTH 64
        #define vm_ghost_has_VM_GHOST_LONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_WIDTH
    #define vm_ghost_has_VM_GHOST_LONG_WIDTH 0
#endif

#if vm_ghost_has(VM_GHOST_LONG_WIDTH)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        vm_ghost_static_assert(sizeof(long) * VM_GHOST_CHAR_WIDTH == VM_GHOST_LONG_WIDTH,
                "VM_GHOST_LONG_WIDTH is incorrect");
    #endif
#endif

/************************************
 * ghost/type/long/ghost_long_max.h
 ************************************/

#ifndef vm_ghost_has_VM_GHOST_LONG_MAX
    #ifdef VM_GHOST_LONG_MAX
        #define vm_ghost_has_VM_GHOST_LONG_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_MAX
    #if !vm_ghost_has(vm_ghost_long)
        #define vm_ghost_has_VM_GHOST_LONG_MAX 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_MAX
    #ifdef LONG_MAX
        #define VM_GHOST_LONG_MAX LONG_MAX
        #define vm_ghost_has_VM_GHOST_LONG_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_MAX
    #ifdef __LONG_MAX__
        #define VM_GHOST_LONG_MAX __LONG_MAX__
        #define vm_ghost_has_VM_GHOST_LONG_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_MAX
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        #if vm_ghost_has(VM_GHOST_LONG_WIDTH)
            #if VM_GHOST_LONG_WIDTH == 64
                #define VM_GHOST_LONG_MAX 9223372036854775807L
            #elif VM_GHOST_LONG_WIDTH == 32
                #define VM_GHOST_LONG_MAX 2147483647L
            #elif VM_GHOST_LONG_WIDTH == 16
                #define VM_GHOST_LONG_MAX 32767L
            #elif VM_GHOST_LONG_WIDTH == 8
                #define VM_GHOST_LONG_MAX 127L
            #else
                #define VM_GHOST_LONG_MAX ((((1L << (VM_GHOST_LONG_WIDTH - 2)) - 1L) << 1) + 1L)
            #endif
            #define vm_ghost_has_VM_GHOST_LONG_MAX 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_MAX
    #define vm_ghost_has_VM_GHOST_LONG_MAX 0
#endif

#if vm_ghost_has(VM_GHOST_LONG_MAX)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        vm_ghost_static_assert(VM_GHOST_LONG_MAX ==
                    ((((1L << (sizeof(long) * VM_GHOST_CHAR_WIDTH - 2)) - 1L) << 1) + 1L),
                "VM_GHOST_LONG_MAX is incorrect");
    #endif
#endif

/************************************
 * ghost/type/long/ghost_long_min.h
 ************************************/

#ifndef vm_ghost_has_VM_GHOST_LONG_MIN
    #ifdef VM_GHOST_LONG_MIN
        #define vm_ghost_has_VM_GHOST_LONG_MIN 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_MIN
    #if !vm_ghost_has(vm_ghost_long)
        #define vm_ghost_has_VM_GHOST_LONG_MIN 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_MIN
    #ifdef LONG_MIN
        #define VM_GHOST_LONG_MIN LONG_MIN
        #define vm_ghost_has_VM_GHOST_LONG_MIN 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_MIN
    #if VM_GHOST_IMPL_ASSUME_TWOS_COMPLEMENT
        #if vm_ghost_has(VM_GHOST_LONG_MAX)
            #define VM_GHOST_LONG_MIN (-1 - VM_GHOST_LONG_MAX)
            #define vm_ghost_has_VM_GHOST_LONG_MIN 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_LONG_MIN
    #define vm_ghost_has_VM_GHOST_LONG_MIN 0
#endif

#if vm_ghost_has(VM_GHOST_LONG_MIN)
    #if VM_GHOST_IMPL_ASSUME_TWOS_COMPLEMENT
        #if vm_ghost_has(VM_GHOST_LONG_MAX)
            vm_ghost_static_assert(VM_GHOST_LONG_MIN == -1 - VM_GHOST_LONG_MAX,
                    "VM_GHOST_LONG_MIN is incorrect");
        #endif
    #endif
#endif

/**********************************
 * ghost/type/ulong/ghost_ulong.h
 **********************************/

#ifndef vm_ghost_has_vm_ghost_ulong
    #define vm_ghost_has_vm_ghost_ulong 1
#endif

/************************************
 * ghost/type/ulong/ghost_ulong_c.h
 ************************************/

#ifndef vm_ghost_has_VM_GHOST_ULONG_C
    #ifndef VM_GHOST_ULONG_C
        #define VM_GHOST_ULONG_C(x) x ## UL
    #endif
    #define vm_ghost_has_VM_GHOST_ULONG_C 1
#endif

/****************************************
 * ghost/type/ulong/ghost_ulong_width.h
 ****************************************/

#ifndef vm_ghost_has_VM_GHOST_ULONG_WIDTH
    #ifdef VM_GHOST_ULONG_WIDTH
        #define vm_ghost_has_VM_GHOST_ULONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULONG_WIDTH
    #if !vm_ghost_has(vm_ghost_long)
        #define vm_ghost_has_VM_GHOST_ULONG_WIDTH 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULONG_WIDTH
    #ifdef ULONG_WIDTH
        #define VM_GHOST_ULONG_WIDTH ULONG_WIDTH
        #define vm_ghost_has_VM_GHOST_ULONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULONG_WIDTH
    #if VM_GHOST_IMPL_ASSUME_SIGNED_UNSIGNED_SAME_WIDTH
        #if vm_ghost_has(VM_GHOST_LONG_WIDTH)
            #define VM_GHOST_ULONG_WIDTH VM_GHOST_LONG_WIDTH
            #define vm_ghost_has_VM_GHOST_ULONG_WIDTH 1
        #endif
    #endif
#endif

#if vm_ghost_has(VM_GHOST_ULONG_WIDTH)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        vm_ghost_static_assert(sizeof(unsigned long) * VM_GHOST_CHAR_WIDTH == VM_GHOST_ULONG_WIDTH,
                "VM_GHOST_ULONG_WIDTH is incorrect");
    #endif
#endif

/**************************************
 * ghost/type/ulong/ghost_ulong_max.h
 **************************************/

#ifndef vm_ghost_has_VM_GHOST_ULONG_MAX
    #ifdef VM_GHOST_ULONG_MAX
        #define vm_ghost_has_VM_GHOST_ULONG_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULONG_MAX
    #if !vm_ghost_has(vm_ghost_ulong)
        #define vm_ghost_has_VM_GHOST_ULONG_MAX 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULONG_MAX
    #ifdef ULONG_MAX
        #define VM_GHOST_ULONG_MAX ULONG_MAX
        #define vm_ghost_has_VM_GHOST_ULONG_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULONG_MAX
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        #if vm_ghost_has(VM_GHOST_ULONG_WIDTH)
            #if VM_GHOST_ULONG_WIDTH == 64
                #define VM_GHOST_ULONG_MAX 18446744073709551615UL
            #elif VM_GHOST_ULONG_WIDTH == 32
                #define VM_GHOST_ULONG_MAX 4294967295UL
            #elif VM_GHOST_ULONG_WIDTH == 16
                #define VM_GHOST_ULONG_MAX 65535UL
            #elif VM_GHOST_ULONG_WIDTH == 8
                #define VM_GHOST_ULONG_MAX 255UL
            #else
                #define VM_GHOST_ULONG_MAX ((((1UL << (VM_GHOST_ULONG_WIDTH - 1)) - 1UL) << 1) + 1UL)
            #endif
            #define vm_ghost_has_VM_GHOST_ULONG_MAX 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULONG_MAX
    #define vm_ghost_has_VM_GHOST_ULONG_MAX 0
#endif

#if vm_ghost_has(VM_GHOST_ULONG_MAX)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        vm_ghost_static_assert(VM_GHOST_ULONG_MAX ==
                    ((((1UL << (sizeof(unsigned long) * VM_GHOST_CHAR_WIDTH - 1)) - 1UL) << 1) + 1UL),
                "VM_GHOST_ULONG_MAX is incorrect");
    #endif
#endif

#if vm_ghost_has(VM_GHOST_ULONG_MAX)
    #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
        VM_GHOST_SILENCE_PUSH
        VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT
        VM_GHOST_SILENCE_CONSTANT_OVERFLOW
    #endif

    vm_ghost_static_assert(0 == vm_ghost_static_cast(unsigned long, VM_GHOST_ULONG_MAX + 1UL),
            "VM_GHOST_ULONG_MAX is incorrect");

    #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
        VM_GHOST_SILENCE_POP
    #endif
#endif

/************************************
 * ghost/type/ullong/ghost_ullong.h
 ************************************/

#if VM_GHOST_DOCUMENTATION

typedef long long vm_ghost_ullong;
#define vm_ghost_has_vm_ghost_ullong 1
#endif

#ifndef vm_ghost_has_vm_ghost_ullong
    #ifdef vm_ghost_ullong
        #define vm_ghost_has_vm_ghost_ullong 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_ullong
    #if defined(__STDC_VERSION__)
        #if VM_GHOST_STDC_VERSION >= 199901L
            typedef unsigned long long vm_ghost_ullong;
            #define vm_ghost_has_vm_ghost_ullong 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_ullong
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 201103L
            typedef unsigned long long vm_ghost_ullong;
            #define vm_ghost_has_vm_ghost_ullong 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_ullong
    #ifdef _MSC_VER
        typedef unsigned long long vm_ghost_ullong;
        #define vm_ghost_has_vm_ghost_ullong 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_ullong
    #ifdef __GNUC__
        __extension__ typedef unsigned long long vm_ghost_ullong;
        #define vm_ghost_has_vm_ghost_ullong 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_ullong
    #define vm_ghost_has_vm_ghost_ullong 0
#endif

/**************************************
 * ghost/type/ullong/ghost_ullong_c.h
 **************************************/

#ifndef vm_ghost_has_VM_GHOST_ULLONG_C
    #ifdef VM_GHOST_ULLONG_C
        #define vm_ghost_has_VM_GHOST_ULLONG_C 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULLONG_C
    #if vm_ghost_has(vm_ghost_ullong)
        #define VM_GHOST_ULLONG_C(x) x ## ULL
        #define vm_ghost_has_VM_GHOST_ULLONG_C 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULLONG_C
    #define vm_ghost_has_VM_GHOST_ULLONG_C 0
#endif

/******************************************
 * ghost/type/ullong/ghost_ullong_width.h
 ******************************************/

#ifndef vm_ghost_has_VM_GHOST_ULLONG_WIDTH
    #ifdef VM_GHOST_ULLONG_WIDTH
        #define vm_ghost_has_VM_GHOST_ULLONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULLONG_WIDTH
    #if !vm_ghost_has(vm_ghost_ullong)
        #define vm_ghost_has_VM_GHOST_ULLONG_WIDTH 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULLONG_WIDTH
    #ifdef ULLONG_WIDTH
        #define VM_GHOST_ULLONG_WIDTH ULLONG_WIDTH
        #define vm_ghost_has_VM_GHOST_ULLONG_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULLONG_WIDTH
    #if VM_GHOST_IMPL_ASSUME_SIGNED_UNSIGNED_SAME_WIDTH
        #if vm_ghost_has(VM_GHOST_LLONG_WIDTH)
            #define VM_GHOST_ULLONG_WIDTH VM_GHOST_LLONG_WIDTH
            #define vm_ghost_has_VM_GHOST_ULLONG_WIDTH 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULLONG_WIDTH
    #define vm_ghost_has_VM_GHOST_ULLONG_WIDTH 0
#endif

#if vm_ghost_has(VM_GHOST_ULLONG_WIDTH)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        vm_ghost_static_assert(sizeof(vm_ghost_ullong) * VM_GHOST_CHAR_WIDTH == VM_GHOST_ULLONG_WIDTH,
                "VM_GHOST_ULLONG_WIDTH is incorrect");
    #endif
#endif

/****************************************
 * ghost/type/ullong/ghost_ullong_max.h
 ****************************************/

#ifndef vm_ghost_has_VM_GHOST_ULLONG_MAX
    #ifdef VM_GHOST_ULLONG_MAX
        #define vm_ghost_has_VM_GHOST_ULLONG_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULLONG_MAX
    #if !vm_ghost_has(vm_ghost_ullong)
        #define vm_ghost_has_VM_GHOST_ULLONG_MAX 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULLONG_MAX
    #ifdef ULLONG_MAX
        #define VM_GHOST_ULLONG_MAX ULLONG_MAX
        #define vm_ghost_has_VM_GHOST_ULLONG_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULLONG_MAX
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        #if vm_ghost_has(VM_GHOST_ULLONG_WIDTH)
            #if VM_GHOST_ULLONG_WIDTH == 64
                #define VM_GHOST_ULLONG_MAX 18446744073709551615ULL
            #elif VM_GHOST_ULLONG_WIDTH == 32
                #define VM_GHOST_ULLONG_MAX 4294967295ULL
            #elif VM_GHOST_ULLONG_WIDTH == 16
                #define VM_GHOST_ULLONG_MAX 65535ULL
            #elif VM_GHOST_ULLONG_WIDTH == 8
                #define VM_GHOST_ULLONG_MAX 255ULL
            #else
                #define VM_GHOST_ULLONG_MAX ((((1ULL << (VM_GHOST_ULLONG_WIDTH - 1)) - 1ULL) << 1) + 1ULL)
            #endif
            #define vm_ghost_has_VM_GHOST_ULLONG_MAX 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_ULLONG_MAX
    #define vm_ghost_has_VM_GHOST_ULLONG_MAX 0
#endif

#if vm_ghost_has(VM_GHOST_ULLONG_MAX)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
            VM_GHOST_SILENCE_PUSH
            VM_GHOST_SILENCE_LONG_LONG
        #endif

        vm_ghost_static_assert(VM_GHOST_ULLONG_MAX ==
                    ((((1ULL << (sizeof(vm_ghost_ullong) * VM_GHOST_CHAR_WIDTH - 1)) - 1ULL) << 1) + 1ULL),
                "VM_GHOST_ULLONG_MAX is incorrect");

        #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
            VM_GHOST_SILENCE_POP
        #endif
    #endif
#endif

#if vm_ghost_has(VM_GHOST_ULLONG_MAX)
    #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
        VM_GHOST_SILENCE_PUSH
        VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT
        VM_GHOST_SILENCE_CONSTANT_OVERFLOW
        VM_GHOST_SILENCE_LONG_LONG
    #endif

    vm_ghost_static_assert(0 == vm_ghost_static_cast(vm_ghost_ullong, VM_GHOST_ULLONG_MAX + 1ULL),
            "VM_GHOST_ULLONG_MAX is incorrect");

    #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
        VM_GHOST_SILENCE_POP
    #endif
#endif

/***********************************************
 * ghost/impl/type/ghost_impl_intptr_uintptr.h
 ***********************************************/

#ifndef vm_ghost_has_vm_ghost_intptr_t
    #ifdef vm_ghost_intptr_t
        #define vm_ghost_has_vm_ghost_intptr_t 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uintptr_t
    #ifdef vm_ghost_uintptr_t
        #define vm_ghost_has_vm_ghost_uintptr_t 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INTPTR_WIDTH
    #ifdef VM_GHOST_INTPTR_WIDTH
        #define vm_ghost_has_VM_GHOST_INTPTR_WIDTH 1
    #elif defined(vm_ghost_has_vm_ghost_intptr_t)
        #if !vm_ghost_has_vm_ghost_intptr_t
            #define vm_ghost_has_VM_GHOST_INTPTR_WIDTH 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INTPTR_MAX
    #ifdef VM_GHOST_INTPTR_MAX
        #define vm_ghost_has_VM_GHOST_INTPTR_MAX 1
    #elif defined(vm_ghost_has_vm_ghost_intptr_t)
        #if !vm_ghost_has_vm_ghost_intptr_t
            #define vm_ghost_has_VM_GHOST_INTPTR_MAX 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INTPTR_MIN
    #ifdef VM_GHOST_INTPTR_MIN
        #define vm_ghost_has_VM_GHOST_INTPTR_MIN 1
    #elif defined(vm_ghost_has_vm_ghost_intptr_t)
        #if !vm_ghost_has_vm_ghost_intptr_t
            #define vm_ghost_has_VM_GHOST_INTPTR_MIN 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINTPTR_WIDTH
    #ifdef VM_GHOST_UINTPTR_WIDTH
        #define vm_ghost_has_VM_GHOST_UINTPTR_WIDTH 1
    #elif defined(vm_ghost_has_vm_ghost_uintptr_t)
        #if !vm_ghost_has_vm_ghost_uintptr_t
            #define vm_ghost_has_VM_GHOST_UINTPTR_WIDTH 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINTPTR_MAX
    #ifdef VM_GHOST_UINTPTR_MAX
        #define vm_ghost_has_VM_GHOST_UINTPTR_MAX 1
    #elif defined(vm_ghost_has_vm_ghost_uintptr_t)
        #if !vm_ghost_has_vm_ghost_uintptr_t
            #define vm_ghost_has_VM_GHOST_UINTPTR_MAX 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_intptr_t
    #ifdef INTPTR_MAX
        typedef intptr_t vm_ghost_intptr_t;
        #define vm_ghost_has_vm_ghost_intptr_t 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_uintptr_t
    #ifdef UINTPTR_MAX
        typedef uintptr_t vm_ghost_uintptr_t;
        #define vm_ghost_has_vm_ghost_uintptr_t 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INTPTR_WIDTH
    #ifdef INTPTR_WIDTH
        #define VM_GHOST_INTPTR_WIDTH INTPTR_WIDTH
        #define vm_ghost_has_VM_GHOST_INTPTR_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INTPTR_MAX
    #ifdef INTPTR_MAX
        #define VM_GHOST_INTPTR_MAX INTPTR_MAX
        #define vm_ghost_has_VM_GHOST_INTPTR_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INTPTR_MIN
    #ifdef INTPTR_MIN
        #define VM_GHOST_INTPTR_MIN INTPTR_MIN
        #define vm_ghost_has_VM_GHOST_INTPTR_MIN 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINTPTR_WIDTH
    #ifdef UINTPTR_WIDTH
        #define VM_GHOST_UINTPTR_WIDTH UINTPTR_WIDTH
        #define vm_ghost_has_VM_GHOST_UINTPTR_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINTPTR_MAX
    #ifdef UINTPTR_MAX
        #define VM_GHOST_UINTPTR_MAX UINTPTR_MAX
        #define vm_ghost_has_VM_GHOST_UINTPTR_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_intptr_t
    #ifdef __INTPTR_TYPE__
        typedef __INTPTR_TYPE__ vm_ghost_intptr_t;
        #define vm_ghost_has_vm_ghost_intptr_t 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uintptr_t
    #ifdef __UINTPTR_TYPE__
        typedef __UINTPTR_TYPE__ vm_ghost_uintptr_t;
        #define vm_ghost_has_vm_ghost_uintptr_t 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INTPTR_WIDTH
    #ifdef __INTPTR_WIDTH__
        #define VM_GHOST_INTPTR_WIDTH __INTPTR_WIDTH__
        #define vm_ghost_has_VM_GHOST_INTPTR_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINTPTR_WIDTH
    #ifdef __INTPTR_WIDTH__
        #define VM_GHOST_UINTPTR_WIDTH __INTPTR_WIDTH__
        #define vm_ghost_has_VM_GHOST_UINTPTR_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INTPTR_MAX
    #ifdef __INTPTR_MAX__
        #define VM_GHOST_INTPTR_MAX __INTPTR_MAX__
        #define vm_ghost_has_VM_GHOST_INTPTR_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINTPTR_MAX
    #ifdef __UINTPTR_MAX__
        #define VM_GHOST_UINTPTR_MAX __UINTPTR_MAX__
        #define vm_ghost_has_VM_GHOST_UINTPTR_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_INTPTR_MIN
    #ifdef vm_ghost_has_VM_GHOST_INTPTR_MAX
        #if vm_ghost_has_VM_GHOST_INTPTR_MAX
            #if VM_GHOST_IMPL_ASSUME_TWOS_COMPLEMENT
                #define VM_GHOST_INTPTR_MIN (-1 - VM_GHOST_INTPTR_MAX)
                #define vm_ghost_has_VM_GHOST_INTPTR_MIN 1
            #endif
        #endif
    #endif
#endif

#if \
        !defined(vm_ghost_has_vm_ghost_intptr_t) || \
        !defined(vm_ghost_has_vm_ghost_uintptr_t) || \
        !defined(vm_ghost_has_VM_GHOST_INTPTR_WIDTH) || \
        !defined(vm_ghost_has_VM_GHOST_UINTPTR_WIDTH) || \
        !defined(vm_ghost_has_VM_GHOST_INTPTR_MAX) || \
        !defined(vm_ghost_has_VM_GHOST_INTPTR_MIN) || \
        !defined(vm_ghost_has_VM_GHOST_UINTPTR_MAX) || \
        !defined(vm_ghost_has_VM_GHOST_INTPTR_C) || \
        !defined(vm_ghost_has_VM_GHOST_UINTPTR_C)

    #ifdef VM_GHOST_IMPL_INTPTR_IS_INT
        #error
    #endif
    #ifdef VM_GHOST_IMPL_INTPTR_IS_LONG
        #error
    #endif
    #ifdef VM_GHOST_IMPL_INTPTR_IS_LLONG
        #error
    #endif

    #if defined(_WIN64)
        #define VM_GHOST_IMPL_INTPTR_IS_LLONG
    #elif defined(__LP64__) || defined(_LP64) || \
            defined(__ILP32__) || defined(_ILP32) || \
            defined(_WIN32)
        #define VM_GHOST_IMPL_INTPTR_IS_LONG
    #endif

    #if !defined(VM_GHOST_IMPL_INTPTR_IS_INT) && \
            !defined(VM_GHOST_IMPL_INTPTR_IS_LONG) && \
            !defined(VM_GHOST_IMPL_INTPTR_IS_LLONG)
        #if VM_GHOST_X86_32 || VM_GHOST_X86_64 || \
                defined(__arm__) || defined(__aarch64__) || \
                defined(__riscv) || defined(__riscv__)
            #define VM_GHOST_IMPL_INTPTR_IS_LONG
        #endif
    #endif

    #if !defined(VM_GHOST_IMPL_INTPTR_IS_INT) && \
            !defined(VM_GHOST_IMPL_INTPTR_IS_LONG) && \
            !defined(VM_GHOST_IMPL_INTPTR_IS_LLONG)
        #if vm_ghost_has(vm_ghost_llong)
            #define VM_GHOST_IMPL_INTPTR_IS_LLONG
        #else
            #define VM_GHOST_IMPL_INTPTR_IS_LONG
        #endif
    #endif

    #ifdef VM_GHOST_IMPL_INTPTR_IS_INT
        #ifndef vm_ghost_has_vm_ghost_intptr_t
            typedef int vm_ghost_intptr_t;
            #define vm_ghost_has_vm_ghost_intptr_t 1
        #endif
        #ifndef vm_ghost_has_vm_ghost_uintptr_t
            typedef unsigned int vm_ghost_uintptr_t;
            #define vm_ghost_has_vm_ghost_uintptr_t 1
        #endif
        #ifndef vm_ghost_has_VM_GHOST_INTPTR_WIDTH
            #ifdef VM_GHOST_INT_WIDTH
                #define VM_GHOST_INTPTR_WIDTH VM_GHOST_INT_WIDTH
                #define vm_ghost_has_VM_GHOST_INTPTR_WIDTH 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_INTPTR_MAX
            #ifdef VM_GHOST_INT_MAX
                #define VM_GHOST_INTPTR_MAX VM_GHOST_INT_MAX
                #define vm_ghost_has_VM_GHOST_INTPTR_MAX 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_INTPTR_MIN
            #ifdef VM_GHOST_INT_MIN
                #define VM_GHOST_INTPTR_MIN VM_GHOST_INT_MIN
                #define vm_ghost_has_VM_GHOST_INTPTR_MIN 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_INTPTR_C
            #ifdef VM_GHOST_INT_C
                #define VM_GHOST_INTPTR_C VM_GHOST_INT_C
                #define vm_ghost_has_VM_GHOST_INTPTR_C 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_UINTPTR_WIDTH
            #ifdef VM_GHOST_UINT_WIDTH
                #define VM_GHOST_UINTPTR_WIDTH VM_GHOST_UINT_WIDTH
                #define vm_ghost_has_VM_GHOST_UINTPTR_WIDTH 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_UINTPTR_MAX
            #ifdef VM_GHOST_UINT_MAX
                #define VM_GHOST_UINTPTR_MAX VM_GHOST_UINT_MAX
                #define vm_ghost_has_VM_GHOST_UINTPTR_MAX 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_UINTPTR_C
            #ifdef VM_GHOST_UINT_C
                #define VM_GHOST_UINTPTR_C VM_GHOST_UINT_C
                #define vm_ghost_has_VM_GHOST_UINTPTR_C 1
            #endif
        #endif
    #endif

    #ifdef VM_GHOST_IMPL_INTPTR_IS_LONG
        #ifndef vm_ghost_has_vm_ghost_intptr_t
            typedef long vm_ghost_intptr_t;
            #define vm_ghost_has_vm_ghost_intptr_t 1
        #endif
        #ifndef vm_ghost_has_vm_ghost_uintptr_t
            typedef unsigned long vm_ghost_uintptr_t;
            #define vm_ghost_has_vm_ghost_uintptr_t 1
        #endif
        #ifndef vm_ghost_has_VM_GHOST_INTPTR_WIDTH
            #ifdef VM_GHOST_LONG_WIDTH
                #define VM_GHOST_INTPTR_WIDTH VM_GHOST_LONG_WIDTH
                #define vm_ghost_has_VM_GHOST_INTPTR_WIDTH 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_INTPTR_MAX
            #ifdef VM_GHOST_LONG_MAX
                #define VM_GHOST_INTPTR_MAX VM_GHOST_LONG_MAX
                #define vm_ghost_has_VM_GHOST_INTPTR_MAX 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_INTPTR_MIN
            #ifdef VM_GHOST_LONG_MIN
                #define VM_GHOST_INTPTR_MIN VM_GHOST_LONG_MIN
                #define vm_ghost_has_VM_GHOST_INTPTR_MIN 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_INTPTR_C
            #ifdef VM_GHOST_LONG_C
                #define VM_GHOST_INTPTR_C VM_GHOST_LONG_C
                #define vm_ghost_has_VM_GHOST_INTPTR_C 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_UINTPTR_WIDTH
            #ifdef VM_GHOST_ULONG_WIDTH
                #define VM_GHOST_UINTPTR_WIDTH VM_GHOST_ULONG_WIDTH
                #define vm_ghost_has_VM_GHOST_UINTPTR_WIDTH 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_UINTPTR_MAX
            #ifdef VM_GHOST_ULONG_MAX
                #define VM_GHOST_UINTPTR_MAX VM_GHOST_ULONG_MAX
                #define vm_ghost_has_VM_GHOST_UINTPTR_MAX 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_UINTPTR_C
            #ifdef VM_GHOST_ULONG_C
                #define VM_GHOST_UINTPTR_C VM_GHOST_ULONG_C
                #define vm_ghost_has_VM_GHOST_UINTPTR_C 1
            #endif
        #endif
    #endif

    #ifdef VM_GHOST_IMPL_INTPTR_IS_LLONG
        #ifndef vm_ghost_has_vm_ghost_intptr_t
            typedef long long vm_ghost_intptr_t;
            #define vm_ghost_has_vm_ghost_intptr_t 1
        #endif
        #ifndef vm_ghost_has_vm_ghost_uintptr_t
            typedef unsigned long long vm_ghost_uintptr_t;
            #define vm_ghost_has_vm_ghost_uintptr_t 1
        #endif
        #ifndef vm_ghost_has_VM_GHOST_INTPTR_WIDTH
            #ifdef VM_GHOST_LLONG_WIDTH
                #define VM_GHOST_INTPTR_WIDTH VM_GHOST_LLONG_WIDTH
                #define vm_ghost_has_VM_GHOST_INTPTR_WIDTH 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_INTPTR_MAX
            #ifdef VM_GHOST_LLONG_MAX
                #define VM_GHOST_INTPTR_MAX VM_GHOST_LLONG_MAX
                #define vm_ghost_has_VM_GHOST_INTPTR_MAX 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_INTPTR_MIN
            #ifdef VM_GHOST_LLONG_MIN
                #define VM_GHOST_INTPTR_MIN VM_GHOST_LLONG_MIN
                #define vm_ghost_has_VM_GHOST_INTPTR_MIN 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_INTPTR_C
            #ifdef VM_GHOST_LLONG_C
                #define VM_GHOST_INTPTR_C VM_GHOST_LLONG_C
                #define vm_ghost_has_VM_GHOST_INTPTR_C 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_UINTPTR_WIDTH
            #ifdef VM_GHOST_ULLONG_WIDTH
                #define VM_GHOST_UINTPTR_WIDTH VM_GHOST_ULLONG_WIDTH
                #define vm_ghost_has_VM_GHOST_UINTPTR_WIDTH 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_UINTPTR_MAX
            #ifdef VM_GHOST_ULLONG_MAX
                #define VM_GHOST_UINTPTR_MAX VM_GHOST_ULLONG_MAX
                #define vm_ghost_has_VM_GHOST_UINTPTR_MAX 1
            #endif
        #endif
        #ifndef vm_ghost_has_VM_GHOST_UINTPTR_C
            #ifdef VM_GHOST_ULLONG_C
                #define VM_GHOST_UINTPTR_C VM_GHOST_ULLONG_C
                #define vm_ghost_has_VM_GHOST_UINTPTR_C 1
            #endif
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_intptr_t
    #define vm_ghost_has_vm_ghost_intptr_t 0
#endif
#ifndef vm_ghost_has_VM_GHOST_INTPTR_WIDTH
    #define vm_ghost_has_VM_GHOST_INTPTR_WIDTH 0
#endif
#ifndef vm_ghost_has_VM_GHOST_INTPTR_MAX
    #define vm_ghost_has_VM_GHOST_INTPTR_MAX 0
#endif
#ifndef vm_ghost_has_VM_GHOST_INTPTR_MIN
    #define vm_ghost_has_VM_GHOST_INTPTR_MIN 0
#endif
#ifndef vm_ghost_has_VM_GHOST_INTPTR_C
    #define vm_ghost_has_VM_GHOST_INTPTR_C 0
#endif
#ifndef vm_ghost_has_vm_ghost_uintptr_t
    #define vm_ghost_has_vm_ghost_uintptr_t 0
#endif
#ifndef vm_ghost_has_VM_GHOST_UINTPTR_WIDTH
    #define vm_ghost_has_VM_GHOST_UINTPTR_WIDTH 0
#endif
#ifndef vm_ghost_has_VM_GHOST_UINTPTR_MAX
    #define vm_ghost_has_VM_GHOST_UINTPTR_MAX 0
#endif
#ifndef vm_ghost_has_VM_GHOST_UINTPTR_C
    #define vm_ghost_has_VM_GHOST_UINTPTR_C 0
#endif

#if vm_ghost_has_vm_ghost_intptr_t
    vm_ghost_static_assert(sizeof(vm_ghost_intptr_t) >= sizeof(void*),
            "vm_ghost_intptr_t is not large enough to store a void pointer");
    vm_ghost_static_assert(sizeof(vm_ghost_uintptr_t) >= sizeof(void*),
            "vm_ghost_uintptr_t is not large enough to store a void pointer");
#endif

#if vm_ghost_has_VM_GHOST_CHAR_WIDTH
    #if vm_ghost_has_VM_GHOST_INTPTR_WIDTH
        vm_ghost_static_assert(sizeof(vm_ghost_intptr_t) * VM_GHOST_CHAR_WIDTH == VM_GHOST_INTPTR_WIDTH,
                "VM_GHOST_INTPTR_WIDTH is incorrect");
    #endif
    #if vm_ghost_has_VM_GHOST_UINTPTR_WIDTH
        vm_ghost_static_assert(sizeof(vm_ghost_uintptr_t) * VM_GHOST_CHAR_WIDTH == VM_GHOST_UINTPTR_WIDTH,
                "VM_GHOST_UINTPTR_WIDTH is incorrect");
    #endif
#endif

#if vm_ghost_has_VM_GHOST_UINTPTR_MAX
    #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
        VM_GHOST_SILENCE_PUSH
        VM_GHOST_SILENCE_CAST_TRUNCATES_CONSTANT
        VM_GHOST_SILENCE_CONSTANT_OVERFLOW
    #endif

    vm_ghost_static_assert(0 == vm_ghost_static_cast(vm_ghost_uintptr_t, VM_GHOST_UINTPTR_MAX + 1U),
            "VM_GHOST_UINTPTR_MAX is incorrect");

    #if VM_GHOST_IMPL_ASSUME_TWOS_COMPLEMENT
        vm_ghost_static_assert(VM_GHOST_INTPTR_MAX == vm_ghost_static_cast(vm_ghost_intptr_t, VM_GHOST_UINTPTR_MAX >> 1U),
                "VM_GHOST_INTPTR_MAX is incorrect!");
        vm_ghost_static_assert(VM_GHOST_INTPTR_MIN == -vm_ghost_static_cast(vm_ghost_intptr_t, VM_GHOST_UINTPTR_MAX >> 1U) - 1,
                "VM_GHOST_INTPTR_MIN is incorrect!");
    #endif

    #if vm_ghost_has(VM_GHOST_SILENCE_PUSH) && vm_ghost_has(VM_GHOST_SILENCE_POP)
        VM_GHOST_SILENCE_POP
    #endif
#endif

/**********************************
 * ghost/type/ghost_uintptr_all.h
 **********************************/

#if VM_GHOST_DOCUMENTATION

typedef uintptr_t vm_ghost_uintptr_t;

#endif

/******************************
 * ghost/debug/ghost_ensure.h
 ******************************/

#ifndef vm_ghost_has_vm_ghost_ensure
    #ifdef vm_ghost_ensure
        #define vm_ghost_has_vm_ghost_ensure 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_ensure
    #if vm_ghost_has(vm_ghost_assert_fail)
        #ifdef VM_GHOST_IMPL_ENSURE_PRETTY_FUNCTION
            #error "Cannot pre-define VM_GHOST_IMPL_ENSURE_PRETTY_FUNCTION."
        #endif
        #if vm_ghost_has(VM_GHOST_PRETTY_FUNCTION)
            #define VM_GHOST_IMPL_ENSURE_PRETTY_FUNCTION VM_GHOST_PRETTY_FUNCTION
        #else
            #define VM_GHOST_IMPL_ENSURE_PRETTY_FUNCTION vm_ghost_null
        #endif

        #if !VM_GHOST_PP_VA_ARGS
            #define vm_ghost_ensure(x, msg) ((vm_ghost_expect_true(x)) ? ((void)0) : \
                vm_ghost_assert_fail(#x, "" msg, __FILE__, __LINE__, VM_GHOST_IMPL_ENSURE_PRETTY_FUNCTION))
        #else
            #if VM_GHOST_OMIT_VA_ARGS
                #define vm_ghost_ensure(x, ...) ((vm_ghost_expect_true(x)) ? ((void)0) : \
                    vm_ghost_assert_fail(#x, "" __VA_ARGS__, __FILE__, __LINE__, VM_GHOST_IMPL_ENSURE_PRETTY_FUNCTION))
            #else
                #define vm_ghost_ensure(...) vm_ghost_ensure_impl(__VA_ARGS__, "", 0)
                #define vm_ghost_ensure_impl(x, msg, ...) ((vm_ghost_expect_true(x)) ? ((void)0) : \
                        vm_ghost_assert_fail(#x, "" msg, __FILE__, __LINE__, VM_GHOST_IMPL_ENSURE_PRETTY_FUNCTION))
            #endif
        #endif

        #define vm_ghost_has_vm_ghost_ensure 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_ensure
    #define vm_ghost_has_vm_ghost_ensure 0
#endif

/************************************
 * ghost/language/ghost_auto_cast.h
 ************************************/

#ifndef vm_ghost_has_vm_ghost_auto_cast
    #ifdef vm_ghost_auto_cast
        #define vm_ghost_has_vm_ghost_auto_cast 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_auto_cast
    #ifdef __cplusplus
        VM_GHOST_IMPL_CXX_FUNCTION_OPEN

        struct vm_ghost_impl_auto_cast_cxx {
            template <class vm_ghost_impl_v_T>
            vm_ghost_always_inline
            explicit
            vm_ghost_impl_auto_cast_cxx(vm_ghost_impl_v_T vm_ghost_impl_v_p_arg);

            template <class vm_ghost_impl_v_T>
            vm_ghost_always_inline
            operator vm_ghost_impl_v_T* () {
                return static_cast<vm_ghost_impl_v_T*>(vm_ghost_impl_v_p);
            }

            #if vm_ghost_cplusplus >= 201103L
                #if VM_GHOST_GCC
                    #if __GNUC__ >= 6 && __GNUC__ <= 7
                        #pragma GCC diagnostic push
                        #pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
                    #endif
                #endif

                vm_ghost_always_inline
                bool operator==(decltype(nullptr) vm_ghost_impl_v_n) {
                    return vm_ghost_impl_v_p == vm_ghost_impl_v_n;
                }

                #if VM_GHOST_GCC
                    #if __GNUC__ >= 6 && __GNUC__ <= 7
                        #pragma GCC diagnostic pop
                    #endif
                #endif
            #endif

            vm_ghost_always_inline
            bool operator==(int vm_ghost_impl_v_n) {
                #if vm_ghost_has(vm_ghost_ensure)
                    vm_ghost_ensure(vm_ghost_impl_v_n == 0, "cannot compare auto-cast pointer against non-zero int");
                #else
                    vm_ghost_discard(vm_ghost_impl_v_n);
                #endif
                return vm_ghost_impl_v_p == vm_ghost_null;
            }

        private:
            void* vm_ghost_impl_v_p;
        };

        template <>
        vm_ghost_always_inline
        vm_ghost_impl_auto_cast_cxx::vm_ghost_impl_auto_cast_cxx(void* vm_ghost_impl_v_p_arg)
            : vm_ghost_impl_v_p(vm_ghost_impl_v_p_arg)
        {}

        VM_GHOST_IMPL_CXX_FUNCTION_CLOSE

        #define vm_ghost_auto_cast(x) vm_ghost_impl_auto_cast_cxx(x)
        #define vm_ghost_has_vm_ghost_auto_cast 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_auto_cast
    #if !defined(__PGI) && !defined(__PCC__)
        #if vm_ghost_has(vm_ghost_generic)
            #define vm_ghost_auto_cast(x) \
                vm_ghost_generic((x), \
                    void*: (x), \
                    const void*: (x))
        #else
            #define vm_ghost_auto_cast(x) (x)
        #endif
        #define vm_ghost_has_vm_ghost_auto_cast 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_auto_cast
    #define vm_ghost_auto_cast(x) (x)
    #define vm_ghost_has_vm_ghost_auto_cast 1
#endif

/********************************
 * ghost/language/ghost_bless.h
 ********************************/

#ifndef vm_ghost_has_vm_ghost_bless
    #ifdef vm_ghost_bless
        #define vm_ghost_has_vm_ghost_bless 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_bless
    #ifdef __cplusplus
        #include <new> 
        VM_GHOST_IMPL_CXX_FUNCTION_OPEN
        template <typename vm_ghost_impl_v_T>
        vm_ghost_always_inline
        vm_ghost_impl_v_T* vm_ghost_bless_impl(vm_ghost_size_t vm_ghost_impl_v_n, void* vm_ghost_impl_v_p) {
            if (vm_ghost_impl_v_p)
                new (vm_ghost_impl_v_p) char[vm_ghost_impl_v_n * sizeof(vm_ghost_impl_v_T)];
            return static_cast<vm_ghost_impl_v_T*>(vm_ghost_impl_v_p);
        }
        VM_GHOST_IMPL_CXX_FUNCTION_CLOSE
        #define vm_ghost_bless(T, n, p) vm_ghost_bless_impl<T>(n, p)
        #define vm_ghost_has_vm_ghost_bless 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_bless
    #define vm_ghost_bless(T, n, p) (vm_ghost_discard(n), ((T*)(p)))
    #define vm_ghost_has_vm_ghost_bless 1
#endif

/**************************************
 * ghost/language/ghost_emit_inline.h
 **************************************/

#ifndef vm_ghost_has_vm_ghost_emit_inline
    #ifdef vm_ghost_emit_inline
        #define vm_ghost_has_vm_ghost_emit_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_inline
    #ifdef __cplusplus
        #define vm_ghost_emit_inline inline
        #define vm_ghost_has_vm_ghost_emit_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_inline
    #ifdef _MSC_VER
        #if _MSC_VER >= 1900
            #define vm_ghost_emit_inline inline
            #define vm_ghost_has_vm_ghost_emit_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_inline
    #if !vm_ghost_has(vm_ghost_inline)
        #define vm_ghost_has_vm_ghost_emit_inline 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_inline
    #if defined(__GNUC__) && (defined(__GNUC_GNU_INLINE__) || \
            (!defined(__GNUC_STDC_INLINE__) && !defined(__GNUC_GNU_INLINE__)))
        #define vm_ghost_emit_inline vm_ghost_inline
        #define vm_ghost_has_vm_ghost_emit_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_inline
    #if defined(__STDC_VERSION__)
        #if VM_GHOST_STDC_VERSION >= 199901L
            #define vm_ghost_emit_inline extern inline
            #define vm_ghost_has_vm_ghost_emit_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_emit_inline
    #define vm_ghost_has_vm_ghost_emit_inline 0
#endif

/***************************************
 * ghost/language/ghost_expect_false.h
 ***************************************/

#ifndef vm_ghost_has_vm_ghost_expect_false
    #ifdef vm_ghost_expect_false
        #define vm_ghost_has_vm_ghost_expect_false 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_expect_false
    #define vm_ghost_expect_false(x) vm_ghost_expect(!!(x), 0)
    #define vm_ghost_has_vm_ghost_expect_false 1
#endif

/******************************************
 * ghost/language/ghost_header_function.h
 ******************************************/

#ifndef vm_ghost_has_vm_ghost_header_function
    #ifdef vm_ghost_header_function
        #define vm_ghost_has_vm_ghost_header_function 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_function
    #ifdef __cplusplus
        #define vm_ghost_header_function inline
        #define vm_ghost_has_vm_ghost_header_function 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_inline
    #if defined(_MSC_VER)
        #if _MSC_VER >= 1900
            #define vm_ghost_header_inline inline
            #define vm_ghost_has_vm_ghost_header_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_function
    #if vm_ghost_has(vm_ghost_weak)
        #define vm_ghost_header_function vm_ghost_weak
        #define vm_ghost_has_vm_ghost_header_function 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_function
    #define vm_ghost_header_function vm_ghost_noinline vm_ghost_maybe_unused static
    #define vm_ghost_has_vm_ghost_header_function 1
#endif

/****************************************
 * ghost/language/ghost_header_inline.h
 ****************************************/

#ifndef vm_ghost_has_vm_ghost_header_inline
    #ifdef vm_ghost_header_inline
        #define vm_ghost_has_vm_ghost_header_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_inline
    #if defined(__cplusplus)
        #define vm_ghost_header_inline inline
        #define vm_ghost_has_vm_ghost_header_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_inline
    #if defined(_MSC_VER)
        #if _MSC_VER >= 1900
            #define vm_ghost_header_inline inline
            #define vm_ghost_has_vm_ghost_header_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_inline
    #if vm_ghost_has(vm_ghost_inline)
        #ifdef __PGI
            #define vm_ghost_header_inline vm_ghost_maybe_unused static vm_ghost_inline
        #else
            #define vm_ghost_header_inline static vm_ghost_inline
        #endif
        #define vm_ghost_has_vm_ghost_header_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_header_inline
    #define vm_ghost_header_inline vm_ghost_maybe_unused static
    #define vm_ghost_has_vm_ghost_header_inline 1
#endif

/****************************************
 * ghost/language/ghost_noemit_inline.h
 ****************************************/

#ifndef vm_ghost_has_vm_ghost_noemit_inline
    #ifdef vm_ghost_noemit_inline
        #define vm_ghost_has_vm_ghost_noemit_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noemit_inline
    #ifdef __cplusplus
        #define vm_ghost_noemit_inline inline
        #define vm_ghost_has_vm_ghost_noemit_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noemit_inline
    #ifdef _MSC_VER
        #if _MSC_VER >= 1900
            #define vm_ghost_noemit_inline inline
            #define vm_ghost_has_vm_ghost_noemit_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noemit_inline
    #if !vm_ghost_has(vm_ghost_inline)
        #define vm_ghost_has_vm_ghost_noemit_inline 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noemit_inline
    #if defined(__GNUC__) && (defined(__GNUC_GNU_INLINE__) || \
            (!defined(__GNUC_STDC_INLINE__) && !defined(__GNUC_GNU_INLINE__)))
        #define vm_ghost_noemit_inline extern vm_ghost_inline
        #define vm_ghost_has_vm_ghost_noemit_inline 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noemit_inline
    #if defined(__STDC_VERSION__)
        #if VM_GHOST_STDC_VERSION >= 199901L
            #define vm_ghost_noemit_inline inline
            #define vm_ghost_has_vm_ghost_noemit_inline 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_noemit_inline
    #define vm_ghost_has_vm_ghost_noemit_inline 0
#endif

/***********************************
 * ghost/language/ghost_restrict.h
 ***********************************/

#ifndef vm_ghost_has_vm_ghost_restrict
    #ifdef __STDC_VERSION__
        #if VM_GHOST_STDC_VERSION >= 199901L
            #define vm_ghost_restrict restrict
            #define vm_ghost_has_vm_ghost_restrict 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_restrict
    #if defined(_MSC_VER)
        #define vm_ghost_restrict __restrict
        #define vm_ghost_has_vm_ghost_restrict 1
    #elif defined(__GNUC__)
        #define vm_ghost_restrict __restrict__
        #define vm_ghost_has_vm_ghost_restrict 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_restrict
    #define vm_ghost_restrict 
    #define vm_ghost_has_vm_ghost_restrict 1
#endif

/********************************
 * ghost/type/bool/ghost_bool.h
 ********************************/

#if VM_GHOST_DOCUMENTATION
typedef _Bool vm_ghost_bool;
#endif

#if defined(__cplusplus)
    #ifndef vm_ghost_has_vm_ghost_bool
        typedef bool vm_ghost_bool;
        #define vm_ghost_has_vm_ghost_bool 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_bool
    #ifdef __STDC_VERSION__
        #if VM_GHOST_STDC_VERSION >= 199901L
            typedef _Bool vm_ghost_bool;
            #define vm_ghost_has_vm_ghost_bool 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_bool
    #if defined(__GNUC__) && !defined(__CPARSER__)
        __extension__ typedef _Bool vm_ghost_bool;
        #define vm_ghost_has_vm_ghost_bool 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_bool
    #ifdef _MSC_VER
        #if _MSC_VER >= 1800
            typedef _Bool vm_ghost_bool;
            #define vm_ghost_has_vm_ghost_bool 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_bool
    typedef unsigned char vm_ghost_bool;
    #define vm_ghost_has_vm_ghost_bool 1
#endif

/********************************
 * ghost/type/ghost_uchar_all.h
 ********************************/

#ifndef vm_ghost_has_vm_ghost_uchar
    #define vm_ghost_has_vm_ghost_uchar 1
#endif

#ifndef vm_ghost_has_VM_GHOST_UCHAR_WIDTH
    #ifdef VM_GHOST_UCHAR_WIDTH
        #define vm_ghost_has_VM_GHOST_UCHAR_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UCHAR_WIDTH
    #if defined(UCHAR_WIDTH)
        #define VM_GHOST_UCHAR_WIDTH UCHAR_WIDTH
        #define vm_ghost_has_VM_GHOST_UCHAR_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UCHAR_WIDTH
    #if vm_ghost_has_VM_GHOST_CHAR_WIDTH
        #if VM_GHOST_IMPL_ASSUME_SIGNED_UNSIGNED_SAME_WIDTH
            #define VM_GHOST_UCHAR_WIDTH VM_GHOST_CHAR_WIDTH
            #define vm_ghost_has_VM_GHOST_UCHAR_WIDTH 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UCHAR_WIDTH
    #define vm_ghost_has_VM_GHOST_UCHAR_WIDTH 0
#endif

#ifndef vm_ghost_has_VM_GHOST_UCHAR_C
    #ifndef VM_GHOST_UCHAR_C
        #define VM_GHOST_UCHAR_C(x) x ## U
    #endif
    #define vm_ghost_has_VM_GHOST_UCHAR_C 1
#endif

#ifndef vm_ghost_has_VM_GHOST_UCHAR_MAX
    #ifdef VM_GHOST_UCHAR_MAX
        #define vm_ghost_has_VM_GHOST_UCHAR_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UCHAR_MAX
    #ifdef UCHAR_MAX
        #define VM_GHOST_UCHAR_MAX UCHAR_MAX
        #define vm_ghost_has_VM_GHOST_UCHAR_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UCHAR_MAX
    #if vm_ghost_has_VM_GHOST_CHAR_IS_SIGNED
        #if !VM_GHOST_CHAR_IS_SIGNED && vm_ghost_has_VM_GHOST_CHAR_MAX
            #define VM_GHOST_UCHAR_MAX VM_GHOST_CHAR_MAX
            #define vm_ghost_has_VM_GHOST_UCHAR_MAX 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UCHAR_MAX
    #if vm_ghost_has_VM_GHOST_UCHAR_WIDTH
        #if VM_GHOST_UCHAR_WIDTH == 8
            #define VM_GHOST_UCHAR_MAX 255
        #else
            #define VM_GHOST_UCHAR_MAX ((((1U << (VM_GHOST_UCHAR_WIDTH - 1U)) - 1U) << 1U) + 1U)
        #endif
        #define vm_ghost_has_VM_GHOST_UCHAR_MAX 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UCHAR_MAX
    #define vm_ghost_has_VM_GHOST_UCHAR_MAX 0
#endif

/**********************************
 * ghost/type/short/ghost_short.h
 **********************************/

#ifndef vm_ghost_has_vm_ghost_short
    #define vm_ghost_has_vm_ghost_short 1
#endif

/****************************************
 * ghost/type/short/ghost_short_width.h
 ****************************************/

#ifndef vm_ghost_has_VM_GHOST_SHORT_WIDTH
    #ifdef VM_GHOST_SHORT_WIDTH
        #define vm_ghost_has_VM_GHOST_SHORT_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SHORT_WIDTH
    #if !vm_ghost_has(vm_ghost_short)
        #define vm_ghost_has_VM_GHOST_SHORT_WIDTH 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SHORT_WIDTH
    #ifdef SHRT_WIDTH
        #define VM_GHOST_SHORT_WIDTH SHRT_WIDTH
        #define vm_ghost_has_VM_GHOST_SHORT_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SHORT_WIDTH
    #ifdef __SHRT_WIDTH__
        #define VM_GHOST_SHORT_WIDTH __SHRT_WIDTH__
        #define vm_ghost_has_VM_GHOST_SHORT_WIDTH 1
    #elif defined(__SIZEOF_SHORT__)
        #define VM_GHOST_SHORT_WIDTH (__SIZEOF_SHORT__ * VM_GHOST_CHAR_WIDTH)
        #define vm_ghost_has_VM_GHOST_SHORT_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SHORT_WIDTH
    #if defined(_WIN32) || \
            defined(__LP64__) || defined(_LP64) || \
            defined(__ILP32__) || defined(_ILP32) || \
            defined(__x86_64__) || defined(__i386__) || \
            defined(__arm__) || defined(__aarch64__) || \
            defined(__riscv) || defined(__riscv__) || \
            defined(__wasm) || defined(__wasm__)
        #define VM_GHOST_SHORT_WIDTH 16
        #define vm_ghost_has_VM_GHOST_SHORT_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_SHORT_WIDTH
    #define vm_ghost_has_VM_GHOST_SHORT_WIDTH 0
#endif

#if vm_ghost_has(VM_GHOST_SHORT_WIDTH)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        vm_ghost_static_assert(sizeof(short) * VM_GHOST_CHAR_WIDTH == VM_GHOST_SHORT_WIDTH,
                "VM_GHOST_SHORT_WIDTH is incorrect");
    #endif
#endif

/******************************************
 * ghost/type/ushort/ghost_ushort_width.h
 ******************************************/

#ifndef vm_ghost_has_VM_GHOST_USHORT_WIDTH
    #ifdef VM_GHOST_USHORT_WIDTH
        #define vm_ghost_has_VM_GHOST_USHORT_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_USHORT_WIDTH
    #if !vm_ghost_has(vm_ghost_short)
        #define vm_ghost_has_VM_GHOST_USHORT_WIDTH 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_USHORT_WIDTH
    #ifdef USHRT_WIDTH
        #define VM_GHOST_USHORT_WIDTH USHRT_WIDTH
        #define vm_ghost_has_VM_GHOST_USHORT_WIDTH 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_USHORT_WIDTH
    #if VM_GHOST_IMPL_ASSUME_SIGNED_UNSIGNED_SAME_WIDTH
        #if vm_ghost_has(VM_GHOST_SHORT_WIDTH)
            #define VM_GHOST_USHORT_WIDTH VM_GHOST_SHORT_WIDTH
            #define vm_ghost_has_VM_GHOST_USHORT_WIDTH 1
        #endif
    #endif
#endif

#if vm_ghost_has(VM_GHOST_USHORT_WIDTH)
    #if VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        vm_ghost_static_assert(sizeof(unsigned short) * VM_GHOST_CHAR_WIDTH == VM_GHOST_USHORT_WIDTH,
                "VM_GHOST_USHORT_WIDTH is incorrect");
    #endif
#endif

/****************************************
 * ghost/type/uint32_t/ghost_uint32_t.h
 ****************************************/

#if VM_GHOST_DOCUMENTATION
typedef uint32_t vm_ghost_uint32_t;
#endif

#ifndef vm_ghost_has_vm_ghost_uint32_t
    #ifdef vm_ghost_uint32_t
        #define vm_ghost_has_vm_ghost_uint32_t 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint32_t
    #if vm_ghost_has(vm_ghost_stdint_h) && defined(UINT32_MAX)
        typedef uint32_t vm_ghost_uint32_t;
        #define vm_ghost_has_vm_ghost_uint32_t 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint32_t
    #ifdef __UINT32_TYPE__
        typedef __UINT32_TYPE__ vm_ghost_uint32_t;
        #define vm_ghost_has_vm_ghost_uint32_t 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint32_t
    #ifdef _MSC_VER
        typedef unsigned __int32 vm_ghost_uint32_t;
        #define vm_ghost_has_vm_ghost_uint32_t 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint32_t
    #if !VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        #define vm_ghost_has_vm_ghost_uint32_t 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint32_t
    #if vm_ghost_has(VM_GHOST_UCHAR_WIDTH)
        #if VM_GHOST_UCHAR_WIDTH == 32
            typedef unsigned char vm_ghost_uint32_t;
            #define vm_ghost_has_vm_ghost_uint32_t 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint32_t
    #if vm_ghost_has(VM_GHOST_USHORT_WIDTH)
        #if VM_GHOST_USHORT_WIDTH == 32
            typedef unsigned short vm_ghost_uint32_t;
            #define vm_ghost_has_vm_ghost_uint32_t 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint32_t
    #if vm_ghost_has(VM_GHOST_UINT_WIDTH)
        #if VM_GHOST_UINT_WIDTH == 32
            typedef unsigned int vm_ghost_uint32_t;
            #define vm_ghost_has_vm_ghost_uint32_t 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint32_t
    #if vm_ghost_has(VM_GHOST_ULONG_WIDTH)
        #if VM_GHOST_ULONG_WIDTH == 32
            typedef unsigned long vm_ghost_uint32_t;
            #define vm_ghost_has_vm_ghost_uint32_t 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint32_t
    #if vm_ghost_has(VM_GHOST_ULLONG_WIDTH)
        #if VM_GHOST_ULLONG_WIDTH == 32
            typedef vm_ghost_ullong vm_ghost_uint32_t;
            #define vm_ghost_has_vm_ghost_uint32_t 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint32_t
    #define vm_ghost_has_vm_ghost_uint32_t 0
#endif

/**************************************
 * ghost/malloc/ghost_malloc_system.h
 **************************************/

#ifndef vm_ghost_has_vm_ghost_aligned_free
    #ifdef vm_ghost_aligned_free
        #define vm_ghost_has_vm_ghost_aligned_free 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_aligned_malloc
    #ifdef vm_ghost_aligned_malloc
        #define vm_ghost_has_vm_ghost_aligned_malloc 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_calloc
    #ifdef vm_ghost_calloc
        #define vm_ghost_has_vm_ghost_calloc 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_calloc_0any
    #ifdef vm_ghost_calloc_0any
        #define vm_ghost_has_vm_ghost_calloc_0any 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_calloc_0null
    #ifdef vm_ghost_calloc_0null
        #define vm_ghost_has_vm_ghost_calloc_0null 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_free
    #ifdef vm_ghost_free
        #define vm_ghost_has_vm_ghost_free 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_free_nonnull
    #ifdef vm_ghost_free_nonnull
        #define vm_ghost_has_vm_ghost_free_nonnull 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_malloc
    #ifdef vm_ghost_malloc
        #define vm_ghost_has_vm_ghost_malloc 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_malloc_0any
    #ifdef vm_ghost_malloc_0any
        #define vm_ghost_has_vm_ghost_malloc_0any 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_malloc_0null
    #ifdef vm_ghost_malloc_0null
        #define vm_ghost_has_vm_ghost_malloc_0null 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_malloc_good_size
    #ifdef vm_ghost_malloc_good_size
        #define vm_ghost_has_vm_ghost_malloc_good_size 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_malloc_size
    #ifdef vm_ghost_malloc_size
        #define vm_ghost_has_vm_ghost_malloc_size 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_memdup
    #ifdef vm_ghost_memdup
        #define vm_ghost_has_vm_ghost_memdup 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_realloc
    #ifdef vm_ghost_realloc
        #define vm_ghost_has_vm_ghost_realloc 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_realloc_0alloc
    #ifdef vm_ghost_realloc_0alloc
        #define vm_ghost_has_vm_ghost_realloc_0alloc 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_realloc_0any
    #ifdef vm_ghost_realloc_0any
        #define vm_ghost_has_vm_ghost_realloc_0any 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_realloc_0free
    #ifdef vm_ghost_realloc_0free
        #define vm_ghost_has_vm_ghost_realloc_0free 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_realloc_0null
    #ifdef vm_ghost_realloc_0null
        #define vm_ghost_has_vm_ghost_realloc_0null 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_strdup
    #ifdef vm_ghost_strdup
        #define vm_ghost_has_vm_ghost_strdup 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_strndup
    #ifdef vm_ghost_strndup
        #define vm_ghost_has_vm_ghost_strndup 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_asprintf
    #ifdef vm_ghost_asprintf
        #define vm_ghost_has_vm_ghost_asprintf 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_vasprintf
    #ifdef vm_ghost_vasprintf
        #define vm_ghost_has_vm_ghost_vasprintf 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MALLOC_SYSTEM
    #ifdef VM_GHOST_MALLOC_SYSTEM
        #define vm_ghost_has_VM_GHOST_MALLOC_SYSTEM 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MALLOC_SYSTEM
    #if \
             \
            defined(vm_ghost_has_vm_ghost_aligned_free) || \
            defined(vm_ghost_has_vm_ghost_aligned_malloc) || \
            defined(vm_ghost_has_vm_ghost_calloc) || \
            defined(vm_ghost_has_vm_ghost_calloc_0any) || \
            defined(vm_ghost_has_vm_ghost_calloc_0null) || \
            defined(vm_ghost_has_vm_ghost_free) || \
            defined(vm_ghost_has_vm_ghost_free_nonnull) || \
            defined(vm_ghost_has_vm_ghost_malloc) || \
            defined(vm_ghost_has_vm_ghost_malloc_0any) || \
            defined(vm_ghost_has_vm_ghost_malloc_0null) || \
            defined(vm_ghost_has_vm_ghost_malloc_good_size) || \
            defined(vm_ghost_has_vm_ghost_malloc_size) || \
            defined(vm_ghost_has_vm_ghost_memdup) || \
            defined(vm_ghost_has_vm_ghost_realloc) || \
            defined(vm_ghost_has_vm_ghost_realloc_0alloc) || \
            defined(vm_ghost_has_vm_ghost_realloc_0any) || \
            defined(vm_ghost_has_vm_ghost_realloc_0free) || \
            defined(vm_ghost_has_vm_ghost_realloc_0null) || \
            defined(vm_ghost_has_vm_ghost_strdup) || \
            defined(vm_ghost_has_vm_ghost_strndup) || \
             \
            defined(vm_ghost_has_vm_ghost_asprintf) || \
            defined(vm_ghost_has_vm_ghost_vasprintf)
        #define VM_GHOST_MALLOC_SYSTEM 0
        #define vm_ghost_has_VM_GHOST_MALLOC_SYSTEM 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MALLOC_SYSTEM
    #if VM_GHOST_HOSTED
        #define VM_GHOST_MALLOC_SYSTEM 1
    #else
        #define VM_GHOST_MALLOC_SYSTEM 0
    #endif
    #define vm_ghost_has_VM_GHOST_MALLOC_SYSTEM 1
#endif

/******************************
 * ghost/detect/ghost_glibc.h
 ******************************/

#ifndef vm_ghost_has_VM_GHOST_GLIBC
    #ifndef VM_GHOST_GLIBC
        #if defined(__GLIBC__) && !defined(__UCLIBC__)
            #define VM_GHOST_GLIBC 1
        #else
            #define VM_GHOST_GLIBC 0
        #endif
    #endif
    #define vm_ghost_has_VM_GHOST_GLIBC 1
#endif

/*****************************
 * ghost/detect/ghost_musl.h
 *****************************/

#ifndef vm_ghost_has_VM_GHOST_MUSL_VERSION
    #ifdef VM_GHOST_MUSL_VERSION
        #define vm_ghost_has_VM_GHOST_MUSL_VERSION 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MUSL
    #ifdef VM_GHOST_MUSL
        #define vm_ghost_has_VM_GHOST_MUSL 1
    #elif defined(VM_GHOST_MUSL_VERSION)
        #define VM_GHOST_MUSL 1
        #define vm_ghost_has_VM_GHOST_MUSL 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MUSL
    #ifndef __linux__
        #define VM_GHOST_MUSL 0
        #define vm_ghost_has_VM_GHOST_MUSL 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MUSL
    #if !VM_GHOST_HOSTED
        #define VM_GHOST_MUSL 0
        #define vm_ghost_has_VM_GHOST_MUSL 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MUSL
    #if !vm_ghost_has(vm_ghost_string_h)
        #define VM_GHOST_MUSL 0
        #define vm_ghost_has_VM_GHOST_MUSL 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MUSL
    #if defined(__musl__) || defined(__MUSL__) || defined(__x_musl_version__)
        #define VM_GHOST_MUSL 1
        #define vm_ghost_has_VM_GHOST_MUSL 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MUSL
    #if defined(__GLIBC__) || defined(__UCLIBC__) || defined(__KLIBC__) || \
            defined(__NEWLIB__) || defined(__BIONIC__) || defined(__dietlibc__) || \
             \
            defined(_PDCLIB_NULL) || defined(__MLIBC_ANSI_OPTION)
        #define VM_GHOST_MUSL 0
        #define vm_ghost_has_VM_GHOST_MUSL 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MUSL
    #if vm_ghost_has(vm_ghost_has_include)
        #if \
                !__has_include(<byteswap.h>) || \
                !__has_include(<elf.h>) || \
                !__has_include(<endian.h>) || \
                !__has_include(<features.h>) || \
                !__has_include(<stdio_ext.h>) || \
                !__has_include(<stropts.h>) || \
                !__has_include(<sys/syscall.h>) || \
                !__has_include(<sys/resource.h>) || \
                !__has_include(<sysexits.h>) || \
                !__has_include(<values.h>) || \
                \
                !__has_include(<bits/alltypes.h>) 
            #define VM_GHOST_MUSL 0
            #define vm_ghost_has_VM_GHOST_MUSL 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MUSL
    #if !defined(__NEED_size_t) || !defined(__DEFINED_size_t)
        #define VM_GHOST_MUSL 0
        #define vm_ghost_has_VM_GHOST_MUSL 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MUSL
    #ifdef __NOT_MUSL__
        #define VM_GHOST_MUSL 0
        #define vm_ghost_has_VM_GHOST_MUSL 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_MUSL
    #define VM_GHOST_MUSL 1
    #define vm_ghost_has_VM_GHOST_MUSL 1
#endif

/*****************************************
 * ghost/header/linux/ghost_features_h.h
 *****************************************/

#ifndef vm_ghost_has_vm_ghost_features_h
    #ifdef VM_GHOST_FEATURES_H
        #include VM_GHOST_FEATURES_H
        #define vm_ghost_has_vm_ghost_features_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_features_h
    #if vm_ghost_has(vm_ghost_has_include)
        #if __has_include(<features.h>)
            #include <features.h>
            #define vm_ghost_has_vm_ghost_features_h 1
        #else
            #define vm_ghost_has_vm_ghost_features_h 0
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_features_h
    #ifndef __linux__
        #define vm_ghost_has_vm_ghost_features_h 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_features_h
    #if !vm_ghost_has(vm_ghost_string_h)
        #define vm_ghost_has_vm_ghost_features_h 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_features_h
    #if defined(__GLIBC__) || defined(__UCLIBC__) || VM_GHOST_MUSL || \
            defined(__BIONIC__) || defined(__dietlibc__)
        #include <features.h>
        #define vm_ghost_has_vm_ghost_features_h 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_features_h
    #define vm_ghost_has_vm_ghost_features_h 0
#endif

/*******************************************
 * ghost/language/ghost_reinterpret_cast.h
 *******************************************/

#ifndef vm_ghost_has_vm_ghost_reinterpret_cast
    #ifdef __cplusplus
        #define vm_ghost_reinterpret_cast(Type, value) (reinterpret_cast<Type>(value))
    #else
        #define vm_ghost_reinterpret_cast(Type, value) ((Type)(value))
    #endif
    #define vm_ghost_has_vm_ghost_reinterpret_cast 1
#endif

/****************************************
 * ghost/type/uint32_t/ghost_uint32_c.h
 ****************************************/

#ifndef vm_ghost_has_VM_GHOST_UINT32_C
    #ifdef VM_GHOST_UINT32_C
        #define vm_ghost_has_VM_GHOST_UINT32_C 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT32_C
    #if !vm_ghost_has(vm_ghost_uint32_t)
        #define vm_ghost_has_VM_GHOST_UINT32_C 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT32_C
    #ifdef UINT32_C
        #ifdef __CPARSER__
            #define VM_GHOST_UINT32_C(x) UINT32_C(x)
        #else
            #define VM_GHOST_UINT32_C UINT32_C
        #endif
        #define vm_ghost_has_VM_GHOST_UINT32_C 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT32_C
    #ifdef __UINT32_C
        #ifdef __CPARSER__
            #define VM_GHOST_UINT32_C(x) __UINT32_C(x)
        #else
            #define VM_GHOST_UINT32_C __UINT32_C
        #endif
        #define vm_ghost_has_VM_GHOST_UINT32_C 1
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT32_C
    #if !VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        #define vm_ghost_has_VM_GHOST_UINT32_C 0
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT32_C
    #if vm_ghost_has(VM_GHOST_UCHAR_WIDTH)
        #if VM_GHOST_UCHAR_WIDTH == 32
            #define VM_GHOST_UINT32_C(x) x ## U
            #define vm_ghost_has_VM_GHOST_UINT32_C 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT32_C
    #if vm_ghost_has(VM_GHOST_USHORT_WIDTH)
        #if VM_GHOST_USHORT_WIDTH == 32
            #define VM_GHOST_UINT32_C(x) x ## U
            #define vm_ghost_has_VM_GHOST_UINT32_C 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT32_C
    #if vm_ghost_has(VM_GHOST_UINT_WIDTH)
        #if VM_GHOST_UINT_WIDTH == 32
            #define VM_GHOST_UINT32_C(x) x ## U
            #define vm_ghost_has_VM_GHOST_UINT32_C 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT32_C
    #if vm_ghost_has(VM_GHOST_ULONG_WIDTH)
        #if VM_GHOST_ULONG_WIDTH == 32
            #define VM_GHOST_UINT32_C(x) x ## UL
            #define vm_ghost_has_VM_GHOST_UINT32_C 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT32_C
    #if vm_ghost_has(VM_GHOST_ULLONG_WIDTH)
        #if VM_GHOST_LLONG_WIDTH == 32
            #define VM_GHOST_UINT32_C(x) x ## ULL
            #define vm_ghost_has_VM_GHOST_UINT32_C 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_VM_GHOST_UINT32_C
    #define vm_ghost_has_VM_GHOST_UINT32_C 0
#endif

/**************************************
 * ghost/type/uint8_t/ghost_uint8_t.h
 **************************************/

#if VM_GHOST_DOCUMENTATION
typedef uint8_t vm_ghost_uint8_t;
#endif

#ifndef vm_ghost_has_vm_ghost_uint8_t
    #ifdef vm_ghost_uint8_t
        #define vm_ghost_has_vm_ghost_uint8_t 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_int8_t
    #if vm_ghost_has(vm_ghost_stdint_h) && defined(UINT8_MAX)
        typedef int8_t vm_ghost_int8_t;
        #define vm_ghost_has_vm_ghost_int8_t 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint8_t
    #ifdef __UINT8_TYPE__
        typedef __UINT8_TYPE__ vm_ghost_uint8_t;
        #define vm_ghost_has_vm_ghost_uint8_t 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint8_t
    #ifdef _MSC_VER
        typedef unsigned __int8 vm_ghost_uint8_t;
        #define vm_ghost_has_vm_ghost_uint8_t 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint8_t
    #if !VM_GHOST_IMPL_ASSUME_INTEGERS_HAVE_NO_PADDING_BITS
        #define vm_ghost_has_vm_ghost_uint8_t 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint8_t
    #if vm_ghost_has(VM_GHOST_UCHAR_WIDTH)
        #if VM_GHOST_UCHAR_WIDTH == 8
            typedef unsigned char vm_ghost_uint8_t;
            #define vm_ghost_has_vm_ghost_uint8_t 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint8_t
    #if vm_ghost_has(VM_GHOST_USHORT_WIDTH)
        #if VM_GHOST_USHORT_WIDTH == 8
            typedef unsigned short vm_ghost_uint8_t;
            #define vm_ghost_has_vm_ghost_uint8_t 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint8_t
    #if vm_ghost_has(VM_GHOST_UINT_WIDTH)
        #if VM_GHOST_UINT_WIDTH == 8
            typedef unsigned int vm_ghost_uint8_t;
            #define vm_ghost_has_vm_ghost_uint8_t 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint8_t
    #if vm_ghost_has(VM_GHOST_ULONG_WIDTH)
        #if VM_GHOST_ULONG_WIDTH == 8
            typedef unsigned long vm_ghost_uint8_t;
            #define vm_ghost_has_vm_ghost_uint8_t 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint8_t
    #if vm_ghost_has(VM_GHOST_ULLONG_WIDTH)
        #if VM_GHOST_ULLONG_WIDTH == 8
            typedef vm_ghost_ullong vm_ghost_uint8_t;
            #define vm_ghost_has_vm_ghost_uint8_t 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_uint8_t
    #define vm_ghost_has_vm_ghost_uint8_t 0
#endif

/**********************************
 * ghost/impl/ghost_impl_inline.h
 **********************************/

#if VM_GHOST_STATIC_DEFS
    #if vm_ghost_has_vm_ghost_inline
        #define vm_ghost_impl_inline vm_ghost_inline static
    #else
        #define vm_ghost_impl_inline static
    #endif
#elif VM_GHOST_MANUAL_DEFS
    #if VM_GHOST_EMIT_DEFS
        #define vm_ghost_impl_inline vm_ghost_emit_inline
    #else
        #define vm_ghost_impl_inline vm_ghost_noemit_inline
    #endif
#else
    #define vm_ghost_impl_inline vm_ghost_header_inline
#endif

/****************************************************
 * ghost/impl/malloc/ghost_impl_attrib_alloc_size.h
 ****************************************************/

#ifdef VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE
    #error
#endif

#ifndef VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE
    #if defined(__GNUC__) && !defined(__PGI)
        #if VM_GHOST_IMPL_STANDARD_ATTRIBUTE
            #define VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE(x) [[VM_GHOST_IMPL_ATTRIBUTE_NAMESPACE_GNU::__alloc_size__ x]]
        #endif
    #endif
#endif

#ifndef VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE
    #ifdef __clang__
        #if defined(__APPLE__) || __clang_major__ >= 4
            #define VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE(x) __attribute__((alloc_size x))
        #endif
    #endif
#endif

#ifndef VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE
    #ifdef __GNUC__
        #if VM_GHOST_GCC
            #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)
                #define VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE(x) __attribute__((alloc_size x))
            #endif
        #elif !defined(__clang__)
            #define VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE(x) __attribute__((alloc_size x))
        #endif
    #endif
#endif

#ifndef VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE
    #define VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE(x) 
#endif

/************************************************
 * ghost/impl/malloc/ghost_impl_attrib_malloc.h
 ************************************************/

#ifdef VM_GHOST_IMPL_ATTRIB_MALLOC
    #error
#endif

#ifndef VM_GHOST_IMPL_ATTRIB_MALLOC
    #if defined(__GNUC__) && !defined(__PGI)
        #if VM_GHOST_IMPL_STANDARD_ATTRIBUTE
            #define VM_GHOST_IMPL_ATTRIB_MALLOC [[VM_GHOST_IMPL_ATTRIBUTE_NAMESPACE_GNU::__malloc__]]
        #endif
    #endif
#endif

#ifndef VM_GHOST_IMPL_ATTRIB_MALLOC
    #ifdef __clang__
        #if defined(__APPLE__) || __clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ >= 1)
            #define VM_GHOST_IMPL_ATTRIB_MALLOC __attribute__((malloc))
        #endif
    #endif
#endif

#ifndef VM_GHOST_IMPL_ATTRIB_MALLOC
    #ifdef __GNUC__
        #define VM_GHOST_IMPL_ATTRIB_MALLOC __attribute__((malloc))
    #endif
#endif

#ifndef VM_GHOST_IMPL_ATTRIB_MALLOC
    #define VM_GHOST_IMPL_ATTRIB_MALLOC 
#endif

/********************************************
 * ghost/impl/malloc/ghost_impl_ex_calloc.h
 ********************************************/

#if defined(vm_ghost_has_vm_ghost_impl_ex_calloc) || defined(vm_ghost_impl_ex_calloc)
    #error "vm_ghost_impl_ex_calloc() cannot be overridden or disabled. Override vm_ghost_calloc() or define VM_GHOST_MALLOC_SYSTEM to 0 instead."
#endif

#ifdef vm_ghost_has_vm_ghost_calloc
    #if vm_ghost_has_vm_ghost_calloc
        #define vm_ghost_impl_ex_calloc vm_ghost_calloc
        #define vm_ghost_has_vm_ghost_impl_ex_calloc 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_calloc
    #if !VM_GHOST_MALLOC_SYSTEM
        #define vm_ghost_has_vm_ghost_impl_ex_calloc 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_calloc
    #if !VM_GHOST_HOSTED
        #define vm_ghost_has_vm_ghost_impl_ex_calloc 0
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_impl_ex_calloc
    #if !vm_ghost_has(vm_ghost_stdlib_h)
        #define vm_ghost_has_vm_ghost_impl_ex_calloc 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_calloc
    #ifdef _WIN32
        #ifdef __cplusplus
            #define vm_ghost_impl_ex_calloc ::calloc
        #else
            #define vm_ghost_impl_ex_calloc calloc
        #endif
        #define vm_ghost_has_vm_ghost_impl_ex_calloc 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_calloc
    #ifdef __linux__
        #if VM_GHOST_GLIBC || VM_GHOST_MUSL
            #ifdef __cplusplus
                #define vm_ghost_impl_ex_calloc ::calloc
            #else
                #define vm_ghost_impl_ex_calloc calloc
            #endif
            #define vm_ghost_has_vm_ghost_impl_ex_calloc 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_calloc
    #define vm_ghost_has_vm_ghost_impl_ex_calloc 0
#endif

/*************************************************
 * ghost/impl/malloc/ghost_impl_ex_calloc_0any.h
 *************************************************/

#if defined(vm_ghost_has_vm_ghost_impl_ex_calloc_0any) || defined(vm_ghost_impl_ex_calloc_0any)
    #error "vm_ghost_impl_ex_calloc_0any() cannot be overridden or disabled. Override vm_ghost_calloc_0any() or define VM_GHOST_MALLOC_SYSTEM to 0 instead."
#endif

#ifdef vm_ghost_has_vm_ghost_calloc_0any
    #if vm_ghost_has_vm_ghost_calloc_0any
        #define vm_ghost_impl_ex_calloc_0any vm_ghost_calloc_0any
        #define vm_ghost_has_vm_ghost_impl_ex_calloc_0any 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_calloc_0any
    #if !VM_GHOST_MALLOC_SYSTEM
        #define vm_ghost_has_vm_ghost_impl_ex_calloc_0any 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_calloc_0any
    #if !VM_GHOST_HOSTED
        #define vm_ghost_has_vm_ghost_impl_ex_calloc_0any 0
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_impl_ex_calloc_0any
    #if !vm_ghost_has(vm_ghost_stdlib_h)
        #define vm_ghost_has_vm_ghost_impl_ex_calloc_0any 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_calloc_0any
    #ifdef __cplusplus
        #define vm_ghost_impl_ex_calloc_0any ::calloc
    #else
        #define vm_ghost_impl_ex_calloc_0any calloc
    #endif
    #define vm_ghost_has_vm_ghost_impl_ex_calloc_0any 1
#endif

/**************************************************
 * ghost/impl/malloc/ghost_impl_ex_calloc_0null.h
 **************************************************/

#if defined(vm_ghost_has_vm_ghost_impl_ex_calloc_0null) || defined(vm_ghost_impl_ex_calloc_0null)
    #error "vm_ghost_impl_ex_calloc_0null() cannot be overridden or disabled. Override vm_ghost_calloc_0null() or define VM_GHOST_MALLOC_SYSTEM to 0 instead."
#endif

#ifdef vm_ghost_has_vm_ghost_calloc_0null
    #if vm_ghost_has_vm_ghost_calloc_0null
        #define vm_ghost_impl_ex_calloc_0null vm_ghost_calloc_0null
        #define vm_ghost_has_vm_ghost_impl_ex_calloc_0null 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_calloc_0null
    #define vm_ghost_has_vm_ghost_impl_ex_calloc_0null 0
#endif

/********************************************
 * ghost/impl/malloc/ghost_impl_ex_malloc.h
 ********************************************/

#if defined(vm_ghost_has_vm_ghost_impl_ex_malloc) || defined(vm_ghost_impl_ex_malloc)
    #error "vm_ghost_impl_ex_malloc() cannot be overridden or disabled. Override vm_ghost_malloc() or define VM_GHOST_MALLOC_SYSTEM=0 instead."
#endif

#ifdef vm_ghost_has_vm_ghost_malloc
    #if vm_ghost_has_vm_ghost_malloc
        #define vm_ghost_impl_ex_malloc vm_ghost_malloc
        #define vm_ghost_has_vm_ghost_impl_ex_malloc 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_malloc
    #if !VM_GHOST_MALLOC_SYSTEM
        #define vm_ghost_has_vm_ghost_impl_ex_malloc 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_malloc
    #if !VM_GHOST_HOSTED
        #define vm_ghost_has_vm_ghost_impl_ex_malloc 0
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_impl_ex_malloc
    #if !vm_ghost_has(vm_ghost_stdlib_h)
        #define vm_ghost_has_vm_ghost_impl_ex_malloc 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_malloc
    #ifdef _WIN32
        #ifdef __cplusplus
            #define vm_ghost_impl_ex_malloc ::malloc
        #else
            #define vm_ghost_impl_ex_malloc malloc
        #endif
        #define vm_ghost_has_vm_ghost_impl_ex_malloc 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_malloc
    #ifdef __linux__
        #if VM_GHOST_GLIBC || VM_GHOST_MUSL
            #ifdef __cplusplus
                #define vm_ghost_impl_ex_malloc ::malloc
            #else
                #define vm_ghost_impl_ex_malloc malloc
            #endif
            #define vm_ghost_has_vm_ghost_impl_ex_malloc 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_malloc
    #define vm_ghost_has_vm_ghost_impl_ex_malloc 0
#endif

/*************************************************
 * ghost/impl/malloc/ghost_impl_ex_malloc_0any.h
 *************************************************/

#if defined(vm_ghost_has_vm_ghost_impl_ex_malloc_0any) || defined(vm_ghost_impl_ex_malloc_0any)
    #error "vm_ghost_impl_ex_malloc_0any() cannot be overridden or disabled. Override vm_ghost_malloc_0any() or define VM_GHOST_MALLOC_SYSTEM to 0 instead."
#endif

#ifdef vm_ghost_has_vm_ghost_malloc_0any
    #if vm_ghost_has_vm_ghost_malloc_0any
        #define vm_ghost_impl_ex_malloc_0any vm_ghost_malloc_0any
        #define vm_ghost_has_vm_ghost_impl_ex_malloc_0any 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_malloc_0any
    #if !VM_GHOST_MALLOC_SYSTEM
        #define vm_ghost_has_vm_ghost_impl_ex_malloc_0any 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_malloc_0any
    #if !VM_GHOST_HOSTED
        #define vm_ghost_has_vm_ghost_impl_ex_malloc_0any 0
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_impl_ex_malloc_0any
    #if !vm_ghost_has(vm_ghost_stdlib_h)
        #define vm_ghost_has_vm_ghost_impl_ex_malloc_0any 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_malloc_0any
    #ifdef __cplusplus
        #define vm_ghost_impl_ex_malloc_0any ::malloc
    #else
        #define vm_ghost_impl_ex_malloc_0any malloc
    #endif
    #define vm_ghost_has_vm_ghost_impl_ex_malloc_0any 1
#endif

/**************************************************
 * ghost/impl/malloc/ghost_impl_ex_malloc_0null.h
 **************************************************/

#if defined(vm_ghost_has_vm_ghost_impl_ex_malloc_0null) || defined(vm_ghost_impl_ex_malloc_0null)
    #error "vm_ghost_impl_ex_malloc_0null() cannot be overridden or disabled. Override vm_ghost_malloc_0null() or define VM_GHOST_MALLOC_SYSTEM to 0 instead."
#endif

#ifdef vm_ghost_has_vm_ghost_malloc_0null
    #if vm_ghost_has_vm_ghost_malloc_0null
        #define vm_ghost_impl_ex_malloc_0null vm_ghost_malloc_0null
        #define vm_ghost_has_vm_ghost_impl_ex_malloc_0null 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_malloc_0null
    #define vm_ghost_has_vm_ghost_impl_ex_malloc_0null 0
#endif

/*********************************************
 * ghost/impl/malloc/ghost_impl_ex_realloc.h
 *********************************************/

#if defined(vm_ghost_has_vm_ghost_impl_ex_realloc) || defined(vm_ghost_impl_ex_realloc)
    #error "vm_ghost_impl_ex_realloc() cannot be overridden or disabled. Override vm_ghost_realloc() or define VM_GHOST_MALLOC_SYSTEM=0 instead."
#endif

#ifdef vm_ghost_has_vm_ghost_realloc
    #if vm_ghost_has_vm_ghost_realloc
        #define vm_ghost_impl_ex_realloc vm_ghost_realloc
        #define vm_ghost_has_vm_ghost_impl_ex_realloc 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc
    #define vm_ghost_has_vm_ghost_impl_ex_realloc 0
#endif

/****************************************************
 * ghost/impl/malloc/ghost_impl_ex_realloc_0alloc.h
 ****************************************************/

#if defined(vm_ghost_has_vm_ghost_impl_ex_realloc_0alloc) || defined(vm_ghost_impl_ex_realloc_0alloc)
    #error "vm_ghost_impl_ex_realloc_0alloc() cannot be overridden or disabled. Override vm_ghost_realloc_0alloc() or define VM_GHOST_MALLOC_SYSTEM=0 instead."
#endif

#ifdef vm_ghost_has_vm_ghost_realloc_0alloc
    #if vm_ghost_has_vm_ghost_realloc_0alloc
        #define vm_ghost_impl_ex_realloc_0alloc vm_ghost_realloc_0alloc
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0alloc 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0alloc
    #if !VM_GHOST_MALLOC_SYSTEM
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0alloc 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0alloc
    #if !VM_GHOST_HOSTED
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0alloc 0
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0alloc
    #if !vm_ghost_has(vm_ghost_stdlib_h)
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0alloc 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0alloc
    #if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || \
            defined(__OpenBSD__) || defined(__DragonFly__)
        #ifdef __cplusplus
            #define vm_ghost_impl_ex_realloc_0alloc ::realloc
        #else
            #define vm_ghost_impl_ex_realloc_0alloc realloc
        #endif
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0alloc 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0alloc
    #define vm_ghost_has_vm_ghost_impl_ex_realloc_0alloc 0
#endif

/**************************************************
 * ghost/impl/malloc/ghost_impl_ex_realloc_0any.h
 **************************************************/

#if defined(vm_ghost_has_vm_ghost_impl_ex_realloc_0any) || defined(vm_ghost_impl_ex_realloc_0any)
    #error "vm_ghost_impl_ex_realloc_0any() cannot be overridden or disabled. Override vm_ghost_realloc_0any() or define VM_GHOST_MALLOC_SYSTEM=0 instead."
#endif

#ifdef vm_ghost_has_vm_ghost_realloc_0any
    #if vm_ghost_has_vm_ghost_realloc_0any
        #define vm_ghost_impl_ex_realloc_0any vm_ghost_realloc_0any
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0any 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0any
    #if !VM_GHOST_MALLOC_SYSTEM
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0any 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0any
    #if !VM_GHOST_HOSTED
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0any 0
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0any
    #if !vm_ghost_has(vm_ghost_stdlib_h)
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0any 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0any
    #ifdef __cplusplus
        #define vm_ghost_impl_ex_realloc_0any ::realloc
    #else
        #define vm_ghost_impl_ex_realloc_0any realloc
    #endif
    #define vm_ghost_has_vm_ghost_impl_ex_realloc_0any 1
#endif

/***************************************************
 * ghost/impl/malloc/ghost_impl_ex_realloc_0free.h
 ***************************************************/

#if defined(vm_ghost_has_vm_ghost_impl_ex_realloc_0free) || defined(vm_ghost_impl_ex_realloc_0free)
    #error "vm_ghost_impl_ex_realloc_0free() cannot be overridden or disabled. Override vm_ghost_realloc_0free() or define VM_GHOST_MALLOC_SYSTEM=0 instead."
#endif

#ifdef vm_ghost_has_vm_ghost_realloc_0free
    #if vm_ghost_has_vm_ghost_realloc_0free
        #define vm_ghost_impl_ex_realloc_0free vm_ghost_realloc_0free
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0free 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0free
    #if !VM_GHOST_MALLOC_SYSTEM
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0free 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0free
    #if !VM_GHOST_HOSTED
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0free 0
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0free
    #if !vm_ghost_has(vm_ghost_stdlib_h)
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0free 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0free
    #ifdef _WIN32
        #ifdef __cplusplus
            #define vm_ghost_impl_ex_realloc_0free ::realloc
        #else
            #define vm_ghost_impl_ex_realloc_0free realloc
        #endif
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0free 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0free
    #ifdef __linux__
        #if VM_GHOST_GLIBC
            #ifdef __cplusplus
                #define vm_ghost_impl_ex_realloc_0free ::realloc
            #else
                #define vm_ghost_impl_ex_realloc_0free realloc
            #endif
            #define vm_ghost_has_vm_ghost_impl_ex_realloc_0free 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0free
    #define vm_ghost_has_vm_ghost_impl_ex_realloc_0free 0
#endif

/***************************************************
 * ghost/impl/malloc/ghost_impl_ex_realloc_0null.h
 ***************************************************/

#if defined(vm_ghost_has_vm_ghost_impl_ex_realloc_0null) || defined(vm_ghost_impl_ex_realloc_0null)
    #error "vm_ghost_impl_ex_realloc_0null() cannot be overridden or disabled. Override vm_ghost_realloc_0null() or define VM_GHOST_MALLOC_SYSTEM to 0 instead."
#endif

#ifdef vm_ghost_has_vm_ghost_realloc_0null
    #if vm_ghost_has_vm_ghost_realloc_0null
        #define vm_ghost_impl_ex_realloc_0null vm_ghost_realloc_0null
        #define vm_ghost_has_vm_ghost_impl_ex_realloc_0null 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_impl_ex_realloc_0null
    #define vm_ghost_has_vm_ghost_impl_ex_realloc_0null 0
#endif

/************************************
 * ghost/malloc/ghost_malloc_0any.h
 ************************************/

#if VM_GHOST_DOCUMENTATION
void* vm_ghost_malloc_0any(size_t size);
#endif

#ifndef vm_ghost_has_vm_ghost_malloc_0any
    #if vm_ghost_has(vm_ghost_impl_ex_malloc_0any)
        #define vm_ghost_malloc_0any(x) vm_ghost_auto_cast(vm_ghost_impl_ex_malloc_0any(x))
        #define vm_ghost_has_vm_ghost_malloc_0any 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_malloc_0any
    #if vm_ghost_has(vm_ghost_impl_ex_malloc)
        #define vm_ghost_malloc_0any(x) vm_ghost_auto_cast(vm_ghost_impl_ex_malloc(x))
        #define vm_ghost_has_vm_ghost_malloc_0any 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_malloc_0any
    #if vm_ghost_has(vm_ghost_impl_ex_malloc_0null)
        #define vm_ghost_malloc_0any(x) vm_ghost_auto_cast(vm_ghost_impl_ex_malloc_0null(x))
        #define vm_ghost_has_vm_ghost_malloc_0any 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_malloc_0any
    #if \
            vm_ghost_has(vm_ghost_impl_ex_realloc_0any) || \
            vm_ghost_has(vm_ghost_impl_ex_realloc) || \
            vm_ghost_has(vm_ghost_impl_ex_realloc_0alloc) || \
            vm_ghost_has(vm_ghost_impl_ex_realloc_0free) || \
            vm_ghost_has(vm_ghost_impl_ex_realloc_0null) || \
            \
            vm_ghost_has(vm_ghost_impl_ex_calloc_0any) || \
            vm_ghost_has(vm_ghost_impl_ex_calloc) || \
            vm_ghost_has(vm_ghost_impl_ex_calloc_0null)

        VM_GHOST_IMPL_FUNCTION_OPEN

        VM_GHOST_IMPL_ATTRIB_MALLOC
        VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE((1))
        vm_ghost_impl_inline
        void* vm_ghost_malloc_0any(vm_ghost_size_t vm_ghost_impl_v_size) {
            #if vm_ghost_has(vm_ghost_impl_ex_realloc_0any)
                return vm_ghost_impl_ex_realloc_0any(vm_ghost_null, vm_ghost_impl_v_size);
            #elif vm_ghost_has(vm_ghost_impl_ex_realloc)
                return vm_ghost_impl_ex_realloc(vm_ghost_null, vm_ghost_impl_v_size);
            #elif vm_ghost_has(vm_ghost_impl_ex_realloc_0alloc)
                return vm_ghost_impl_ex_realloc_0alloc(vm_ghost_null, vm_ghost_impl_v_size);
            #elif vm_ghost_has(vm_ghost_impl_ex_realloc_0free)
                return vm_ghost_impl_ex_realloc_0free(vm_ghost_null, vm_ghost_impl_v_size);
            #elif vm_ghost_has(vm_ghost_impl_ex_realloc_0null)
                return vm_ghost_impl_ex_realloc_0null(vm_ghost_null, vm_ghost_impl_v_size);
            #elif vm_ghost_has(vm_ghost_impl_ex_calloc_0any)
                return vm_ghost_impl_ex_calloc_0any(1, vm_ghost_impl_v_size);
            #elif vm_ghost_has(vm_ghost_impl_ex_calloc)
                return vm_ghost_impl_ex_calloc(1, vm_ghost_impl_v_size);
            #elif vm_ghost_has(vm_ghost_impl_ex_calloc_0null)
                return vm_ghost_impl_ex_calloc_0null(1, vm_ghost_impl_v_size);
            #else
                #error
            #endif
        }

        VM_GHOST_IMPL_FUNCTION_CLOSE

        #define vm_ghost_malloc_0any(x) vm_ghost_auto_cast(vm_ghost_malloc_0any(x))
        #define vm_ghost_has_vm_ghost_malloc_0any 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_malloc_0any
    #define vm_ghost_has_vm_ghost_malloc_0any 0
#endif

/*******************************
 * ghost/malloc/ghost_malloc.h
 *******************************/

#if VM_GHOST_DOCUMENTATION
#define vm_ghost_has_vm_ghost_malloc 1

void* vm_ghost_malloc(size_t size);

#endif

#ifndef vm_ghost_has_vm_ghost_malloc
    #if vm_ghost_has(vm_ghost_impl_ex_malloc)
        #define vm_ghost_malloc(x) vm_ghost_auto_cast(vm_ghost_impl_ex_malloc(x))
        #define vm_ghost_has_vm_ghost_malloc 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_malloc
    #if vm_ghost_has(vm_ghost_malloc_0any)
        VM_GHOST_IMPL_FUNCTION_OPEN

        VM_GHOST_IMPL_ATTRIB_MALLOC
        VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE((1))
        vm_ghost_impl_inline
        void* vm_ghost_malloc(vm_ghost_size_t vm_ghost_impl_v_size) {
            if (vm_ghost_impl_v_size == 0)
                vm_ghost_impl_v_size = 1;
            return vm_ghost_malloc_0any(vm_ghost_impl_v_size);
        }

        VM_GHOST_IMPL_FUNCTION_CLOSE

        #define vm_ghost_malloc(x) vm_ghost_auto_cast(vm_ghost_malloc(x))
        #define vm_ghost_has_vm_ghost_malloc 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_malloc
    #define vm_ghost_has_vm_ghost_malloc 0
#endif

/*********************************************************
 * ghost/impl/math/ghost_impl_has_builtin_mul_overflow.h
 *********************************************************/

#ifdef VM_GHOST_IMPL_HAS_BUILTIN_MUL_OVERFLOW
    #error "You cannot pre-define VM_GHOST_IMPL_ macros."
#endif

#ifndef VM_GHOST_IMPL_HAS_BUILTIN_MUL_OVERFLOW
    #if vm_ghost_has(vm_ghost_has_builtin)
        #if __has_builtin(__builtin_mul_overflow)
            #define VM_GHOST_IMPL_HAS_BUILTIN_MUL_OVERFLOW 1
        #else
            #define VM_GHOST_IMPL_HAS_BUILTIN_MUL_OVERFLOW 0
        #endif
    #endif
#endif

#ifndef VM_GHOST_IMPL_HAS_BUILTIN_MUL_OVERFLOW
    #ifdef __GNUC__
        #if VM_GHOST_GCC
            #if __GNUC__ >= 5
                #define VM_GHOST_IMPL_HAS_BUILTIN_MUL_OVERFLOW 1
            #endif
        #endif
    #endif
#endif

#ifndef VM_GHOST_IMPL_HAS_BUILTIN_MUL_OVERFLOW
    #define VM_GHOST_IMPL_HAS_BUILTIN_MUL_OVERFLOW 0
#endif

/************************************************
 * ghost/math/checked_mul/ghost_checked_mul_z.h
 ************************************************/

#if VM_GHOST_DOCUMENTATION
bool vm_ghost_checked_add_z(size_t* out, size_t a, size_t b);
#endif

#ifndef vm_ghost_has_vm_ghost_checked_add_z
    #ifdef vm_ghost_checked_add_z
        #define vm_ghost_has_vm_ghost_checked_add_z 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_checked_add_z
    #if !vm_ghost_has(vm_ghost_size_t)
        #define vm_ghost_has_vm_ghost_checked_add_z 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_checked_mul_z
    #ifdef __STDC_VERSION__
        #if VM_GHOST_STDC_VERSION > 202000L && VM_GHOST_IMPL_ENABLE_C23
            #ifdef ckd_mul
                VM_GHOST_IMPL_FUNCTION_OPEN
                vm_ghost_impl_always_inline
                vm_ghost_bool vm_ghost_checked_mul_z(
                        vm_ghost_size_t* vm_ghost_impl_v_out,
                        vm_ghost_size_t vm_ghost_impl_v_a,
                        vm_ghost_size_t vm_ghost_impl_v_b)
                {
                    return ckd_mul(
                            vm_ghost_impl_v_out,
                            vm_ghost_impl_v_a,
                            vm_ghost_impl_v_b);
                }
                VM_GHOST_IMPL_FUNCTION_CLOSE
                #define vm_ghost_has_vm_ghost_checked_mul_z 1
            #endif
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_checked_mul_z
    #if VM_GHOST_IMPL_HAS_BUILTIN_MUL_OVERFLOW
        VM_GHOST_IMPL_FUNCTION_OPEN
        vm_ghost_impl_always_inline
        vm_ghost_bool vm_ghost_checked_mul_z(
                vm_ghost_size_t* vm_ghost_impl_v_out,
                vm_ghost_size_t vm_ghost_impl_v_a,
                vm_ghost_size_t vm_ghost_impl_v_b)
        {
            return __builtin_mul_overflow(
                    vm_ghost_impl_v_a,
                    vm_ghost_impl_v_b,
                    vm_ghost_impl_v_out);
        }
        VM_GHOST_IMPL_FUNCTION_CLOSE
        #define vm_ghost_has_vm_ghost_checked_mul_z 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_checked_mul_z
    #if VM_GHOST_IMPL_ASSUME_8_BIT_CHAR
        VM_GHOST_IMPL_FUNCTION_OPEN
        vm_ghost_impl_inline
        vm_ghost_bool vm_ghost_checked_mul_z(
                vm_ghost_size_t* vm_ghost_impl_v_out,
                vm_ghost_size_t vm_ghost_impl_v_a,
                vm_ghost_size_t vm_ghost_impl_v_b)
        {
            vm_ghost_size_t vm_ghost_impl_v_large = (vm_ghost_static_cast(vm_ghost_size_t, 1)
                    << vm_ghost_static_cast(vm_ghost_size_t, (sizeof(vm_ghost_size_t) * 4)));

            *vm_ghost_impl_v_out = vm_ghost_impl_v_a * vm_ghost_impl_v_b;

            return (vm_ghost_impl_v_a >= vm_ghost_impl_v_large
                        || vm_ghost_impl_v_b >= vm_ghost_impl_v_large)
                    && vm_ghost_impl_v_a != 0
                    && *vm_ghost_impl_v_out / vm_ghost_impl_v_a != vm_ghost_impl_v_b;
        }
        VM_GHOST_IMPL_FUNCTION_CLOSE
        #define vm_ghost_has_vm_ghost_checked_mul_z 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_checked_mul_z
    #define vm_ghost_has_vm_ghost_checked_mul_z 0
#endif

/*****************************
 * ghost/error/ghost_errno.h
 *****************************/

#ifndef vm_ghost_has_vm_ghost_errno
    #ifdef vm_ghost_errno
        #define vm_ghost_has_vm_ghost_errno 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_errno
    #if vm_ghost_has(vm_ghost_errno_h)
        #define vm_ghost_errno errno
        #define vm_ghost_has_vm_ghost_errno 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_errno
    #define vm_ghost_has_vm_ghost_errno 0
#endif

/***********************************
 * ghost/error/ghost_errno_store.h
 ***********************************/

#ifndef vm_ghost_has_vm_ghost_errno_store
    #ifdef vm_ghost_errno_store
        #define vm_ghost_has_vm_ghost_errno_store 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_errno_store
    #if vm_ghost_has(vm_ghost_errno)
        #define vm_ghost_errno_store(x) vm_ghost_discard(vm_ghost_errno = (x))
    #else
        #define vm_ghost_errno_store(x) vm_ghost_discard(0)
    #endif
    #define vm_ghost_has_vm_ghost_errno_store 1
#endif

/************************************
 * ghost/impl/ghost_impl_function.h
 ************************************/

#if VM_GHOST_STATIC_DEFS
    #define vm_ghost_impl_function vm_ghost_maybe_unused static
#elif VM_GHOST_MANUAL_DEFS
    #define vm_ghost_impl_function 
#else
    #define vm_ghost_impl_function vm_ghost_header_function
#endif

/*******************************
 * ghost/string/ghost_memset.h
 *******************************/

#if VM_GHOST_DOCUMENTATION
void* vm_ghost_memset(void* dest, int c, size_t count);
#endif

#ifndef vm_ghost_has_vm_ghost_memset
    #ifdef vm_ghost_memset
        #define vm_ghost_has_vm_ghost_memset 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_memset
    #if vm_ghost_has(vm_ghost_has_builtin)
        #if __has_builtin(__builtin_memset)
            #define vm_ghost_memset __builtin_memset
            #define vm_ghost_has_vm_ghost_memset 1
        #endif
    #else
        #if VM_GHOST_GCC
            #define vm_ghost_memset __builtin_memset
            #define vm_ghost_has_vm_ghost_memset 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_memset
    #if VM_GHOST_HOSTED
        #if vm_ghost_has_vm_ghost_string_h
            #define vm_ghost_memset memset
            #define vm_ghost_has_vm_ghost_memset 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_memset
    VM_GHOST_IMPL_FUNCTION_OPEN
    vm_ghost_impl_function
    void* vm_ghost_memset(void* vdest, int c, vm_ghost_size_t count) VM_GHOST_IMPL_DEF({
        unsigned char* vm_ghost_restrict dest = vm_ghost_static_cast(unsigned char*, vdest);
        unsigned char* start = dest;
        unsigned char* end = dest + count;
        unsigned char uc = vm_ghost_static_cast(unsigned char, c);
        while (dest != end)
            *dest++ = uc;
        return start;
    })
    VM_GHOST_IMPL_FUNCTION_CLOSE
    #define vm_ghost_has_vm_ghost_memset 1
#endif

/******************************
 * ghost/string/ghost_bzero.h
 ******************************/

#if VM_GHOST_DOCUMENTATION
void vm_ghost_bzero(void* p, size_t count);
#endif

#ifndef vm_ghost_has_vm_ghost_bzero
    #ifdef vm_ghost_bzero
        #define vm_ghost_has_vm_ghost_bzero 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_bzero
    #define vm_ghost_bzero(s, n) vm_ghost_memset((s), 0, (n))
    #define vm_ghost_has_vm_ghost_bzero 1
#endif

/*******************************
 * ghost/string/ghost_memcpy.h
 *******************************/

#if VM_GHOST_DOCUMENTATION
void* vm_ghost_memcpy(const void* restrict dest, void* restrict src, size_t count);
#endif

#ifndef vm_ghost_has_vm_ghost_memcpy
    #ifdef vm_ghost_memcpy
        #define vm_ghost_has_vm_ghost_memcpy 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_memcpy
    #if vm_ghost_has(vm_ghost_has_builtin)
        #if __has_builtin(__builtin_memcpy)
            #define vm_ghost_memcpy __builtin_memcpy
            #define vm_ghost_has_vm_ghost_memcpy 1
        #endif
    #else
        #if VM_GHOST_GCC
            #define vm_ghost_memcpy __builtin_memcpy
            #define vm_ghost_has_vm_ghost_memcpy 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_memcpy
    #if vm_ghost_has(vm_ghost_string_h)
        #define vm_ghost_memcpy memcpy
        #define vm_ghost_has_vm_ghost_memcpy 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_memcpy
    VM_GHOST_IMPL_FUNCTION_OPEN
    vm_ghost_impl_function
    void* vm_ghost_memcpy(void* vm_ghost_restrict vdest, const void* vm_ghost_restrict vsrc, vm_ghost_size_t count) VM_GHOST_IMPL_DEF({
        const unsigned char* vm_ghost_restrict src = vm_ghost_static_cast(const unsigned char*, vsrc);
        unsigned char* dest = vm_ghost_static_cast(unsigned char*, vdest);
        unsigned char* end = dest + count;
        while (dest != end)
            *dest++ = *src++;
        return vdest;
    })
    VM_GHOST_IMPL_FUNCTION_CLOSE
    #define vm_ghost_has_vm_ghost_memcpy 1
#endif

/*******************************
 * ghost/string/ghost_strlen.h
 *******************************/

#if VM_GHOST_DOCUMENTATION
size_t vm_ghost_strlen(const char* s);
#endif

#ifndef vm_ghost_has_vm_ghost_strlen
    #ifdef vm_ghost_strlen
        #define vm_ghost_has_vm_ghost_strlen 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_strlen
    #if vm_ghost_has(vm_ghost_has_builtin)
        #if __has_builtin(__builtin_strlen)
            #define vm_ghost_strlen __builtin_strlen
            #define vm_ghost_has_vm_ghost_strlen 1
        #endif
    #else
        #if VM_GHOST_GCC
            #define vm_ghost_strlen __builtin_strlen
            #define vm_ghost_has_vm_ghost_strlen 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_strlen
    #if vm_ghost_has(vm_ghost_string_h)
        #define vm_ghost_strlen strlen
        #define vm_ghost_has_vm_ghost_strlen 1
    #endif
#endif

#ifndef vm_ghost_strlen
    VM_GHOST_IMPL_FUNCTION_OPEN
    vm_ghost_impl_function
    vm_ghost_size_t vm_ghost_strlen(const char* s) VM_GHOST_IMPL_DEF({
        const char* end = s;
        while (*end != '\000')
            ++end;
        return vm_ghost_static_cast(vm_ghost_size_t, end - s);
    })
    VM_GHOST_IMPL_FUNCTION_CLOSE
    #define vm_ghost_has_vm_ghost_strlen 1
#endif

/***********************************************
 * ghost/serialization/bswap/ghost_bswap_u32.h
 ***********************************************/

#if VM_GHOST_DOCUMENTATION
uint32_t vm_ghost_bswap_u32(uint32_t u);
#endif

#ifndef vm_ghost_has_vm_ghost_bswap_u32
    #ifdef vm_ghost_bswap_u32
        #define vm_ghost_has_vm_ghost_bswap_u32 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_bswap_u32
    #if vm_ghost_has(vm_ghost_has_builtin)
        #if __has_builtin(__builtin_bswap32)
            #define vm_ghost_bswap_u32 __builtin_bswap32
            #define vm_ghost_has_vm_ghost_bswap_u32 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_bswap_u32
    #if defined(_MSC_VER) && defined(_WIN32)
        #include <stdlib.h>
        #define vm_ghost_bswap_u32 _byteswap_ulong
        #define vm_ghost_has_vm_ghost_bswap_u32 1
    #endif
#endif

/* TODO extend this to all others as well */
#ifndef vm_ghost_has_vm_ghost_bswap_u32
    #if !vm_ghost_has(VM_GHOST_UINT32_C)
        #define vm_ghost_has_vm_ghost_bswap_u32 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_bswap_u32
    VM_GHOST_IMPL_FUNCTION_OPEN
    vm_ghost_impl_inline
    vm_ghost_uint32_t vm_ghost_bswap_u32(vm_ghost_uint32_t u) {
        return vm_ghost_static_cast(vm_ghost_uint32_t,
                ((u & VM_GHOST_UINT32_C(0x000000FF)) << 24) |
                ((u & VM_GHOST_UINT32_C(0x0000FF00)) << 8) |
                ((u & VM_GHOST_UINT32_C(0x00FF0000)) >> 8) |
                ((u & VM_GHOST_UINT32_C(0xFF000000)) >> 24));
    }
    VM_GHOST_IMPL_FUNCTION_CLOSE
    #define vm_ghost_has_vm_ghost_bswap_u32 1
#endif

/************************************
 * ghost/malloc/ghost_calloc_0any.h
 ************************************/

#if VM_GHOST_DOCUMENTATION
void* vm_ghost_calloc_0any(size_t count, size_t element_size);
#endif

#ifndef vm_ghost_has_vm_ghost_calloc_0any
    #if vm_ghost_has(vm_ghost_impl_ex_calloc_0any)
        #define vm_ghost_calloc_0any(n, z) vm_ghost_auto_cast(vm_ghost_impl_ex_calloc_0any(n, z))
        #define vm_ghost_has_vm_ghost_calloc_0any 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_calloc_0any
    #if vm_ghost_has(vm_ghost_impl_ex_calloc)
        #define vm_ghost_calloc_0any(p, n) vm_ghost_auto_cast(vm_ghost_impl_ex_calloc(p, n))
        #define vm_ghost_has_vm_ghost_calloc_0any 1
    #endif
#endif
#ifndef vm_ghost_has_vm_ghost_calloc_0any
    #if vm_ghost_has(vm_ghost_impl_ex_calloc_0null)
        #define vm_ghost_calloc_0any(p, n) vm_ghost_auto_cast(vm_ghost_impl_ex_calloc_0null(p, n))
        #define vm_ghost_has_vm_ghost_calloc_0any 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_calloc_0any
    #if vm_ghost_has(vm_ghost_malloc_0any)
        VM_GHOST_IMPL_FUNCTION_OPEN

        VM_GHOST_IMPL_ATTRIB_MALLOC
        VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE((1, 2))
        vm_ghost_impl_inline
        void* vm_ghost_calloc_0any(vm_ghost_size_t vm_ghost_impl_v_count, vm_ghost_size_t vm_ghost_impl_v_stride) {
            void* vm_ghost_impl_v_ret;
            vm_ghost_size_t vm_ghost_impl_v_total;

            if (vm_ghost_expect_false(vm_ghost_checked_mul_z(&vm_ghost_impl_v_total, vm_ghost_impl_v_count, vm_ghost_impl_v_stride))) {
                vm_ghost_errno_store(EINVAL);
                return vm_ghost_null;
            }

            vm_ghost_impl_v_ret = vm_ghost_malloc_0any(vm_ghost_impl_v_total);
            if (vm_ghost_expect_true(vm_ghost_impl_v_ret != vm_ghost_null))
                vm_ghost_bzero(vm_ghost_impl_v_ret, vm_ghost_impl_v_total);
            return vm_ghost_impl_v_ret;
        }

        VM_GHOST_IMPL_FUNCTION_CLOSE

        #define vm_ghost_calloc_0any(n, z) vm_ghost_auto_cast(vm_ghost_calloc_0any(n, z))
        #define vm_ghost_has_vm_ghost_calloc_0any 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_calloc_0any
    #define vm_ghost_has_vm_ghost_calloc_0any 0
#endif

/*******************************
 * ghost/malloc/ghost_calloc.h
 *******************************/

#if VM_GHOST_DOCUMENTATION
void* vm_ghost_calloc(size_t count, size_t element_size);
#endif

#ifndef vm_ghost_has_vm_ghost_calloc
    #if vm_ghost_has(vm_ghost_impl_ex_calloc)
        #define vm_ghost_calloc(n, z) vm_ghost_auto_cast(vm_ghost_impl_ex_calloc(n, z))
        #define vm_ghost_has_vm_ghost_calloc 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_calloc
    #if vm_ghost_has(vm_ghost_calloc_0any)
        VM_GHOST_IMPL_FUNCTION_OPEN

        VM_GHOST_IMPL_ATTRIB_MALLOC
        VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE((1, 2))
        vm_ghost_impl_inline
        void* vm_ghost_calloc(vm_ghost_size_t vm_ghost_impl_v_count, vm_ghost_size_t vm_ghost_impl_v_stride) {
            if (vm_ghost_impl_v_count == 0 || vm_ghost_impl_v_stride == 0) {
                vm_ghost_impl_v_count = 1;
                vm_ghost_impl_v_stride = 1;
            }
            return vm_ghost_calloc_0any(vm_ghost_impl_v_count, vm_ghost_impl_v_stride);
        }

        VM_GHOST_IMPL_FUNCTION_CLOSE

        #define vm_ghost_calloc(n, z) vm_ghost_auto_cast(vm_ghost_calloc(n, z))
        #define vm_ghost_has_vm_ghost_calloc 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_calloc
    #define vm_ghost_has_vm_ghost_calloc 0
#endif

/************************************************
 * ghost/impl/serialization/ghost_impl_endian.h
 ************************************************/

#ifdef vm_ghost_has_VM_GHOST_BIG_ENDIAN
    #error "Don't define vm_ghost_has_VM_GHOST_BIG_ENDIAN; only define VM_GHOST_BIG_ENDIAN."
#endif
#ifdef vm_ghost_has_VM_GHOST_LITTLE_ENDIAN
    #error "Don't define vm_ghost_has_VM_GHOST_LITTLE_ENDIAN; only define VM_GHOST_LITTLE_ENDIAN."
#endif

#if defined(VM_GHOST_LITTLE_ENDIAN) && !defined(VM_GHOST_BIG_ENDIAN)
    #if VM_GHOST_LITTLE_ENDIAN
        #define VM_GHOST_BIG_ENDIAN 0
    #endif
#endif
#if defined(VM_GHOST_BIG_ENDIAN) && !defined(VM_GHOST_LITTLE_ENDIAN)
    #if VM_GHOST_BIG_ENDIAN
        #define VM_GHOST_LITTLE_ENDIAN 0
    #endif
#endif

#ifdef _WIN32
    #if defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64)
        #ifndef VM_GHOST_LITTLE_ENDIAN
            #define VM_GHOST_LITTLE_ENDIAN 1
        #endif
        #ifndef VM_GHOST_BIG_ENDIAN
            #define VM_GHOST_BIG_ENDIAN 0
        #endif
    #endif
#endif

#ifdef __BYTE_ORDER__
    #ifdef __ORDER_LITTLE_ENDIAN__
        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            #ifndef VM_GHOST_LITTLE_ENDIAN
                #define VM_GHOST_LITTLE_ENDIAN 1
            #endif
            #ifndef VM_GHOST_BIG_ENDIAN
                #define VM_GHOST_BIG_ENDIAN 0
            #endif
        #endif
    #endif
    #ifdef __ORDER_BIG_ENDIAN__
        #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            #ifndef VM_GHOST_BIG_ENDIAN
                #define VM_GHOST_BIG_ENDIAN 1
            #endif
            #ifndef VM_GHOST_LITTLE_ENDIAN
                #define VM_GHOST_LITTLE_ENDIAN 0
            #endif
        #endif
    #endif
#endif

#if defined(__GLIBC__) && defined(__BYTE_ORDER)
    #ifdef __LITTLE_ENDIAN
        #if __BYTE_ORDER == __LITTLE_ENDIAN
            #ifndef VM_GHOST_LITTLE_ENDIAN
                #define VM_GHOST_LITTLE_ENDIAN 1
            #endif
            #ifndef VM_GHOST_BIG_ENDIAN
                #define VM_GHOST_BIG_ENDIAN 0
            #endif
        #endif
    #endif
    #ifdef __BIG_ENDIAN
        #if __BYTE_ORDER == __BIG_ENDIAN
            #ifndef VM_GHOST_BIG_ENDIAN
                #define VM_GHOST_BIG_ENDIAN 1
            #endif
            #ifndef VM_GHOST_LITTLE_ENDIAN
                #define VM_GHOST_LITTLE_ENDIAN 0
            #endif
        #endif
    #endif
#endif

#if (defined(__LITTLE_ENDIAN__) || defined(__LITTLEENDIAN__)) && \
        !defined(__BIG_ENDIAN__) && !defined(__BIGENDIAN__)
    #ifndef VM_GHOST_LITTLE_ENDIAN
        #define VM_GHOST_LITTLE_ENDIAN 1
    #endif
    #ifndef VM_GHOST_BIG_ENDIAN
        #define VM_GHOST_BIG_ENDIAN 0
    #endif
#endif
#if (defined(__BIG_ENDIAN__) || defined(__BIGENDIAN__)) && \
        !defined(__LITTLE_ENDIAN__) && !defined(__LITTLEENDIAN__)
    #ifndef VM_GHOST_BIG_ENDIAN
        #define VM_GHOST_BIG_ENDIAN 1
    #endif
    #ifndef VM_GHOST_LITTLE_ENDIAN
        #define VM_GHOST_LITTLE_ENDIAN 0
    #endif
#endif

#if VM_GHOST_X86_32 || VM_GHOST_X86_64 || \
        defined(__ARMEL__) || defined(__THUMBEL__) || defined(__AARCH64EL__) || \
        defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
    #ifndef VM_GHOST_LITTLE_ENDIAN
        #define VM_GHOST_LITTLE_ENDIAN 1
    #endif
    #ifndef VM_GHOST_BIG_ENDIAN
        #define VM_GHOST_BIG_ENDIAN 0
    #endif
#endif
#if defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || \
        defined(_MIPSEB) || defined(__MIPSEB) || defined(__MIPSEB__)
    #ifndef VM_GHOST_BIG_ENDIAN
        #define VM_GHOST_BIG_ENDIAN 1
    #endif
    #ifndef VM_GHOST_LITTLE_ENDIAN
        #define VM_GHOST_LITTLE_ENDIAN 0
    #endif
#endif

#ifndef VM_GHOST_LITTLE_ENDIAN
    #define VM_GHOST_LITTLE_ENDIAN 0
#endif
#ifndef VM_GHOST_BIG_ENDIAN
    #define VM_GHOST_BIG_ENDIAN 0
#endif

#if VM_GHOST_BIG_ENDIAN && VM_GHOST_LITTLE_ENDIAN
    #error "Only one of VM_GHOST_LITTLE_ENDIAN and VM_GHOST_BIG_ENDIAN can be 1."
#endif

#define vm_ghost_has_VM_GHOST_BIG_ENDIAN 1
#define vm_ghost_has_VM_GHOST_LITTLE_ENDIAN 1

/************************************************
 * ghost/serialization/load/ghost_load_he_u32.h
 ************************************************/

#if VM_GHOST_DOCUMENTATION
uint32_t vm_ghost_load_he_u32(const void* p);
#endif

#ifndef vm_ghost_has_vm_ghost_load_he_u32
    #ifdef vm_ghost_load_he_u32
        #define vm_ghost_has_vm_ghost_load_he_u32 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_load_he_u32
    #ifndef VM_GHOST_LOAD_HE_U32_USE_UNALIGNED
        #if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)
            #define VM_GHOST_LOAD_HE_U32_USE_UNALIGNED 1
        #endif
    #endif
    #ifndef VM_GHOST_LOAD_HE_U32_USE_UNALIGNED
        #define VM_GHOST_LOAD_HE_U32_USE_UNALIGNED 0
    #endif

    VM_GHOST_IMPL_FUNCTION_OPEN
    vm_ghost_impl_always_inline
    vm_ghost_uint32_t vm_ghost_load_he_u32(const void* p) {
        #if VM_GHOST_LOAD_HE_U32_USE_UNALIGNED
            return *vm_ghost_static_cast(const vm_ghost_uint32_t*, p);
        #else
            uint32_t value;
            vm_ghost_uint8_t* vm_ghost_restrict to = vm_ghost_reinterpret_cast(vm_ghost_uint8_t*, &value);
            const vm_ghost_uint8_t* vm_ghost_restrict from = vm_ghost_static_cast(const vm_ghost_uint8_t*, p);
            to[0] = from[0];
            to[1] = from[1];
            to[2] = from[2];
            to[3] = from[3];
            return value;
        #endif
    }
    VM_GHOST_IMPL_FUNCTION_CLOSE

    #undef VM_GHOST_LOAD_HE_U32_USE_UNALIGNED
    #define vm_ghost_has_vm_ghost_load_he_u32 1
#endif

/**************************************************
 * ghost/serialization/store/ghost_store_he_u32.h
 **************************************************/

#if VM_GHOST_DOCUMENTATION
void vm_ghost_store_he_u32(void* p, uint32_t v);
#endif

#ifndef vm_ghost_has_vm_ghost_store_he_u32
    #ifdef vm_ghost_store_he_u32
        #define vm_ghost_has_vm_ghost_store_he_u32 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_store_he_u32
    #ifndef VM_GHOST_STORE_HE_U32_USE_UNALIGNED
        #if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)
            #define VM_GHOST_STORE_HE_U32_USE_UNALIGNED 1
        #endif
    #endif
    #ifndef VM_GHOST_STORE_HE_U32_USE_UNALIGNED
        #define VM_GHOST_STORE_HE_U32_USE_UNALIGNED 0
    #endif

    VM_GHOST_IMPL_FUNCTION_OPEN
    vm_ghost_impl_always_inline
    void vm_ghost_store_he_u32(void* p, vm_ghost_uint32_t v) {
        #if VM_GHOST_STORE_HE_U32_USE_UNALIGNED
            *vm_ghost_static_cast(vm_ghost_uint32_t*, p) = v;
        #else
            vm_ghost_uint8_t* vm_ghost_restrict to = vm_ghost_static_cast(vm_ghost_uint8_t*, p);
            vm_ghost_uint8_t* vm_ghost_restrict from = vm_ghost_reinterpret_cast(vm_ghost_uint8_t*, &v);
            to[0] = from[0];
            to[1] = from[1];
            to[2] = from[2];
            to[3] = from[3];
        #endif
    }
    VM_GHOST_IMPL_FUNCTION_CLOSE

    #undef VM_GHOST_STORE_HE_U32_USE_UNALIGNED
    #define vm_ghost_has_vm_ghost_store_he_u32 1
#endif

/********************************************************
 * ghost/impl/binding/libbsd/ghost_impl_libbsd_config.h
 ********************************************************/

#ifndef VM_GHOST_USE_LIBBSD
    #define VM_GHOST_USE_LIBBSD 0
#endif

/**********************************************************
 * ghost/impl/binding/libbsd/ghost_impl_libbsd_string_h.h
 **********************************************************/

#if VM_GHOST_USE_LIBBSD
    #ifdef LIBBSD_OVERLAY
        #include <string.h>
    #else
        #include <bsd/string.h>
    #endif
#endif

/*************************************
 * ghost/language/ghost_const_cast.h
 *************************************/

#ifndef vm_ghost_has_vm_ghost_const_cast
    #ifdef vm_ghost_const_cast
        #define vm_ghost_has_vm_ghost_const_cast 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_const_cast
    #ifdef __cplusplus
        #define vm_ghost_const_cast(Type, value) (const_cast<Type>(value))
        #define vm_ghost_has_vm_ghost_const_cast 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_const_cast
    #ifdef __GNUC__
        #if vm_ghost_has(vm_ghost_typeof)
            #define vm_ghost_const_cast(Type, value) ( \
                     \
                    (Type)(void*)(vm_ghost_uintptr_t)(const void*)(value) \
                )
            #define vm_ghost_has_vm_ghost_const_cast 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_const_cast
    #define vm_ghost_const_cast(Type, value) ((Type)(void*)(vm_ghost_uintptr_t)(const void*)(value))
    #define vm_ghost_has_vm_ghost_const_cast 1
#endif

/********************************
 * ghost/string/ghost_strlcat.h
 ********************************/

#if VM_GHOST_DOCUMENTATION
size_t vm_ghost_strlcat(char* restrict to, const char* restrict from, size_t count);
#endif

#ifndef vm_ghost_has_vm_ghost_strlcat
    #ifdef vm_ghost_strlcat
        #define vm_ghost_has_vm_ghost_strlcat 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_strlcat
    #if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
            defined(__DragonFly__) || defined(__APPLE__)
        #if vm_ghost_has(vm_ghost_string_h)
            #define vm_ghost_strlcat strlcat
            #define vm_ghost_has_vm_ghost_strlcat 1
        #endif
    #endif
#endif

#ifndef vm_ghost_strlcat
    #if VM_GHOST_USE_LIBBSD
        #define vm_ghost_strlcat strlcat
        #define vm_ghost_has_vm_ghost_strlcat 1
    #endif
#endif

#ifndef vm_ghost_strlcat
    VM_GHOST_IMPL_FUNCTION_OPEN
    vm_ghost_impl_function
    vm_ghost_size_t vm_ghost_strlcat(char* vm_ghost_restrict to,
            const char* vm_ghost_restrict from, vm_ghost_size_t count) VM_GHOST_IMPL_DEF(
    {
        const char* from_start = from;

        while (*to != '\000' && count != 0) {
            ++to;
            --count;
        }

        if (count > 0) {

            char* to_end = to + count - 1;
            while (to != to_end && *from) {
                *to++ = *from++;
            }

            *to = 0;
        }

        while (*from)
            ++from;
        return vm_ghost_static_cast(vm_ghost_size_t, from - from_start);
    })
    VM_GHOST_IMPL_FUNCTION_CLOSE
    #define vm_ghost_has_vm_ghost_strlcat 1
#endif

/********************************
 * ghost/string/ghost_strlcpy.h
 ********************************/

#if VM_GHOST_DOCUMENTATION
size_t vm_ghost_strlcpy(char* restrict dest, const char* restrict src, size_t n);
#endif

#ifndef vm_ghost_has_vm_ghost_strlcpy
    #if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
            defined(__DragonFly__) || defined(__APPLE__)
        #define vm_ghost_strlcpy strlcpy
        #define vm_ghost_has_vm_ghost_strlcpy 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_strlcpy
    #if VM_GHOST_USE_LIBBSD
        #define vm_ghost_strlcpy strlcpy
        #define vm_ghost_has_vm_ghost_strlcpy 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_strlcpy
    VM_GHOST_IMPL_FUNCTION_OPEN
    vm_ghost_impl_function
    vm_ghost_size_t vm_ghost_strlcpy(char* vm_ghost_restrict to,
            const char* vm_ghost_restrict from, vm_ghost_size_t count) VM_GHOST_IMPL_DEF(
    {
        const char* from_start = from;

        if (vm_ghost_expect_true(count > 0)) {

            char* to_end = to + count - 1;
            while (to != to_end && *from != '\000') {
                *to = *from;
                ++to;
                ++from;
            }

            *to = 0;
        }

        while (*from)
            ++from;
        return vm_ghost_static_cast(size_t, from - from_start);
    })
    VM_GHOST_IMPL_FUNCTION_CLOSE
    #define vm_ghost_has_vm_ghost_strlcpy 1
#endif

/**********************************
 * ghost/math/min/ghost_min_u32.h
 **********************************/

#if VM_GHOST_DOCUMENTATION
uint32_t vm_ghost_min_u32(uint32_t left, uint32_t right);
#endif

#ifndef vm_ghost_has_vm_ghost_min_u32
    #ifdef vm_ghost_min_u32
        #define vm_ghost_has_vm_ghost_min_u32 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_min_u32
    #if vm_ghost_has_vm_ghost_uint32_t
        VM_GHOST_IMPL_FUNCTION_OPEN
        vm_ghost_impl_inline
        vm_ghost_uint32_t vm_ghost_min_u32(vm_ghost_uint32_t left, vm_ghost_uint32_t right) {
            return (left < right) ? left : right;
        }
        VM_GHOST_IMPL_FUNCTION_CLOSE
        #define vm_ghost_has_vm_ghost_min_u32 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_min_u32
    #define vm_ghost_has_vm_ghost_min_u32 0
#endif

/************************************
 * ghost/math/rotr/ghost_rotr_u32.h
 ************************************/

#if VM_GHOST_DOCUMENTATION
uint32_t vm_ghost_rotr_u32(uint32_t value, unsigned bits);
#endif

#ifndef vm_ghost_has_vm_ghost_rotr_u32
    #ifdef vm_ghost_rotr_u32
        #define vm_ghost_has_vm_ghost_rotr_u32 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_rotr_u32
    #if !vm_ghost_has_vm_ghost_uint32_t
        #define vm_ghost_has_vm_ghost_rotr_u32 0
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_rotr_u32
    #ifdef __cplusplus
        #if vm_ghost_cplusplus >= 202002L
            #if vm_ghost_has(vm_ghost_cxx_bit_h)
                VM_GHOST_IMPL_FUNCTION_OPEN
                vm_ghost_impl_always_inline
                vm_ghost_uint32_t vm_ghost_rotr_u32(vm_ghost_uint32_t vm_ghost_impl_v_value, unsigned int vm_ghost_impl_v_bits) {
                    return ::std::rotr(vm_ghost_impl_v_value, vm_ghost_static_cast(int, vm_ghost_impl_v_bits));
                }
                VM_GHOST_IMPL_FUNCTION_CLOSE
                #define vm_ghost_has_vm_ghost_rotr_u32 1
            #endif
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_rotr_u32
    #if vm_ghost_has(vm_ghost_has_builtin)
        #if __has_builtin(__builtin_rotateright32)
            VM_GHOST_IMPL_FUNCTION_OPEN
            vm_ghost_impl_always_inline
            vm_ghost_uint32_t vm_ghost_rotr_u32(vm_ghost_uint32_t vm_ghost_impl_v_value, unsigned int vm_ghost_impl_v_bits) {
                return __builtin_rotateright32(vm_ghost_impl_v_value, vm_ghost_static_cast(unsigned char, vm_ghost_impl_v_bits));
            }
            VM_GHOST_IMPL_FUNCTION_CLOSE
            #define vm_ghost_has_vm_ghost_rotr_u32 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_rotr_u32
    #ifdef _MSC_VER
        VM_GHOST_IMPL_FUNCTION_OPEN
        vm_ghost_impl_always_inline
        vm_ghost_uint32_t vm_ghost_rotr_u32(vm_ghost_uint32_t vm_ghost_impl_v_value, unsigned int vm_ghost_impl_v_bits) {
            return _rotr(vm_ghost_impl_v_value, vm_ghost_static_cast(int, vm_ghost_impl_v_bits));
        }
        VM_GHOST_IMPL_FUNCTION_CLOSE
        #define vm_ghost_has_vm_ghost_rotr_u32 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_rotr_u32
    VM_GHOST_IMPL_FUNCTION_OPEN
    vm_ghost_impl_inline
    vm_ghost_uint32_t vm_ghost_rotr_u32(vm_ghost_uint32_t vm_ghost_impl_v_value, unsigned int vm_ghost_impl_v_bits) {
        vm_ghost_impl_v_bits &= (32 - 1);
        return vm_ghost_static_cast(vm_ghost_uint32_t, (vm_ghost_impl_v_value >> vm_ghost_impl_v_bits) |
                (vm_ghost_impl_v_value << ((-vm_ghost_impl_v_bits) & (32 - 1))));
    }
    VM_GHOST_IMPL_FUNCTION_CLOSE
    #define vm_ghost_has_vm_ghost_rotr_u32 1
#endif

/************************************************
 * ghost/serialization/load/ghost_load_le_u32.h
 ************************************************/

#if VM_GHOST_DOCUMENTATION
uint32_t vm_ghost_load_le_u32(const void* p);
#endif

#ifndef vm_ghost_has_vm_ghost_load_le_u32
    #ifdef vm_ghost_load_le_u32
        #define vm_ghost_has_vm_ghost_load_le_u32 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_load_le_u32
    #if VM_GHOST_LITTLE_ENDIAN
        #define vm_ghost_load_le_u32 vm_ghost_load_he_u32
        #define vm_ghost_has_vm_ghost_load_le_u32 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_load_le_u32
    #if VM_GHOST_BIG_ENDIAN
    #else
    #endif

    VM_GHOST_IMPL_FUNCTION_OPEN
    vm_ghost_impl_always_inline
    vm_ghost_uint32_t vm_ghost_load_le_u32(const void* vp) {
        #if VM_GHOST_BIG_ENDIAN
            return vm_ghost_bswap_u32(vm_ghost_load_he_u32(vp));
        #else
            const vm_ghost_uint8_t* p = vm_ghost_static_cast(const vm_ghost_uint8_t*, vp);
            return vm_ghost_static_cast(vm_ghost_uint32_t,
                    (vm_ghost_static_cast(vm_ghost_uint32_t, p[3]) << 24) |
                    (vm_ghost_static_cast(vm_ghost_uint32_t, p[2]) << 16) |
                    (vm_ghost_static_cast(vm_ghost_uint32_t, p[1]) << 8) |
                    (vm_ghost_static_cast(vm_ghost_uint32_t, p[0])));
        #endif
    }
    VM_GHOST_IMPL_FUNCTION_CLOSE
    #define vm_ghost_has_vm_ghost_load_le_u32 1
#endif

/**************************************************
 * ghost/serialization/store/ghost_store_le_u32.h
 **************************************************/

#if VM_GHOST_DOCUMENTATION
void vm_ghost_store_le_u32(void* p, uint32_t v);
#endif

#ifndef vm_ghost_has_vm_ghost_store_le_u32
    #ifdef vm_ghost_store_le_u32
        #define vm_ghost_has_vm_ghost_store_le_u32 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_store_le_u32
    #if VM_GHOST_LITTLE_ENDIAN
        #define vm_ghost_store_le_u32 vm_ghost_store_he_u32
        #define vm_ghost_has_vm_ghost_store_le_u32 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_store_le_u32
    #if vm_ghost_has(vm_ghost_uint32_t)
        #if VM_GHOST_BIG_ENDIAN
        #else
        #endif

        VM_GHOST_IMPL_FUNCTION_OPEN
        vm_ghost_impl_inline
        void vm_ghost_store_le_u32(void* vp, vm_ghost_uint32_t v) {
            #if VM_GHOST_BIG_ENDIAN
                vm_ghost_store_he_u32(vp, vm_ghost_bswap_u32(v));
            #else
                vm_ghost_uint8_t* p = vm_ghost_static_cast(vm_ghost_uint8_t*, vp);
                p[0] = vm_ghost_static_cast(uint8_t,  v        & 0xFF);
                p[1] = vm_ghost_static_cast(uint8_t, (v >>  8) & 0xFF);
                p[2] = vm_ghost_static_cast(uint8_t, (v >> 16) & 0xFF);
                p[3] = vm_ghost_static_cast(uint8_t, (v >> 24) & 0xFF);
            #endif
        }
        VM_GHOST_IMPL_FUNCTION_CLOSE
        #define vm_ghost_has_vm_ghost_store_le_u32 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_store_le_u32
    #define vm_ghost_has_vm_ghost_store_le_u32 0
#endif

/************************************
 * ghost/malloc/ghost_alloc_array.h
 ************************************/

#ifndef vm_ghost_has_vm_ghost_alloc_array
    #ifdef vm_ghost_alloc_array
        #define vm_ghost_has_vm_ghost_alloc_array 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_alloc_array
    #if vm_ghost_has(vm_ghost_malloc)

        VM_GHOST_IMPL_FUNCTION_OPEN
        VM_GHOST_IMPL_ATTRIB_MALLOC
        VM_GHOST_IMPL_ATTRIB_ALLOC_SIZE((1, 2))
        vm_ghost_impl_inline
        void* vm_ghost_impl_alloc_array(vm_ghost_size_t vm_ghost_impl_v_count, vm_ghost_size_t vm_ghost_impl_v_stride) {
            vm_ghost_size_t vm_ghost_impl_v_total;
            if (vm_ghost_expect_false(vm_ghost_checked_mul_z(&vm_ghost_impl_v_total, vm_ghost_impl_v_count, vm_ghost_impl_v_stride))) {
                vm_ghost_errno_store(EINVAL);
                return vm_ghost_null;
            }
            return vm_ghost_malloc(vm_ghost_impl_v_total);
        }
        VM_GHOST_IMPL_FUNCTION_CLOSE

        #define vm_ghost_alloc_array(T, n) vm_ghost_bless(T, n, vm_ghost_impl_alloc_array((n), sizeof(T)))
        #define vm_ghost_has_vm_ghost_alloc_array 1
    #else
        #define vm_ghost_has_vm_ghost_alloc_array 0
    #endif
#endif

/***********************************
 * ghost/malloc/ghost_alloc_zero.h
 ***********************************/

#ifndef vm_ghost_has_vm_ghost_alloc_zero
    #ifdef vm_ghost_alloc_zero
        #define vm_ghost_has_vm_ghost_alloc_zero 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_alloc_zero
    #if vm_ghost_has(vm_ghost_calloc)
        #define vm_ghost_alloc_zero(T) vm_ghost_bless(T, 1, vm_ghost_calloc(1, sizeof(T)))
        #define vm_ghost_has_vm_ghost_alloc_zero 1
    #else
        #define vm_ghost_has_vm_ghost_alloc_zero 0
    #endif
#endif

/*******************************
 * ghost/malloc/ghost_strdup.h
 *******************************/

#if VM_GHOST_DOCUMENTATION
char* vm_ghost_strdup(const char* s);
#endif

#if 0
#ifndef vm_ghost_has_vm_ghost_strdup
    #if VM_GHOST_MALLOC_SYSTEM
        #if vm_ghost_has(vm_ghost_has_builtin)
            #if __has_builtin(__builtin_strdup)
                #define vm_ghost_strdup __builtin_strdup
                #define vm_ghost_has_vm_ghost_strdup 1
            #endif
        #else
            #if VM_GHOST_GCC
                #if __GNUC__ >= 4
                    #define vm_ghost_strdup __builtin_strdup
                    #define vm_ghost_has_vm_ghost_strdup 1
                #endif
            #endif
        #endif
    #endif
#endif
#endif

#ifndef vm_ghost_has_vm_ghost_strdup
    #if VM_GHOST_MALLOC_SYSTEM
        #ifdef _WIN32
            VM_GHOST_IMPL_FUNCTION_OPEN

            VM_GHOST_IMPL_ATTRIB_MALLOC
            vm_ghost_impl_always_inline
            char* vm_ghost_strdup(const char* s) {
                return _strdup(s);
            }

            VM_GHOST_IMPL_FUNCTION_CLOSE

            #define vm_ghost_has_vm_ghost_strdup 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_strdup
    #if VM_GHOST_MALLOC_SYSTEM
        #ifdef __linux__
            VM_GHOST_IMPL_FUNCTION_OPEN

            VM_GHOST_IMPL_ATTRIB_MALLOC
            vm_ghost_impl_always_inline
            char* vm_ghost_strdup(const char* s) {
                #if VM_GHOST_IMPL_LIBC_ASM
                    vm_ghost_dllimport
                    char* vm_ghost_impl_ex_strdup(const char* s)
                        vm_ghost_impl_libc_noexcept(vm_ghost_impl_libc_asm("strdup"));
                    return vm_ghost_impl_ex_strdup(s);
                #else
                    vm_ghost_dllimport
                    char* strdup(const char* s)
                        vm_ghost_impl_libc_noexcept(VM_GHOST_NOTHING);
                    return strdup(s);
                #endif
            }

            VM_GHOST_IMPL_FUNCTION_CLOSE

            #define vm_ghost_has_vm_ghost_strdup 1
        #endif
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_strdup
    #if vm_ghost_has(vm_ghost_malloc)
        VM_GHOST_IMPL_FUNCTION_OPEN

        VM_GHOST_IMPL_ATTRIB_MALLOC
        vm_ghost_impl_function
        char* vm_ghost_strdup(const char* s) {
            vm_ghost_size_t size = vm_ghost_strlen(s) + 1;
            char* p = vm_ghost_bless(char, size, vm_ghost_malloc(size));
            if (p)
                vm_ghost_memcpy(p, s, size);
            return p;
        }

        VM_GHOST_IMPL_FUNCTION_CLOSE

        #define vm_ghost_has_vm_ghost_strdup 1
    #endif
#endif

#ifndef vm_ghost_has_vm_ghost_strdup
    #define vm_ghost_has_vm_ghost_strdup 0
#endif

#endif

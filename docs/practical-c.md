# Onramp Practical C

Onramp Practical C (opC) is a subset of C. It is implemented by the [second stage Onramp compiler (cci/1-opc)](../core/cci/1-opc/).

It's designed to be simple enough that we can implement it in [Onramp Minimal C (omC)](minimal-c.md), but featureful enough that we can use it to comfortably implement all of modern C. It is the implementation language for the final stage of most of the core components in Onramp.



## Details

opC is a strict subset of C99. It is somewhat limited compared to ANSI C but the syntax is more like C99 in practice.

Starting from the ANSI C spec, we backport a few quality-of-life features from C99:

- mixed declarations and code;
- `for` loop variable declarations;
- C++-style `//` comments;
- `long long`;
- trailing commas in enums;
- flexible array members (struct hack);

However, we exclude a few ANSI C features and common extensions:

- function pointers;
- multi-dimensional arrays;
- floating point types;
- digraphs (C95) and trigraphs of course;
- K&R-style function declarations;
- function declarations without arguments;
- labels and `goto`;
- and more.

We also simplify a few others (in some cases violating proper C rules):

- declarations of pointers to arrays are not supported. e.g. `int** x[3];` works but `int* (*x)[3];` does not.
- `union` may be an alias of `struct` (union type punning doesn't necessarily work.)
- `_Bool` (C99) is an alias of `unsigned char` (true values are not necessarily coerced to 1.)
- Bitfield size specifiers are ignored (and zero-width bitfield members are forbidden.)
- `const`, `auto`, `register`, `volatile`, `inline`, `_Noreturn`, `restrict` and more are all ignored.
- All C11 attributes including `[[noreturn]]` and `[[deprecated]]` are ignored.
- Alignment specifiers are ignored.
- Enum types are equivalent to `int`, and enum values are integer global variables (and cannot be used in constant expressions.)
- `enum`, `struct`, `union` and `typedef` definitions are only allowed at file scope.

The syntax for many of these features is still parsed so that our final stage C compiler can use them and recompile itself to take advantage of them.

The lack of function pointers is limiting since it eliminates several forms of metaprogramming and polymorphism. It is a worthwhile tradeoff however since this, in combination with the array and qualifier limitations, makes the type system for opC extremely simple to implement. See the implementation document linked above for details.

We're not going to bother writing up much of a spec here. The spec is basically ANSI C (ISO/IEC 9899:1990) with the additions and changes mentioned above.



## Preprocessor

opC does not define a corresponding preprocessor stage. The opC compiler is paired with either the omC preprocessor or the full C preprocessor depending on the phase of bootstrapping. Code written in opC tends to support both.

For example, `string_hash()` in libo is defined like this:

```c
/**
 * Returns the FNV-1a hash of the given string.
 */
#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define string_hash(str) ((str)->entry.hash)
    #endif
#endif
#ifndef string_hash
    static inline const char* string_hash(string_t* str) {
        return str->entry.hash;
    }
#endif
```

We prefer to define it as a macro for performance reasons since Onramp does not currently optimize inline functions. This only works if a full C preprocessor is available. Since the omC preprocessor does not support function-like macros, we provide a static function fallback.

Note that we avoid the use of `#else` and other features not supported by omC, at least outside of those blocks guarded by `#ifndef __onramp_cpp_omc__`.

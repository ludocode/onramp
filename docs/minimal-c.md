# Onramp Minimal C

Onramp Minimal C (omC) is a tiny subset of C. It is implemented by the [first stage Onramp compiler (cci/0-omc)](../core/cci/0-omc/).

It contains the minimum number of C features required to reasonably implement the next stage compiler. It's designed to be simple enough for its compiler to be implemented in assembly, but powerful enough to be used implement most of ANSI C (i.e. [Onramp Practical C](practical-c.md).) It is a critical step in the jump from assembly to a full implementation of C.

Compared to C, it has:

- Only `void`, `int` and `char` types and pointers to them.
    - No `struct`/`union`/`enum`;
    - No arrays;
    - No `unsigned`, `long`, `short`;
    - No function pointers;
    - `const` is ignored;
    - Limited `typedef`.
- Only `if`, `while`, `continue`, `break` and `return` for control flow.
    - No `else`;
    - No `for` or `do` loops;
    - No labels or `goto`;
    - No `switch`/`case`;
    - Braces are required for `if` and `while`.
- Limited operators: only simple arithmetic and logic.
    - No compound assignment (`+=`, `&=`, etc.)
    - No `++`/`--`;
    - No logical `&&` or `||` (bitwise `&` and `|` must be used instead);
    - No ternary conditional `?:`, comma operator, and more;
    - `sizeof` only with parenthesized types, not expressions.
- No binary operator precedence. All binary expressions must be fully parenthesized!
- No variadic functions.
- Only `#include`, `#ifdef`/`#ifndef`, and object-like `#define` preprocessor directives.

Despite these limitations, omC is a proper subset of C. It can be compiled (and tested!) with any modern C compiler.

We support enough of C to define libc-compatible headers. This requires `void` and `void*`, `extern` for global variables, and `typedef` for `FILE` and `size_t` (it's actually harder to do these with `#define`.) The support for `typedef` and file inclusion in particular allows for the construction of large scale projects with idiomatic code organization in omC.

We also support a few C99 features that were not in C89, namely mixed declarations and code and C++-style comments.



## History

omC is analogous to the short-lived [NB or "New B"](https://www.bell-labs.com/usr/dmr/www/chist.html), an extension of [B](https://en.wikipedia.org/wiki/B_(programming_language)) that served as an intermediary to C. NB essentially just added the `int` and `char` types to the typeless B, which paved the way for `struct` support to become C. Our subsets of C are inspired by the way C was originally bootstrapped by Dennis Ritchie more than five decades ago.

Unfortunately NB was lost. It went through several iterations, probably never having any concrete specification, and it existed so briefly that no version of it was preserved. This time, we are keeping the intermediate language around! We are implementing it in assembly and we will attempt to specify it here.

The main difference between NB and omC is, rather than making it an extension of B, omC is instead a proper subset of C. omC files can be compiled with any ordinary C99 or later compiler. This allows us to use modern tools to test and debug our code.



## Preprocessor

The omC preprocessor supports only a subset of the preprocessing directives in standard C:

- `#include`
- `#define` for non-recursive object-like macros
- `#ifdef`, `#ifndef` and `#endif` conditionals
- `#line`

The above features are enough to support header files with include guards and to do platform-based conditionals and substitutions (to test and debug on Linux.) This makes it possible to build large-scale portable projects in omC with idiomatic C-style code organization.

The omC preprocessor defines `__onramp_cpp__` to 1 and `__onramp_cpp_omc__` to 1.

Any unrecognized preprocessor directives are ignored in untaken conditional branches. You should therefore wrap any conventional preprocessor directives (like `#if`) in `#ifndef __onramp_cpp_omc__`. Most final-stage Onramp programs use this to support bootstrapping with the omC preprocessor and the opC compiler.



### Limitations

The omC preprocessor do not support function-like macros (i.e. `#define` with arguments); `#if` and friends; `#else`; `#undef`; and more. It also supports no built-in macros (like `__FILE__` or `__LINE__`), no modern features (like `__has_builtin`) and no extensions (like `__COUNTER__` or `#include_next`.)

The omC preprocessor, like the compiler, only supports the [ASCII](https://en.wikipedia.org/wiki/ASCII)-encoded [POSIX portable character set](https://en.wikipedia.org/wiki/Portable_character_set), excluding null, alert and backspace. Any characters outside of this range cause an error.

The omC preprocessor does not support recursive macros. For example:

```
#define FOO BAR
#define BAR FOO
FOO
```

Unlike standard C (which preprocesses this to `FOO`), in the omC preprocessor this is malformed. The omC preprocessor may diagnose it, or it may crash, go into an infinite loop or exhibit other undefined behaviour. This also means that you cannot define a macro to itself, which is otherwise legal C.

The `#` character can only appear at the start of a line, after optional whitespace and comments. Since `#` cannot appear anywhere else outside of a comment, the preprocessor is not required to detect and diagnose this. It may treat a `#` anywhere outside of a comment as the start of a preprocessor directive. Such code is nevertheless malformed and this behaviour should not be relied upon.

Macro names and expansions are limited to 256 characters.

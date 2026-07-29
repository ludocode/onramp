# Onramp Implementation Status

This is an incomplete list of features in C and related standards along with their implementation status in Onramp.

If a ✓ appears in the status column, the feature is implemented in Onramp. If a ½ appears in the status column, the feature is at least partially implemented, and may or may not be usable.

This is perhaps a wishlist of features that we'd like to implement, but it is in no way a promise or even a plan to implement them. Many of the features here will probably never be implemented. The main motivation for implementing features is whether real software we want to compile uses them.

Standards:

- [Onramp](#onramp)
- [C89 (ANSI C)](#c89-ansi-c)
- [C95](#c95)
- [C99](#c99)
- [C11](#c11)
- [C17](#c17)
- [C23](#c23)
- [C2Y](#c2y)
- [POSIX](#posix)
- [BSD](#bsd)
- [Common Extensions](#gnu-extensions)
- [GNU Extensions](#gnu-extensions)
- [Clang Extensions](#clang-extensions)
- [Microsoft Extensions](#microsoft-extensions)
- [Plan 9 Extensions](#plan-9-extensions)
- [TinyCC Extensions](#tinycc-extensions)





## Onramp

This is mostly a list of internal Onramp features that still need to be implemented.

### Virtual Machine

A feature is checked here if it is well-specified and it is implemented in the debugger and (for non-debug features) other common VMs.

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| Basic instruction set                                         |    ✓   |
| Command-line args, environment vars                           |    ✓   |
| Syscall table, optional syscalls                              |    ✓   |
| Syscalls: File I/O                                            |    ✓   |
| Syscalls: Stat                                                |        |
| Syscalls: Directories                                         |        |
| Syscalls: Symlinks                                            |        |
| Syscalls: Time, Sleep                                         |    ½   |
| Syscalls: External Spawn                                      |        |
| Interrupts, compare-and-swap                                  |        |
| Debug info                                                    |    ✓   |
| Stack traces                                                  |    ✓   |
| Debugger single-stepping                                      |    ✓   |
| Debugger breakpoints                                          |        |
| Profiling                                                     |        |

### Compiler

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| External zero/uninitialized storage (i.e. "bss")              |        |
| RTL-style IR for optimization                                 |    ✓   |
| Language standard selection (e.g. `-std=c11`)                 |        |
| Warning flags                                                 |   ½\*  |
| Rich error messages, notes                                    |   \*   |

\* Parsing of warning flags mostly works but only in `cci`. It needs to be moved to libo and shared with `cc` and `cpp`.

\* Rich error messages should print the stack of include files that led to a token and should display the source line of code with an arrow pointing to the column containing the error. This also needs to be moved to libo.

### Preprocessor

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| Header guard optimization                                     |        |
| Warnings                                                      |        |

### Linker

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| Constructors and destructors                                  |    ✓   |
| Weak symbols                                                  |        |
| Garbage collection                                            |        |

### Standard Library

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| Software floating point                                       |        |





## C89 (ANSI C)

### Compiler

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| Basic integer types                                           |    ✓   |
| `long long`                                                   |    ✓   |
| `float`, `double`                                             |   \*   |
| Arrays                                                        |    ✓   |
| Structs, unions                                               |    ✓   |
| Bitfields                                                     |   ½\*  |
| Enums                                                         |    ✓   |
| Functions                                                     |    ✓   |
| Function prototypes                                           |    ✓   |
| K&R-style functions                                           |        |
| Function pointers                                             |    ✓   |
| Typedef                                                       |    ✓   |
| Declarator parsing                                            |    ✓   |
| Arithmetic expressions                                        |    ✓   |
| Casts, implicit and explicit                                  |    ✓   |
| Address-of and dereference                                    |    ✓   |
| Passing structs by value                                      |    ✓   |
| Type promotion rules                                          |    ✓   |
| Functions                                                     |    ✓   |
| Variadic functions                                            |    ✓   |
| Linkage specifiers                                            |    ✓   |
| `if`, `else`                                                  |    ✓   |
| `for`, `while`, `do`                                          |    ✓   |
| `switch`, `case`, `default`                                   |    ✓   |
| Labels, `goto`                                                |    ✓   |
| Tentative definitions                                         |    ✓   |
| `const` checks                                                |    ✓   |
| Wide characters and strings (`L` prefix)                      |        |
| Initializers                                                  |   ½\*  |
| String escape sequences                                       |    ✓   |
| `setjmp()`/`longjmp()`                                        |    ✓   |

\* `float` and `double` support is mostly implemented in the compiler (except for floating-point literals) but it emits calls to libc functions that do not yet exist. Floating point arithmetic functions must be implemented in the libc; in the meantime it is not usable.

\* Bitfields are parsed but ignored. This is a valid implementation of the spec, except for missing required diagnostics (e.g. preventing taking the address of a bitfield member.)

\* Initializers are fully implemented and do work but they are grossly inefficient. A simple static array of `char` will emit initializaton bytecode an order of magnitude larger than the actual data. There is also no support yet for zero or uninitialized storage (i.e. there is no "bss" section.)

### Preprocessor

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| Object-like macros                                            |    ✓   |
| Function-like macros                                          |    ✓   |
| Recursive macro expansion with hideset                        |    ✓   |
| `__FILE__`, `__LINE__`                                        |    ✓   |
| `__DATE__`, `__TIME__`                                        |        |
| Stringify                                                     |    ✓   |
| Token pasting                                                 |    ✓   |
| `#ifdef`, `#ifndef`, `#endif`                                 |    ✓   |
| `#if`, `#elif` expression evaluation                          |    ✓   |
| `#include` with macro expansion                               |    ✓   |

### Standard Library

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| `<stddef.h>` basic types and macros                           |    ✓   |
| Basic printing: `fputs()`, `fputc()`, etc.                    |    ✓   |
| Strings: `memcpy()`, `strncmp()`, etc.                        |    ✓   |
| Formatting: `printf()`, `vsnprintf()`, etc.                   |   ½\*  |
| Scanning: `scanf()`, `sscanf()`, etc.                         |        |
| File I/O: `fopen()`, `fread()`, etc.                          |    ✓   |
| Memory allocation: `malloc()`, `realloc()`, etc.              |    ✓   |
| `<math.h>` functions                                          |        |
| `<errno.h>`                                                   |    ✓   |
| `<stdlib.h>` algorithms: `qsort()`, etc.                      |    ½   |
| `<ctype.h>` character functions                               |    ✓   |
| Signals: `<signal.h>`, `sigaction()`, `raise()`               |        |

\* Basic `printf()` formatting works but specifier flags and modifiers are mostly ignored.





## C95

No additional compiler features are necessary for C95. The C95 preprocessor and library additions are included under `-std=c89`.

### Preprocessor

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| digraphs                                                      |        |
| `__STDC_VERSION__`                                            |        |

### Standard Library

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| wide character support: `<wchar.h>`, `<wctype.h>`             |        |
| wide character I/O                                            |        |
| `<iso646.h>` alternate macros                                 |    ✓   |






## C99

### Compiler

| Feature                                       | Status |
|:----------------------------------------------|:------:|
| Mixed declarations and code                   |    ✓   |
| `for` loop initializer                        |    ✓   |
| Flexible array member                         |    ✓   |
| Anonymous struct member                       |    ✓   |
| `_Bool`                                       |    ✓   |
| `__func__`                                    |    ✓   |
| `inline`                                      |    \*  |
| `restrict`                                    |   ✓\*  |
| Compound literals                             |    ✓   |
| Variable-length arrays                        |        |
| Complex and imaginary numbers                 |        |
| Floating-point environment controls           |        |
| `static` in array parameters                  |        |
| `va_copy()`                                   |    ✓   |

\* `inline` is ignored. This works fine for `static inline` but non-extern `inline` will have external definitions emitted which will cause linker errors.

\* `restrict` is mostly ignored, which is valid except for the omission of required diagnostics.

### Preprocessor

| Feature                                       | Status |
|:----------------------------------------------|:------:|
| Line comments (`//`)                          |    ✓   |
| Variadic macros                               |    ✓   |
| `_Pragma`                                     |    ✓   |

### Standard Library

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| `<stdint.h>`                                                  |    ✓   |
| `<inttypes.h>`                                                |    ✓   |
| `<stdbool.h>`, `bool`, `true`, `false`                        |    ✓   |
| `<complex.h>` math functions                                  |        |
| `<tgmath.h>`                                                  |        |
| `<fenv.h>`                                                    |        |





## C11

### Compiler

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| Unicode string literals                                       |    ?   |
| Alignment specifiers, `_Alignas` and `_Alignof`               |        |
| `__typeof__`                                                  |        |
| `_Static_assert`                                              |        |
| `_Atomic`                                                     |        |
| `_Thread_local`                                               |        |
| `_Generic()` expressions                                      |        |

### Preprocessor

| Feature                                       | Status |
|:----------------------------------------------|:------:|
| Feature test macros                           |        |
| Complex number construction: `CMPLX()`, etc.  |        |

### Standard Library

| Feature                                       | Status |
|:----------------------------------------------|:------:|
| `aligned_alloc()`                             |    ✓   |
| `<stdatomic.h>`                               |        |
| `<threads.h>`                                 |        |





## C17

C17 added no new features and only fixed C11 defects. In Onramp, C17 is an alias for C11; all C17 fixes are backported to C11.





## C23

### Compiler

| Feature                                       | Status |
|:----------------------------------------------|:------:|
| `[[attributes]]`                              |        |
| Label without statement                       |    ✓   |
| Digit separator                               |    ✓   |
| `typeof`, `typeof_unqual`                     |        |
| `nullptr`                                     |        |
| Unicode identifiers                           |   ½\*  |

\* Unicode identifiers are generally supported but Onramp's set of identifier characters intentionally differs from Unicode Annex 31 in order to avoid a dependency on generated data tables.

### Preprocessor

| Feature                                       | Status |
|:----------------------------------------------|:------:|
| `__has_c_attribute`                           |        |
| `__has_include`                               |        |
| `__has_embed`                                 |        |
| `__has_builtin`                               |        |
| `#elifdef`, `#elifndef`                       |    ✓   |
| `#warning`                                    |    ½   |
| `__VA_OPT__`                                  |        |

### Standard Library

| Feature                                       | Status |
|:----------------------------------------------|:------:|
| `<stdbit.h>`                                  |        |
| `<stdckdint.h>`                               |        |
| `free_sized()`, `free_aligned_sized()`        |    ✓   |





## C2Y

### Compiler

| Feature                                       | Status |
|:----------------------------------------------|:------:|
| `defer` (TS 25755)                            |    ✓   |





## POSIX

POSIX features are activated by defining `_POSIX_C_SOURCE` before including any header files.

POSIX is huge, and Onramp will never be able to implement some parts of it (such as `fork()`.) This is just a small subset of features, those we need or could conceivably implement in the near future.

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| I/O with file descriptors (`open()`, `read()`, etc.)          |    ½   |
| Symlinks (`symlink()`, `lstat()`)                             |        |
| Terminal control (`<termios.h>`, `fcntl()` on stdin)          |    ½   |
| `<sys/stat.h>`: `chmod()`, `stat()`                           |    ½   |
| Process spawn (`vfork()`, `exec()`, `posix_spawn()`           |        |
| Threads (`<pthread.h>`)                                       |        |
| `<strings.h>`                                                 |    ½   |
| `<locale.h>`, `lconv()`                                       |        |





## BSD

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| Safe string copying: `strlcpy()`, `strlcat()`                 |        |
| `arc4random()`                                                |        |





## Common Extensions

| Feature                                       | Status |
|:----------------------------------------------|:------:|
| `#pragma once`                                |        |





## GNU Extensions

GNU extensions are activated with the command-line argument `-fgnu-extensions` or with a `-std=gnu*` option.

When GNU extensions are enabled, some later version C features are available in earlier versions.

### Compiler

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| `alloca()`                                                    |        |
| `__attribute__`                                               |        |
| Statement expressions                                         |    ✓   |
| `__asm__` name                                                |    ✓   |
| `case` ranges                                                 |        |
| gnu89-style `inline`                                          |        |
| Member designators using `:`                                  |        |
| Computed gotos (`&&label`)                                    |        |
| local (block scope) labels                                    |        |
| `__builtin_types_compatible_p()`                              |        |
| `__builtin_add_overflow()` etc.                               |        |
| Nested functions                                              |        |
| r-value array subscripts                                      |        |
| `void*` arithmetic                                            |    ✓   |
| Cast to union                                                 |        |
| `__extension__`                                               |        |
| 128-bit integers                                              |        |
| Half floats (`_Float16` or `__fp16`)                          |        |
| Elvis operator (`?:`)                                         |        |
| `__builtin_apply()`, constructed function calls               |        |
| Dollar signs in identifiers                                   |    ✓   |
| `__sync` atomics                                              |        |

### Preprocessor

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| `, ## __VA_ARGS__` comma elision                              |        |
| `__COUNTER__`                                                 |    ✓   |
| `__INCLUDE_LEVEL__`                                           |        |
| `__INT_WIDTH__` and other `<stdint.h>` macros as builtins     |        |
| Directives in macro arguments                                 |    ✓   |
| `#include_next`                                               |        |
| GNU-style linemarkers                                         |        |





## Clang Extensions

Clang extensions are included in GNU extensions. They are activated with the command-line argument `-fgnu-extensions`.

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| `__has_warning`                                               |        |
| `__has_extension`                                             |        |
| Deprecated, final, restricted expansion macros                |        |





## Microsoft Extensions

Microsoft Visual C++ extensions (to C) are activated with the command-line argument `-fms-extensions`.

| Feature                                       | Status |
|:----------------------------------------------|:------:|
| `#pragma pack`                                |        |





## Plan 9 Extensions

Plan 9 extensions are activated with the command-line argument `-fplan9-extensions`.

| Feature                                                       | Status |
|:--------------------------------------------------------------|:------:|
| Anonymous non-inline struct                                   |        |
| Implicit conversion of struct ptr to anonymous member ptr     |        |
| Array designators without `=`                                 |        |





## TinyCC Extensions

The only TinyCC extension Onramp plans to support is running as a script.

| Feature                                       | Status |
|:----------------------------------------------|:------:|
| `-run` and `#!` script support                |        |

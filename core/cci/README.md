# Onramp Compiler

`cci` is the Onramp preprocessed C compiler. It takes preprocessed C as input and outputs Onramp assembly.

This is a description of the implementation stages of the compiler. The input to the compiler is (a subset of) the C programming language after preprocessing, the subset of which depends on the stage. For a specification of the output of the compiler, see [Onramp assembly](../../docs/assembly.md).

(It's called "`cci`" because it compiles `.i` files. The analogous program in other compilers might be called `cc1`, except a `cc1` binary typically also handles preprocessing, which in Onramp is the separate program `cpp`.)

The compiler is implemented in three stages:

- [`0-omc`](0-omc/) is written in compound assembly and compiles [Onramp Minimal C (omC)](../../docs/minimal-c.md). It is a single-pass compiler designed to be as simple as possible with no optimizations whatsoever.

- [`1-opc`](1-opc/) is written in omC and compiles [Onramp Practical C (omC)](../../docs/practical-c.md). This aims to support a large subset of modern C sufficient to comfortably implement the final stage. It is also a single-pass compiler with only trivial micro-optimizations where it is convenient.

- [`2-full`](2-full/) aims to implement most of C2x, sufficient to compile any modern C software.



## Input Format

The input to `cci` is the output of the Onramp preprocessor `cpp`. The preprocessor performs the first five [phases of translation](https://en.cppreference.com/w/c/language/translation_phases). This means:

- Preprocessing is already done. We do not need to handle any preprocessor directives (except for debug info, see below) or include any other files.
- Comments are stripped. No stage of `cci` handles comments at all.
- Line endings are normalized. There should be no carriage returns in the input.

These restrictions greatly simplify our lexer. Unlike all other Onramp programs, `cci` is not designed to accept hand-written input files.

The only exception for preprocessor directives is `#line` and `#pragma` for debug info. These are handled specially by the lexer.



## Lexer

The lexer is similar in all three stages. Its operation is described here.

The lexer consumes characters from the input file and produces tokens. Each token is one of the following types:

- "a" -- alphanumeric (a keyword, a type name or a variable name)
- "n" -- number
- "c" -- character literal
- "s" -- string literal (not merged with adjacent string literals)
- "p" -- punctuation (a valid operator or other punctuation token)
- "e" -- end-of-file

The lexer does not use the typical C [lexer hack](https://en.wikipedia.org/wiki/Lexer_hack). Instead the lexer just returns tokens of type "alphanumeric". It is up to the parser to determine whether such tokens are keywords, type names or variable names. The lexer is entirely context-free.

The lexer also handles the few preprocessor directives supported (`#line` and `#pragma`). The lexer does *not* handle comments; they must be stripped by an Onramp preprocessor.

Later stages have a slightly more sophisticated lexer but it follows the same basic principles.

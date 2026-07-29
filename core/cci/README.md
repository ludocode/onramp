# Onramp Compiler

`cci` is the Onramp preprocessed C compiler. It takes preprocessed C as input and outputs Onramp assembly.

This is a description of the implementation stages of the compiler. The input to the compiler is (a subset of) the C programming language after preprocessing, the subset of which depends on the stage. The output also depends on the stage; see below.

(It's called "`cci`" because it compiles `.i` files. The analogous program in other compilers might be called `cc1`, except a `cc1` binary typically also handles preprocessing, which in Onramp is the separate program `cpp`.)

The compiler is implemented in three stages:

- [`0-omc`](0-omc/) is written in compound assembly and compiles [Onramp Minimal C (omC)](../../docs/minimal-c.md). It is a single-pass compiler designed to be as simple as possible. It emits a subset of [Onramp assembly](../../docs/assembly.md) which can be optimized by the [first stage code generator](../cg/0-asm/).

- [`1-opc`](1-opc/) is written in omC and compiles [Onramp Practical C (opC)](../../docs/practical-c.md). This aims to support a large subset of modern C sufficient to comfortably implement the final stage. It is also a single-pass compiler emitting assembly like the previous stage.

- [`2-full`](2-full/) is written in opC and aims to implement most of C23 plus several extensions. This adds function pointers, `long long`, floating point and more. It should be sufficient to compile most modern C software. It emits an [intermediate representation (IR)](../../docs/intermediate-representation.md) for the [final stage code generator](../cg/1-full/).




## Input Format

The input to `cci` is the output of the Onramp preprocessor `cpp`. The preprocessor performs the first five [phases of translation](https://en.cppreference.com/w/c/language/translation_phases). This means:

- Preprocessing is already done. We do not need to handle any preprocessor directives (except for debug info, see below) or include any other files.
- Comments are stripped. No stage of `cci` handles comments at all.
- Line endings are normalized. There should be no carriage returns in the input.

These restrictions greatly simplify our lexer. Unlike most other Onramp programs, `cci` is not designed to accept hand-written input files.

The only exception for preprocessor directives is `#line` and `#pragma` for debug info. These are handled specially by the lexer in each stage.

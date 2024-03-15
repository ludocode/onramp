# Boostrapping Path

This document describes the bootstrap process that Onramp takes to get from nothing all the way to a full implementation of C.



## Overview

First we need a hex tool and virtual machine. These steps are platform-specific.

- [hex](../platform/hex), any implementation for your system
- [vm](../platform/vm), any implementation for your system

We also need a few tools to run the bootstrap scripts. These tools are platform-independent but they need to be bootstrapped in a platform-specific manner.

- [hex/onramp](../platform/hex/onramp), replacing the native tool, with improved error checking and the ability to run inside the VM.
- [sh](../core/sh), the [Onramp shell](shell.md), to run the bootstrap shell script inside the VM.

The rest of the process happens inside the VM, scripted automatically by the Onramp shell. It is platform-independent. See [core/build.sh](../core/build.sh) for the actual script.

We start by getting the linker up, along with a libc and utility library we can link in to the rest of our programs.

- [ld/0-labels](../core/ld/0-labels), the basic linker written in hexadecimal bytecode, giving us support for labels
- [libc/0-oo](../core/libc/0-oo), a basic libc. `__start()`/`exit()`, `memcpy()`/`strlen()`, `fopen()`/`fread()`/`fwrite()`, etc.
- [libo/0-oo](../core/libo/0-oo), a utility library for compiler development. `fatal()` error handling, `iota()`, `fnv1a()`, etc.
- [ar/0-cat](../core/ar/0-cat), a trivial archiving tool to join our libc and libo into static libraries

Next we build our way up to a powerful assembler.

- [as/0-basic](../core/as/0-basic), a primitive assembler
- [as/1-compound](../core/as/1-compound), adding decimal numbers and many more opcodes: `call`/`ret`, `push`/`pop`, etc.

We now have a rich [assembly language](assembly.md). It's powerful enough both to implement our [Onramp Minimal C (omC)](minimal-c.md) compiler and to serve as the compilation target for all Onramp compiler stages.

- [cci/0-omc](../core/cci/0-omc), the omC compiler written in assembly language
- [cpp/0-strip](../core/cpp/0-strip), a fake "preprocessor", also in assembly, that just strips preprocessor directives

We can now write everything in [omC](minimal-c.md)! To complete our omC implementation, we still need a (real) preprocessor, linker, and driver, plus libc upgrades.

- [cpp/1-omc](../core/cpp/0-omc), the basic omC preprocessor
- [ld/1-omc](../core/ld/1-omc), a new linker that adds file scope
- [libc/1-omc](../core/libc/1-omc), a more featureful libc
- [cc](../core/cc), the compiler driver

At this point we have a usable `cc` tool. It can preprocess, compile, assemble and link `.c` files, but only if they contain omC. We're now ready to build our [Onramp Practical C (opC)](practical-c.md) compiler.

- [cci/1-opc](../core/cci/1-opc), the opC compiler
- [libc/2-opc](../core/libc/2-opc), a nearly complete libc

This adds the important features of C99: struct, enum, arrays, switch, for loops, the full expression syntax, `printf()` and friends, and many other things we will need to build a real compiler. We also have modern conveniences like mixed declarations and code and C++-style comments.

We don't bother with more advanced versions of the other tools yet. These are powerful enough to go straight to our feature-complete C compiler.

- [libo/1-opc](../core/libo/1-opc), additional utilities for compiler development
- [cci/2-full](../core/cci/2-full), the full C compiler

Now that we have a full C compiler, we can build up the rest of our implementation, all of which is written in modern C.

- [cpp/2-full](../core/cpp/2-full), the full C preprocessor (`#if`, variadic function-like macros, `#embed`, `__has_include`, etc.)
- [ld/2-full](../core/ld/2-full), the full linker (garbage collection, weak symbols, contructors, bss, etc.)
- [libc/3-full](../core/libc/3-full), the missing libc components (floating point math)
- [as/2-full](../core/as/2-full), the full assembler (improved error checking and debug output)

With our modern compiler, we rebuild everything to take advantage of the (few) optimizations in the compiler, linker and libc. This step is not really necessary but it creates smaller and faster binaries and it's a good test of compiler self-hosting. (We build libraries first because they are statically linked into the remaining components.)

- [libc/3-full](../core/libc/3-full), the full libc again
- [libo/1-opc](../core/libo/1-opc), the utility library again
- [ld/2-full](../core/ld/2-full), the full linker again
- [as/2-full](../core/as/2-full), the full assembler again
- [cci/2-full](../core/cci/2-full), the full C compiler again
- [cpp/2-full](../core/cpp/2-full), the full C preprocessor again
- [cc](../core/cc), the driver again

We also build a few ancillary tools:

- [hex/c](../platform/hex/c), replacing our handwritten bytecode
- [ar/1-unix](../core/as/1-unix), the full-featured archive tool

The final compiler driver can now produce Onramp binaries wrapped for native platforms (UNIX and Windows.) You now have a standalone compiler for your system! It can be used as an ordinary C compiler in the compilation process for a native toolchain.



## Language Progression

The Onramp unit tests include a program to generate all solutions to the [eight queens puzzle](https://en.wikipedia.org/wiki/Eight_queens_puzzle). This program exists for most language stages implemented by Onramp. Since the program is functionally identical in each stage, it's useful for illustrating the progression of languages in Onramp.

- [Eight queens in hexadecimal bytecode](../test/vm/programs/eight-queens.oe.ohx). This is the lowest level: everything is done by hand. Note the fixed addresses and padding between symbols. Editing can be painful if you need to move a function or recalculate a relative jump.

- [Eight queens in object code](../test/ld/0-global/programs/eight-queens.oo). Nearly the same as the raw bytecode, but it uses labels instead of fixed addresses. It is much easier to change and fix bugs.

- [Eight queens in primitive assembly](../test/as/0-basic/programs/eight-queens.os). This assembles to the identical program as the object code but it is much more legible. You don't need to memorize the bytecode values to understand and edit the code. Things like function calling, stack manipulation and conditional jumps are still a pain.

- [Eight queens in compound assembly](../test/as/1-compound/programs/eight-queens.os). This is much shorter! It feels more like a CISC machine now. Compound instructions like `call` replace many primitive instructions. Short instructions like `ret`, `inc` and `mov` are extremely clear. Decimal numbers are much more legible than quoted hex. Conditional jumps like `jne` are much easier to follow than the stacked `jz` calls that were needed before. `push` and `pop` replace manual stack pointer manipulation.

- [Eight queens in Onramp Minimal C](../test/cci/0-omc/programs/eight-queens.c). We now have real structured programming! No more labels and jumps. We also have named variables! No more dealing with registers or spilling. No stack manipulation at all. We have function prototypes, include files for the standard library, and more. There are some obvious limitations though: no initializers, no `for` loops, no `else`, no arrays, parentheses everywhere. The queens array must be manually allocated.

- [Eight queens in Onramp Practical C](../test/cci/1-opc/programs/eight-queens.c). This is a much more featureful subset of C with most of what we need for large-scale programs. It looks modern and idiomatic. We have full expression syntax with operator precedence. We have arrays, `for` loops with declarations, etc. We also have `struct`, `enum`, `switch`, `goto`, variadic functions and more, although this code doesn't use them. This is powerful enough to support most of a modern libc including features like `printf()`.

The final stage compiler adds function pointers, initializer lists, `long long`, floating-point math and more. None of these are used in the eight queens puzzle so we don't have an example for the final stage.

If you want to try these, you can compile all of the above with `onrampcc`, except for the first which must be compiled with `onramphex`.



## Step-By-Step

(The below documentation is incomplete and needs to be expanded.)


### Platform-Specific Language

Our first tools are platform-specific. See the [Setup Guide](setup-guide.md) for information on how to set these up.

#### [hex](../platform/hex/)

The hex tool lets us handwrite binary files in commented hexadecimal.

The machine code VMs are written in this commented hexadecimal format so we bootstrap it first. Even if you don't need a hex tool to build your VM you'll still need one to convert the initial Onramp binaries.

#### [vm](../platform/vm/)

Any implementation for your system.



### Onramp Bytecode

We have an Onramp VM! We can now write portable programs in [Onramp bytecode](virtual-machine.md).

We don't have a linker yet so we have to write each tool in a single file of commented hexadecimal. We'll lean on our strategies for [coding without labels](coding-without-labels.md).

#### [hex/onramp](../platform/hex/onramp/)

We build the Onramp implementation of the hex tool. This one is platform-independent and we can use it inside the VM. It also performs all error checking (including address assertions) in case the platform-specific hex tool did not.

#### [sh](../core/sh/)

The Onramp shell, to run the bootstrap shell script inside the VM.

Although these last two tools are platform-independent, building them must be done in a platform-dependent manner. That isn't the case for the rest of this process. Now that we have a shell tool, we can use it to run [this shell script](../core/build.sh) that will perform the entire rest of the bootstrap process inside the VM automatically.

#### [ld/0-global](../core/ld/0-global/)

The basic linker written in hexadecimal bytecode, giving us support for labels



### Onramp Object Code

We have a linker! We can now write programs in [Onramp object code](object-code.md).

This also means we can implement code in multiple files, and we can even re-use code files in different programs. We'll use this to immediately implement a primitive libc so we can stop duplicating functionality like `strcmp()` in each program.

#### [libc/0-oo](../core/libc/0-oo/)

A basic libc. `_start()`/`exit()`, `memcpy()`/`strcmp()`, `fopen()`/`fread()`/`fwrite()`, etc.

The libc is written but we don't actually do anything to compile it yet; it's already in object code. We need another tool to collect it into a static library.

#### [libo/0-oo](../core/libo/0-oo/)

The Onramp utility library used in the implementation of all core tools. This first stage just includes some error handling (e.g. a `fatal()` function) and utilities like `itoa()` and `fnv1a_cstr()`.

As with the libc, there's nothing to build yet.

#### [ar/0-cat](../core/ar/0-cat/)

A static archive tool. This is essentially a glorified `cat`. It combines object files delimited by a bit of metadata into a static archive.

We compile it by manually providing all our libc and libo sources to the linker [here](../core/ar/0-cat/build.sh). This is the only time this needs to be done.

We then immediately use it to form the libc and libo into static libraries. From now on we can just provide these static archives to the linker so we don't have to list all files anymore.

#### [as/0-basic](../core/as/0-basic/)

A basic assembler written in object code.

This just does straight keyword replacement: `add` is replaced with `70`, `rsp` is replaced with `8C`, and so on. It also converts strings to hexadecimal.

Although this tool is quite trivial, assembly is much more readable. We can now write larger scale programs. From this point on, most programs will span across several source files.



### Assembly Language

We finally have an [assembly language](assembly.md)! We don't have to write our opcodes and registers directly in hexadecimal anymore.

We can also write strings normally from this point on. This makes it much easier to provide rich error messages.

#### [as/1-compound](../core/as/1-compound/)

Our first step is to build a more powerful assembler. The compound assembler supports many more opcodes: `call`/`ret`, `enter`/`leave`, `push`/`pop`, signed math, etc.

The resulting assembly language is powerful enough to implement our compiler.

#### [cpp/0-strip](../core/cpp/0-strip/)

This is the first stage C preprocessor. We call it a "preprocessor" but really it just strips comments and preprocessor directives.

This breaks a "chicken-and-egg" dependency in bootstraping our next stage preprocessor that is written in omC. We want it to be portable (to test it with a modern C compiler) but portability requires including C headers. The stripping preprocessor gives us a nice workaround.

In addition to this, being able to strip comments in the preprocessor right away means that no version of our C compiler ever needs to parse comments.

#### [cci/0-omc](../core/cci/0-omc/)

This is the first real C compiler. It's written in compound assembly and it compiles Onramp Minimal C.

We've kept omC as limited as possible in order to minimize the amount of assembly we need to write. This is the final tool implemented in assembly.



### Primitive C

We can finally compile C! Well, something like C, with a whole lot of limitations. From here on out, everything is written in some subset of C.

We don't have a real preprocessor yet, and our linker can't link multiple C files together because their generated label names would clash. Let's solve those problems next.

#### [cpp/1-omc](../core/cpp/1-omc/)

The basic omC preprocessor. Only `#include`, `#ifdef`/`#ifndef` and object-like `#define` are supported. This lets us write typical C include files with header guards, allowing us to write large-scale projects with idiomatic code organization.

#### [ld/1-omc](../core/ld/1-omc/)

Our new linker adds support for file scope.

Until now we've been able to get away with all labels being global because we've been manually giving each label a unique name. We now have a compiler that generates labels; this new linker uses it to avoid clashes between files.

This also fixes variables and functions with `static` linkage. We can now write multi-file C projects.

#### [libc/1-omc](../core/libc/1-omc/)

Now that we can link multiple `.c` files we can re-implement parts of our libc in omC.

This gives us a much more featureful libc. It adds a real `malloc()` and `free()`, among other things.

Most of the functionality of the first-stage libc doesn't need to be re-implemented so this is structured like an overlay. Some files are replaced; others are kept as is. The build process for this archives some files from both stage 0 and stage 1 into a new `libc.oa` static library.

#### [cc](../core/cc/)

The compiler driver. We no longer need to manually run all the individual steps to compile something.



### Upgrading to modern C

#### [cci/1-opc](../core/cci/1-opc/)

The C99-ish compiler we will use to build our final stage C compiler. We call this Onramp Practical C.

We now have a C compiler powerful enough to write "comfortably", in a reasonably modern idiomatic style similar to C99. We use this to write our full C compiler.

#### [libc/2-opc](../core/libc/2-opc/)

The nearly full libc. It adds some features we need for our full C compiler, in particular `printf()`.

#### [cci/2-full](../core/cci/2-full/)

This is the full C compiler.

#### [cpp/2-full](../core/cpp/2-full/)

The full-featured C preprocessor (`#if`, variadic function-like macros, `#embed`, `__has_include`, etc.)


### Modern C

We now have a working compiler for modern C, but there are still a few features missing for a complete implementation.

Thankfully, since we have full C support now, we can write everything in modern C.

#### [libc/3-full](../core/libc/3-full/)

The full libc. This mainly adds floating point math.


### Rebuilding Everything

We need to recompile everything with itself to get a properly optimized build.

Proof of self-hosting, optimizations, actually using the libc, getting a wrapped `onrampcc` for normal use

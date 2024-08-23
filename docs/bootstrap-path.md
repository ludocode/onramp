# Onramp Boostrap Path

This document describes the bootstrap process that Onramp takes to get from nothing all the way to a full implementation of C.



## Overview

First we need a hex tool and virtual machine. These steps are platform-specific.

- [hex](../platform/hex), any implementation for your system
- [vm](../platform/vm), any implementation for your system

We also need a few tools to run the bootstrap scripts. These tools are platform-independent but they need to be bootstrapped in a platform-specific manner.

- [hex/0-onramp](../core/hex/0-onramp), replacing the native tool, with improved error checking and the ability to run inside the VM.
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

This adds the important features of C99: struct, enum, arrays, switch, for loops, the full expression syntax, `printf()` and friends, and many other things we will need to build a real compiler. We have modern conveniences like mixed declarations and code and C++-style comments.

We can now build our feature-complete C compiler. It requires the final linker and utility library so we build those first.

- [libo/1-opc](../core/libo/1-opc), additional utilities for compiler development
- [ld/2-full](../core/ld/2-full), the full linker (contructors, bss, weak symbols, garbage collection, etc.)
- [cci/2-full](../core/cci/2-full), the full C compiler
- [cpp/2-full](../core/cpp/2-full), the full C preprocessor (`#if`, variadic function-like macros, `#embed`, `__has_include`, etc.)

Now that we have a full C compiler, we can build up the last few bits of our toolchain.

- [libc/3-full](../core/libc/3-full), the missing libc components (things that need function pointers for example)
- [as/2-full](../core/as/2-full), the full assembler (improved error checking, debug output and minor optimizations)

With our full toolchain built, we rebuild everything to take advantage of the (few) optimizations in the final stage compiler, linker, assembler and libc. This step is not really necessary but it creates smaller and faster binaries and it's a good test of compiler self-hosting. (We build libraries first because they are statically linked into the remaining components.)

- [libc/3-full](../core/libc/3-full), the full libc again
- [libo/1-opc](../core/libo/1-opc), the utility library again
- [ld/2-full](../core/ld/2-full), the full linker again
- [as/2-full](../core/as/2-full), the full assembler again
- [cci/2-full](../core/cci/2-full), the full C compiler again
- [cpp/2-full](../core/cpp/2-full), the full C preprocessor again
- [cc](../core/cc), the driver again

We also build a few ancillary tools:

- [hex/1-c89](../core/hex/1-c89), replacing our handwritten bytecode
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

The final stage compiler adds function pointers, initializer lists, `long long`, floating-point math, preprocessor expressions, function-like macros and more. None of these are used in the eight queens puzzle so we don't have an example for the final stage.

If you want to try these, you can compile all of the above with `onrampcc`, except for the first which must be compiled with `onramphex`.



## Step-By-Step

This is a full step-by-step explanation of the bootstrap process. It is grouped by the language in which each stage is implemented.



### Platform-Specific Language

Our first tools are platform-specific. See the [Setup Guide](setup-guide.md) for information on how to set these up.

#### [hex](../platform/hex/)

The hex tool lets us handwrite binary files in commented hexadecimal.

The machine code VMs are written in this commented hexadecimal format so we bootstrap it first. Even if you don't need a hex tool to build your VM you'll still need one to convert the initial Onramp binaries.

#### [vm](../platform/vm/)

The virtual machine, any implementation for your system.

Typically, in a hosted environment, this tool will bridge the native filesystem into the virtual machine environment. Onramp-compiled programs, including the toolchain itself, can therefore access the host filesystem just like native programs.

Everything from here on out is designed to run on the Onramp virtual machine.



### Onramp Bytecode

We have an Onramp VM! We can now write portable programs in [Onramp bytecode](virtual-machine.md).

We don't have a linker yet so we have to write each tool in a single file of commented hexadecimal. We'll lean on our strategies for [coding without labels](coding-without-labels.md).

#### [hex/0-onramp](../core/hex/0-onramp/)

The Onramp implementation of the hex tool. This one is platform-independent and we can use it inside the VM. It also performs all error checking (including address assertions) in case the platform-specific hex tool did not.

#### [sh](../core/sh/)

The Onramp shell, to run the bootstrap shell script inside the VM.

Although these last two tools are platform-independent, building them must be done in a platform-dependent manner. That isn't the case for the rest of this process. Now that we have a shell tool, we can use it to run [this shell script](../core/build.sh) that will perform the entire rest of the bootstrap process inside the VM automatically.

#### [ld/0-global](../core/ld/0-global/)

The basic linker is the last program written purely in hexadecimal. It gives us basic support for defining labels and resolving addresses to them.

This lets us finally move away from counting bytes and maintaining addresses by hand.



### Onramp Object Code

We have a linker! We can now write programs in [Onramp object code](object-code.md).

This also means we can implement code in multiple files, and we can even re-use code files in different programs. We'll use this to immediately implement a primitive libc so we can stop duplicating functionality like `strcmp()` in each program.

#### [libc/0-oo](../core/libc/0-oo/)

A basic libc. `_start()`/`exit()`, `memcpy()`/`strcmp()`, `fopen()`/`fread()`/`fwrite()`, a primitive `malloc()`, etc.

We don't actually need to do anything to compile this code yet. It's already in object code so we can pass the handwritten files directly to the linker.

However, we don't want to have to specify all of the libc source files every time we build something. We need another tool to collect it into a static library.

#### [libo/0-oo](../core/libo/0-oo/)

The Onramp utility library used in the implementation of all core tools. This first stage just includes some error handling (e.g. a `fatal()` function) and utilities like `itoa()` and `fnv1a_cstr()`.

As with the libc, there's nothing to build yet. It's just handwritten object files.

#### [ar/0-cat](../core/ar/0-cat/)

A static archive tool. This is essentially a glorified `cat`. It combines object files delimited by a bit of metadata into a static archive.

We compile it by manually providing all our libc and libo sources to the linker [here](../core/ar/0-cat/build.sh). This is the only time this needs to be done.

We then immediately use it to form the `libc.oa` and `libo.oa` static libraries. From now on we can just provide static library archives to the linker so we don't have to list all the library files anymore.

#### [as/0-basic](../core/as/0-basic/)

A basic assembler. This is the final tool written in object code.

This just does straight keyword replacement: `add` is replaced with `70`, `rsp` is replaced with `8C`, and so on. It also converts strings to hexadecimal.

Although this tool is quite trivial, the assembly it lets us write is much more readable. We can now write larger scale programs. From this point on, most programs will span across several source files.



### Assembly Language

We finally have an [assembly language](assembly.md)! We don't have to write our opcodes and registers directly in hexadecimal anymore.

We can also write strings normally from this point on. This makes it much easier to provide rich error messages.

#### [as/1-compound](../core/as/1-compound/)

Our first step is to build a more powerful assembler. This is the only program written in primitive assembly.

The compound assembler supports many more opcodes: `call`/`ret`, `enter`/`leave`, `push`/`pop`, signed math, etc.

The compound assembly language is powerful enough to implement our first C compiler.

#### [cpp/0-strip](../core/cpp/0-strip/)

This is the first stage C preprocessor. We call it a "preprocessor" but really it just strips comments and preprocessor directives.

This breaks a "chicken-and-egg" dependency in bootstraping our next stage preprocessor. We'd really prefer to write it in C (rather, [omC](minimal-c.md)) instead of assembly, and we want it to be somewhat portable (so we can test it with a modern C compiler.) But portability requires including C headers, which is not possible until the preprocessor exists!

The stripping preprocessor gives us a nice workaround. Instead of performing the `#include`s, it just ignores them and lets us declare things manually instead.

In addition to this, being able to strip comments in the preprocessor right away means that no version of our C compiler ever needs to parse comments.

#### [cci/0-omc](../core/cci/0-omc/)

This is the first real C compiler. It's written in compound assembly and it compiles [Onramp Minimal C (omC)](minimal-c.md).

We've kept omC as limited as possible in order to minimize the amount of assembly we need to write. This is the final tool implemented in assembly.



### Minimal C

We can finally compile C! Well, something like C, with a whole lot of limitations. It doesn't even have structs, so it's probably closer to [B](https://en.wikipedia.org/wiki/B_(programming_language)) than C. In some ways it's even more limited because it doesn't have operator precedence.

Still, we have genuine structured programming! We don't need to worry about labels, registers, instructions, stack frames, or addressing modes anymore. From here on out, everything is written in some subset of C.

We don't have a real preprocessor yet, and our linker can't link multiple C files together because their generated label names would clash. Let's solve those problems next.

#### [cpp/1-omc](../core/cpp/1-omc/)

The basic omC preprocessor. Only `#include`, `#ifdef`/`#ifndef` and object-like `#define` are supported. This is just enough to support typical C header files with include guards, allowing us to write large-scale programs with idiomatic code organization.

#### [ld/1-omc](../core/ld/1-omc/)

Our new linker adds support for file scope.

Until now we've been able to get away with all labels being global because we've been manually giving each label a unique name. We now have a compiler that generates labels, so we need file scope. This new linker uses file scope to avoid clashes between compiled translation units.

This also correctly handles variables and functions with `static` linkage. We can now write multi-file C programs.

#### [libc/1-omc](../core/libc/1-omc/)

Now that we can link multiple `.c` files we can re-implement parts of our libc in omC.

This gives us a more featureful libc. It adds a real `malloc()` with a coalescing `free()`, which is important because our programs will start to use quite a bit more memory.

Most of the functionality of the first-stage libc doesn't need to be re-implemented so this is structured like an overlay. Some files are replaced; most are kept as is. The build script archives some files from both stage 0 and stage 1 into a new `libc.oa` static library.

#### [cc](../core/cc/)

The compiler driver. We no longer need to manually run all the individual steps to compile something.

The driver runs the various programs: `cpp`, `cci`, `as`, `ld`. It also links against the libc and provides access to its headers. By default it expects all of these to be their final stages, installed in their final locations. None of these exist yet!

Therefore, we need to pass options to `cc` to manually specific each tool it should use: `-with-cpp`, `-with-cci`, `-nostdlib` and so on. To shorten our build scripts, `cc` can take command-line arguments from a file.

All programs from here on out use `cc` with a `build-ccargs` file. It contains command-line options to specify the tools to use for each particular build.

#### [cci/1-opc](../core/cci/1-opc/)

This is the C99-ish compiler we will use to build our final stage C compiler. This is the last tool implemented in omC.



### Practical C

We can now write [Onramp Practical C (opC)](practical-c.md)! This is very much like modern, idiomatic C99.

We still have some important limitations. There are no function pointers, no compound initializers, no `long long` and more. We also haven't built a corresponding libc yet. We do that next.

#### [libc/2-opc](../core/libc/2-opc/)

The nearly full libc. It adds some features we need for our full C compiler, in particular `printf()`, and 64-bit and floating-point math functions. It adds buffered file I/O which is much faster under certain (unbuffered) VMs.

This is again structured like an overlay. We only build the files that are new; we link in previously compiled objects from libc/1 and hand-written objects from libc/0 into a new `libc.oa`.

This libc adds a couple new initialization features: constructor/destructor symbols and zero symbols (bss). These rely on special symbols that are only generated by the final stage linker. Without them, we can't link anything against this libc!

That's okay, because the next program we'll build is a new linker. But first we need a new helper library.

#### [libo/1-opc](../core/libo/1-opc/)

This is a more complete utility library. It gives us an intern string, a hash table, a vector and more. All tools from here on out use this.

#### [ld/2-omc](../core/ld/2-full/)

The final stage linker. It adds constructor/destructor symbols, zero symbols (bss), weak (or COMDAT) symbols, garbage-collection, and more. Some of these features are required for our final stage compiler, and the previous libc needs this linker, so we build it now.

We have another "chicken-and-egg" problem here. This linker depends on libc/2 because it uses `printf()` among other things. However, libc/2 depends on this linker because we generate the special constructor/destructor/zero symbols needed by libc/2. Without these symbols, the libc will fail to link.

We break this dependency by manually writing the symbols into the C code that would ordinarily be generated by this linker. They are activated by a simple `#ifdef`.

#### [cci/2-full](../core/cci/2-full/)

This is the full C compiler.

This is the only program where the limitations of opC really matter. The compiler can't use `long long`, `float` or `double` in its own implementation; instead it has a `u64_t` struct to store 64-bit values, and it makes and emits manual calls to math functions in libc/2 (e.g. `__llong_add()`, `__float_mul()`, etc.) We also can't use function pointers; in cases where they'd be useful (like tables of builtins), we have to use enums and switches instead.

This is the last time we have these limitations. From here on out, we have full, modern C.



### Modern C

We finally can write real C! Well, the language anyway. We don't have the full preprocessor yet, and we're missing lots of libc functions. We build those next.

#### [cpp/2-full](../core/cpp/2-full/)

This is the full-featured C preprocessor (`#if`, variadic function-like macros, `#embed`, `__has_include`, etc.)

This is the first tool we build with our full C compiler. It uses 64-bit math and function pointers naturally.

This is also the last tool that is limited to the omC preprocessor. From here on out we can write function-like macros without fallbacks.

#### [libc/3-full](../core/libc/3-full/)

The full libc. We are again only building the additional C files, linking in other objects compiled in previous stages.

This mainly adds functions that take function pointers such as `atexit()` and `qsort()`.

#### [as/2-full](../core/as/2-full/)

The full assembler.

Unlike our full linker which is written in opC, the full assembler requires the full C compiler. It makes extensive use of array initializers and function pointers which are not available in opC. It's the last tool we bootstrap in our core toolchain.



### Rebuilding Everything

Now that we've bootstrapped all our final stage core tools, we recompile the entire toolchain.

This step isn't technically necessary but there are good reasons to do this. For one, it proves that the final toolchain is self-hosting. More importantly, we can build everything with optimizations!

- Our final preprocessor can inline small accessor macros that were previously functions.
- Our final compiler doesn't produce great code but it's a lot better than the simple stack machines of previous stages.
- Our final assembler can perform some small peephole optimizations.
- Our linker can garbage collect unused symbols.

The toolchain will be significantly smaller and faster if we rebuild it.

We build libraries first because they're statically linked into the executables. Since our libc stages are overlaid, we previously only built the additional files in each stage. This time we rebuild all necessary parts from all stages all at once.

- [libc/3-full](../core/libc/3-full), the full libc again
- [libo/1-opc](../core/libo/1-opc), the utility library again

We can now rebuild our entire toolchain, linked against the optimized libraries we just built. The order we build these in doesn't really matter.

- [ld/2-full](../core/ld/2-full), the full linker again
- [as/2-full](../core/as/2-full), the full assembler again
- [cci/2-full](../core/cci/2-full), the full C compiler again
- [cpp/2-full](../core/cpp/2-full), the full C preprocessor again
- [cc](../core/cc), the driver again



### Additional Tools

Lastly, we build some additional tools that we provide as part of the Onramp toolchain.

#### [hex/1-c89](../core/hex/1-c89)

Our first hex tool was native and platform specific; our second was in Onramp bytecode. This final hex tool is written in C. This is wrapped as `onramphex` and installed as part of the toolchain.

The compiled code is not necessarily faster than what we had previously handwritten (since our compiler isn't great at optimization) but this one will use the buffered I/O functions in our final libc. This can make a dramatic difference in performance on unbuffered VMs, where reading and writing would otherwise require a kernel system call for each individual byte. It also provides much better error messages, making it easier to create new Onramp hex files.

It is useful to extend Onramp, for example with machine code implementations of `hex` and `vm` for new platforms. It could also be useful if you want to use Onramp Hexadecimal to write your own binary files unrelated to Onramp.

#### [ar/1-unix](../core/as/1-unix)

Our first stage archiver only supported `rc` mode for creating new archives. That was all that was needed to bootstrap Onramp.

However, many programs rely on separately-compiled libraries and perform various manipulations on them, so we probably do need a real archive tool as part of our toolchain. This tool is written in C, so we can build it now.

This tool is wrapped as `onrampar` and installed as part of the toolchain.

### Done!

At this point the full toolchain is built. A platform-specific mechanism can now install the toolchain, potentially with wrappers to be able to run the Onramp tools natively.

The following native wrappers are typically installed:

- `onrampvm` -- The virtual machine, the only native program we need
- `onrampcc` -- The compiler driver, wrapping `cc.oe`
- `onrampar` -- The archiver, wrapping `ar.oe`
- `onramphex` -- The hex tool, wrapping `hex.oe`

We also install all of the Onramp binaries (`cpp.oe`, `cci.oe`, `ar.oe`, `ld.oe`), the libc archive (`libc.oa`) and the libc headers. The driver will access all of these as needed to compile programs.

The [Setup Guide](setup-guide.md) describes how to install Onramp on contemporary operating systems, and how installation and operating system integration might work on future and alien platforms.

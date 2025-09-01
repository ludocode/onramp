# Onramp Code Generator - First Stage

This is the code for the first stage Onramp code generator.

The first stage code generator takes a form of [Onramp assembly](../../../docs/assembly.md) emitted by early stage [Onramp compilers](../../cci/) as input, optimizes it, and outputs it in general Onramp assembly.

The code generator is still somewhat limited. The goal is to make enough optimizations that it reduces the instruction count of the compiler output by 2x or more, which should more than make up for the time it takes to run the code generator after every compilation. It will be worth the couple thousand extra lines of code if it can significantly reduce the overall time it takes to bootstrap Onramp.



## Bootstrapping a Code Generator

The first stage compiler, by virtue of being written in assembly, emits [terribly inefficient code](../../cci/0-omc/README.md#expression-evaluation) so there is a lot of opportunity for simple optimizations. This code generator is essentially just a peephole optimizer for it.

This code generator doesn't work on arbitrary assembly however. It makes various assumptions about the input (for example, it assumes that registers r4-r8 are unused), so it only works on assembly output by the early stage Onramp compilers. In this sense the input really is an intermediate representation, not full assembly but a restricted subset of it. This code generator accepts it and turns it into general assembly (one that uses all registers for example.)

But the fact that the compiler output is itself valid assembly makes it possible to compile this code generator without first needing a code generator! This is how we break the chicken-and-egg problem without having to write a code generator in assembly. The [build script for this code generator](build.sh) is interesting: we compile it into an unoptimized executable, then immediately run it on its own assembly and re-link to produce an optimized build without having to recompile.



## Code Organization

The first stage code generator is bootstrapped before the second stage linker. This means we don't have local label support, so we have to fit everything in one translation unit. However, the stage is fairly large, so we bootstrap it after the second stage preprocessor in order to use include files to organize it.

This means the code layout is a bit unconventional: almost all of the code is in header files only. These still include each other naturally. If there would be a circular dependency between header files, we move the necessary declarations into `common.h`, which is ultimately included first.

This code is written in [Onramp Minimal C (omC)](../docs/minimal-c.md). Since we don't have structs, it uses "emulated structs" [as described in cci/1](../../cci/1-opc/README.md#emulated-structs) to store instructions.

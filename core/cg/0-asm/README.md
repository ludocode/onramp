# Onramp Code Generator - First Stage

This is the code for the first stage Onramp code generator.

The first stage code generator takes a form of [Onramp assembly](../../docs/assembly.md) emitted by early stage [Onramp compilers](../../cci/) as input, optimizes it, and outputs it in general [Onramp assembly](../../docs/assembly.md).

It is essentially just a peephole optimizer. The first stage compiler, by virtue of being written in assembly, emits [terribly inefficient code](../../cci/0-omc/README.md#expression-evaluation) so there is a lot of opportunity for simple optimizations.

The code generator is incomplete; it currently just parses and outputs the assembly unchanged. The goal is to make enough optimizations that it reduces the instruction count of the compiler output by 2x or more, which should more than make up for the time it takes to run the code generator after every compilation. It will be worth the couple thousand extra lines of code if it can significantly reduce the overall time it takes to bootstrap Onramp.

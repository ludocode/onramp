# Onramp Code Generator

`cg` is the Onramp code generator. It takes the output of the [Onramp Compiler](../cci/) as input and outputs optimized [Onramp Assembly](../../docs/assembly.md).

The intermediate representation used by the code generator depends on the stage.

- [`0-asm`](0-asm/) is written in [Onramp Minimal C (omC)](../../docs/minimal-c.md) and takes a form of Onramp Assembly as input. It is designed to be run on the output of [`cci/0`](../cci/0-omc/) and [`cci/1`](../cci/1-opc/), the first and second stages of the Onramp compiler. It is similar to a peephole optimizer: it translates the "stack machine" assembly produced by the compilers into something more efficient.

- [1-full](1-full/) is written in [Onramp Practical C (opC)](../../docs/practical-c.md) and takes [Onramp Intermediate Representation (IR)](../../docs/intermediate-representation.md) as input. It performs register allocation and stack frame layout along with various optimizations. It is designed to be run on the output of [`cci/2`](../cci/2-full/), the final stage Onramp compiler.

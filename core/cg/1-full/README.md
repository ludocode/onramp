# Onramp Code Generator - Final Stage

This is the final stage Onramp code generator. It takes [Onramp IR](../../../docs/intermediate-representation.md) as input and outputs optimized [Onramp Assembly](../../../docs/assembly.md).

Unlike the [previous stage code generator](../0-asm) which took assembly as input, this one takes IR from [cci/2](../../cci/2-full/) as input. Its primary purpose is to perform stack frame layout and register allocation. Its secondary purpose is to optimize the IR and assembly.



## Overview

The code generator transforms one function at a time. For each function, it performs the following steps:

- The IR of the function is parsed into memory;
- The IR is optimized;
- Register allocation and stack frame layout is performed;
- The IR is transformed to assembly;
- The assembly is optimized;
- The assembly is emitted to the output file.

Once the function is emitted, all data is freed and the code generator proceeds to the next function.

The IR is represented in memory as a set of basic blocks, each containing a sequence of instructions. An instruction's arguments can be numbers, temporaries (when representing IR), or registers (when representing assembly.)

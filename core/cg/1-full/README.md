# Onramp Code Generator - Final Stage

This is the final stage Onramp code generator. It takes [Onramp IR](../../../docs/intermediate-representation.md) as input and outputs optimized [Onramp Assembly](../../../docs/assembly.md).

Unlike the [previous stage code generator](../0-asm) which took assembly as input, this one takes IR from [cci/2](../../cci/2-full/) as input. Its primary purpose is to perform register allocation and stack frame layout. This greatly simplifies the compiler and opens up new possibilities for optimization.



## Overview

The code generator transforms one function at a time. For each function, it performs the following steps:

- The IR of the function is parsed into memory.
- Variables are created, along with the function preamble and epilogue (parameter passing, `enter`/`leave`, `ret` transformation.)
- The IR is (optionally) optimized.
- Register allocation and stack frame layout are performed. Additional variables are created for spilled temporaries.
- The IR is transformed into assembly, spilling temporaries, frame offsets and mix-type bytes.
- The assembly is (optionally) optimized.
- The assembly is emitted to the output file.
- All data is freed and we move on to the next function.

The IR is represented in memory as a set of basic blocks, each containing a sequence of instructions.



## Variable Creation

Each `var` instruction, each parameter, and each spilled temporary creates a variable. A variable is a fixed offset from the frame pointer representing space for data in the stack frame.

Parameters and `var` instructions are replaced with `add` instructions that add the variable offset to `rfp`. For example, given the following function:

```c
void foo(x) {
    int y = 1;
    // ...
}
```

The (simplified) IR is something like:

```asm
=foo
    %x
:_L1
    var %y 4 %
    stw 1 %y
    ; ...
```

Immediately after parsing, variables are assigned to each parameter and `var`, and they are converted to instructions like this:

```asm
=foo
    enter
    stw r0 rfp @x
    add %x rfp @x
:_L1
    add %y rfp @y
    stw 1 %y
    ; ...
```

Here `@x` and `@y` are variables: they represent a constant number, a fixed offset in the stack frame where data can be stored. These numbers are mostly not assigned yet so we represent them in debug output with `@`. Most offsets are negative, but parameters beyond the first four are assigned positive offsets because they are passed on the stack.

Optimizations can then eliminate many of these temporaries, and in some cases also the variables. The register allocator then runs. If temporaries need to be spilled, the register allocator creates additional variables for them.



## Register Allocator

The current register allocator is a simple depth-first linear scan.

First, an iterative liveness analysis is performed. Each block is annotated with the set of temporaries that may be live at the end of the block. (Liveness per-instruction can then be generated simply by walking up the block.)

Next, live interval analysis is performed. The function is scanned forward depth-first assigning each instruction an index. Each block is then scanned backwards, tracking the bounds of the ranges (the interval) in which each temporary is live. The temporaries are then sorted by start, and each temporary is either assigned a register or spilled.

Note that no transformations are made by the register allocator. It is an analysis pass only.

Registers r0-r7 are used for live temporaries, so up to 8 temporaries can be live at a time. The linear scan algorithm allocates as many temporaries as possible to these dedicated registers.



## Stack Frame Layout

The stack frame layout pass assigns an offset in the stack frame to every variable and calculates the total size of the stack frame.

Currently this is as simple as possible: every variable is given a distinct location in the stack frame in arbitrary order. The algorithm makes no attempt to share stack space for variables that don't overlap.



## Conversion from IR to Assembly

Once analysis is done, a single transformation pass performs most of the conversion from IR to assembly. This includes:

- Replacement of temporaries with registers, inserting loads and stores for spilled temporaries
- Replacement of variables with fixed frame offsets
- Inserting immediate loads for numbers (including variable frame offsets) that don't fit in a mix-type byte

The reason we do these together is because all of these may need an extra spill register to store a value. Registers r8-r9 are used for spilled temporaries and for immediate numbers that don't fit in a mix-type byte (the code sometimes refer to these as spills as well.)

Almost all instructions have at most two inputs and at most one output so we only need these two extra registers. If inputs to the instruction are spilled, r8 and r9 are loaded with their values before the instruction. If the output of the instruction is spilled, it is placed in r8 and stored afterwards.

For example, given the following instruction:

```asm
add %x %y %z
```

Suppose all three temporaries are spilled: `%x` is at -4, `%y` is at -8 and `%z` is at -12. `ldw` instructions are prepended for the inputs and `stw` instructions are prepended for the output like so:

```asm
ldw r8 rfp -8
ldw r9 rfp -12
add r8 r8 r9
stw r8 rfp -4
```

The same is true for large frame offsets and immediate valeus that don't fit in a mix-type byte. For example:

```asm
add %x @1 200
```

Suppose `%x` is live in `r4` and `@1` is frame offset -300. The result of the transformation is:

```asm
imw r8 -300
imw r9 200
add r4 r8 r9
```

In the case of spilled temporaries, the offset of the variable may not fit in a mix-type byte. In this case we need additional instructions to set the offset. Inputs use the same register and the sole output uses r9 (since r8 is used for the result of the instruction.) For example, suppose our `add %x %y %z` instruction now has `%x` at -200, `%y` at -300 and `%z` at -400. The result is:

```asm
imw r8 -300
ldw r8 rfp r8
imw r9 -400
ldw r9 rfp r9
add r8 r8 r9
imw r9 -200
stw r8 rfp r9
```

The above works for all two-input instructions. The only exceptions to the two input limit are store instructions and the call instruction; these are described below.

### Store Instructions

The store instructions take three inputs, but we only have two registers (r8 and r9) for loading spilled temporaries. The store instruction therefore has special handling in register allocation.

If at most two inputs are spilled, r8 and r9 are used for the spilled inputs; live inputs come directly from their registers. This is the common case, and we don't need to do anything else.

In the rare case that all three inputs are spilled, we perform the addition separately. The base and offset are loaded into r8 and r9, and an add instruction is inserted which outputs to r9. Then the value is loaded into r8 and the store instruction stores it to r9.

For example, given the following:

```asm
stw %x %y %z
```

Suppose `%x` and `%z` were spilled to -4 and -12 respectively, while `%y` is in r3. The result would be:

```asm
ldw r8 rfp -4
ldw r9 rfp -12
stw r8 r3 r9
```

However, if `%y` were also spilled to -8, the code is expanded to:

```asm
ldw r8 rfp -8
ldw r9 rfp -12
add r9 r8 r9
ldw r8 rfp -4
stw r8 0 r9
```

### Call Instruction

A call instruction is replaced with the following steps:

- Live variables in registers r0-r7 are stored to the stack. (The register allocator creates variables for them even though they are live to ensure they can be preserved across a function call.)
- Arguments beyond the first four are pushed to the stack.
- If the function call is indirect (and the temporary is not in registers r4-r9), the function address is loaded into r4, permuting registers where necessary.
- The first four arguments are placed in registers r0-r3, permuting registers where necessary.
- The call is performed.
- Live variables in registers r0-r7 are restored from the stack.

There's a bit of tricky register permutation necessary. For example if the first argument is in r1, the second argument is in r2, and the third argument is in r0, these registers need to be rotated. r8 and r9 are used to break cycles.

The quality of the resulting assembly isn't great but it works. We rely on optimization of the final assembly (similar to cg/0) to clean up the results. For example, register renaming is used to move the computation of function arguments directly into registers r0-r3.

(An eventual optimization pass will tentatively assign temporaries to the registers they need to occupy in call instructions to reduce register moves. This is not yet implemented.)

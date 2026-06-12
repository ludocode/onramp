# Onramp Intermediate Representation

Onramp Intermediate Representation (IR) is a language used for code generation in the Onramp compiler. A `.oir` file contains Onramp IR.

The [final stage compiler (cci/2)](../core/cci/2-full/) consumes preprocessed C source code and produces Onramp IR. The [final stage code generator (cg/1)](../core/cci/1-full/) consumes Onramp IR and produces [Onramp Assembly](assembly.md).

(Note that earlier stages of the compiler and code generator do not use this IR. Instead they use a subset of Onramp Assembly as intermediate language.)

The primary task of the final stage code generator is to perform register allocation and stack manipulation. A secondary task is optimization. The intermediate representation is designed to facilitate these tasks.

This document is a draft. Most of this is not implemented yet; in the meantime cci/2 is generating assembly directly.



## Overview / Comparison to Onramp Assembly

Onramp IR is very similar to Onramp Assembly, both in syntax and semantics. It is an untyped low-level assembly-like language. The main differences are:

- Instead of a finite set of registers, Onramp IR has unlimited temporaries. Temporaries are prefixed with `%`. Register allocation is performed by the code generator.

- Labels begin basic blocks. Control flow instructions can only occur at the end of basic blocks, and all basic blocks end in a control flow instruction.

- Stack manipulation is performed by the code generator. There are no `push`/`pop` or `enter`/`leave` instructions, no access to the `rsp` or `rfp` registers, etc. Instead `var` and `alloc` instructions reserve space in the stack frame.

- Instructions that take mix-type arguments can take any 32-bit integer. They are not limited to the range of a mix-type byte.

- Load and store instructions take only a single address argument in IR (as opposed to a base and offset pair in assembly.)

- Function call arguments do not need to be manually placed in registers and on the stack. Instead, an arbitrary number of arguments can be passed to the `call` instruction:

```asm
call %result ^strcmp %left %right end
```

Onramp IR is designed primarily to be easy. It is easy to emit from a compiler; easy to read and debug; easy to make simple optimizations; and easy to translate to assembly. It is not designed for more sophisticated optimizations such that might require a more complicated representation.

Like Onramp Assembly, Onramp IR is untyped. All temporaries contain 32-bit integers, and all immediate values are 32-bit integers. (This is very much unlike contemporary IRs like LLVM.)



## Three-Address Code

Onramp IR is essentially a three-address code. The only real difference compared to a traditional three-address code is that the syntax is more like assembly language. Here's a table showing examples of three-address code and the same code in Onramp IR.

|------------------------|-----------------------|
| Three-Address Code     | Onramp IR             |
|------------------------|-----------------------|
| `x = y + z`            | `add %x %y %z`        |
| `x = y & z`            | `and %x %y %z`        |
| `x = y << z`           | `shl %x %y %z`        |
| `x = y ^ z`            | `xor %x %y %z`        |
|------------------------|-----------------------|
| `x = y`                | `mov %x %y`           |
| `x = -y`               | `sub %x 0 %y`         |
| `x = !y`               | `isz %x %y`           |
| `x = !!y`              | `bool %x %y`          |
|------------------------|-----------------------|
| `x = &y`               | `mov %x %%y` \*       |
| `x = *y`               | `ldw %x %y`           |
| `*x = y`               | `stw %y %x` \*        |
|------------------------|-----------------------|
| `goto L1`              | `jmp &L1`             |
|------------------------|-----------------------|
| `if x < y goto L1`     | `lts %_T1 %x %y`      |
| `goto L2`              | `br %_T1 &L1 &L2`     |
|------------------------|-----------------------|

\* By convention, if a temporary `%y` is backed by a variable, the address of that variable is stored in another temporary called `%%y`. There is no way to get the address of a temporary.

\* Note the different order of arguments in load and store instructions. For all load and store instructions, the first argument is the temporary value and the second argument is the address. This order is the same as Onramp assembly; the difference with assembly is that there is only a single address argument instead of a base and offset pair.



## Temporaries

Instead of a finite set of registers, Onramp IR has unlimited temporaries.

A temporary always contains a 32-bit integer. (There are no types in Onramp IR.) A temporary is a generalized form of a register; the main difference is that unlimited temporaries may be used. The code generator will assign registers to temporaries and spill temporaries to the stack as needed.

A temporary is represented by one or more `%` followed by an identifier (which may start with a digit.) Temporaries do not have to be declared before being used.

For example, to place the value 5 in a temporary `foo`:

```asm
mov %foo 5
```

Temporaries are local to their containing symbol. All occurences of a temporary identifier in a symbol refer to the same temporary. Occurences of a temporary identifier in different symbols refer to different temporaries.



## Local variables

Local variables in C are typically placed on the stack.

The compiler tries to use the names of variables for the temporaries that contain their values. (This is not always possible due to shadowing, among other things.) By convention, it uses an additional `%` to indicate a pointer to the storage of a variable.

For example:

```c
int foo = 5 + 3 * 2;
```

The above would be compiled (with optimizations disabled) to the following:

```asm
mul %1 3 2
add %foo 5 %1
var %%foo 4 %
stw %foo %%foo
```

In the above code:

- The computation is performed using a generated temporary `%1`.
- The result is stored in temporary `%foo`.
- A variable is allocated on the stack where `%%foo` is a pointer to it.
- Finally, the value `%foo` is stored in the variable.

The C expression `&foo` is simply the temporary `%%foo`. If the address of the variable is never taken, `%%foo` will never be read, so the variable can be optimized away.

As seen above, the compiler generates numbered temporaries for anonymous intermediate r-values. Variables for these are never created because it is not possible to take the address of r-values.



## Arguments

Function parameters are named after the symbol declaration and before the first label.

Parameters are given as *variables*: storage for them is already allocated as though by `var` instructions and the parameter value is stored at that location. (This is done automatically for all parameters because the compiler would do it anyway.)

Therefore, named parameters are actually *pointers* to the arguments given to the function. They are named with a `%%` prefix by convention.

For example, in a function like this:

```c
int add(int x, int y) {
    return x + y;
}
```

The compiler will emit code like this:

```asm
=add
    %%x %%y
:_L1
    ldw %x %%x
    ldw %y %%x
    add %1 %x %y
    ret %1
```

In the above, `%x` is the value of the first parameter, and `%%x` is a pointer to its storage on the stack. These are used by the compiler like ordinary variables: whenever the compiler changes the value of `%x`, it will also emit a store to `%%x`, and whenever it uses the value `%x`, it will first emit a load of `%%x`.

If the address of `x` is taken in the C code, `&x` is simply the value `%%x`. If the address of `x` is never taken, the loads and stores to `%%x` may be optimized away.

Parameters can be ignored with the sentinel `%` for the value, the variable, or both. This is usually used for unnamed parameters.

In the standard Onramp call convention, the first four arguments are passed in registers and additional arguments are passed on the stack. The code generator will allocate variables for the first four arguments; if they are unused, they will be optimized away. The compiler will map any additional arguments to their position above the frame pointer.

Note that in the standard calling convention, non-primitive types and types larger than 32 bits are passed indirectly: the caller allocates storage and passes a pointer to it. Therefore, the parameter variable is a pointer to the pointer. For example:

```c
struct point {int x, y};
int taxicab_length(struct point p) {
    return p.x + p.y;
}
```

The compiler will emit:

```asm
=taxicab_length
    %%%p
:_L1
    ldw %%p %%%p
    ldw %x %%p
    add %1 %%p 4
    ldw %y %1
    add %2 %x %y
    ret %2
```



## Variadic Arguments

If a function takes variadic arguments, the keyword `varargs` should precede a final temporary that will contain the address of the first variadic argument. By convention this variable is named `%%_Vargs` by the compiler. For example:

```c
int fprintf(FILE* file, const char* format, ...) {
    // code
}
```

The above would be compiled as:

```asm
=printf
    %%file %%format varargs %%_Vargs
:_L1
    ; function body
```

The code can load from `%%_Vargs` and decrement it to extract the variadic parameters.



## Sentinel

The plain `%` sigil without an identifier (i.e. followed by whitespace) is used as a sentinel value. It has several purposes:

- To ignore the return value of a function. For example:

```asm
; puts(str);
call % puts %str end
```

- To ignore a function parameter. For example:

```asm
; void free_sized(void* ptr, size_t) {free(ptr);}
=free_sized
    %%ptr %
:_Lstart
    call % ^free %ptr %
    ret %
```

- To represent no value. For example, to return from a `void` function:

```asm
; return;
ret %
```

- As a default value, for example to use default alignment in a stack allocation:

```asm
; int x[4];
var %%x 16 %
```



## Basic Blocks

A symbol is composed of basic blocks.

A basic block starts with a label declaration. This linker directive gives the basic block its name. All instructions until the next symbol or label are part of a basic block.

Each basic block ends in one of three control flow instructions:

- The [`ret` (return) instruction][ret] exits the function.
- The [`jmp` (jump) instruction][jmp] jumps to another basic block.
- The [`br` (branch) instruction][br] jumps to one of two basic blocks.

Control flow instructions can only appear at the end of a basic block. In other words, these instructions cannot be followed by another instruction; they can only be followed by a linker declaration or by the end of the file.

Basic blocks in a symbol therefore form a simple graph. Each block forwards control to zero, one or two other blocks, depending on the last instruction. Control flow only jumps from the end of a block to the start of another block; there is no control flow inside of blocks. This makes the code simple and easy to optimize.



### Stack Management

Onramp IR maintains the stack frame automatically. There are no `enter` or `leave` instructions, and instructions cannot access the `rfp` or `rsp` registers.

Storage space for variables is managed with the following instructions:

- [`var`][var] - Reserve stack space of a fixed size. (Used for most variables.)
- [`alloc`][alloc] - Allocate stack space dynamically. (Used for `alloca()` and VLAs.)
- [`free`][free] - Deallocate stack space dynamically. (Used when leaving the scope of a VLA.)

- `var <temp:ptr> <imm:size> <imm?:align>`: Statically allocates stack space of the given size, placing the address in the given temporary. The size must be a constant number. This reserves space in the function's stack frame, giving the variable a fixed address relative to the frame pointer. Each execution of a given `var` instruction places the same address in the output pointer.

- `alloc <temp:ptr> <mix:size>`: Dynamically allocates stack space of the given size, placing the address in the given temporary. This performs the same function as the `alloca()` extension in C. The stack pointer is adjusted by the given size and the address of the new storage is placed in the given temporary. Each execution of a an `alloca` instruction allocates additional storage and places a new address in the output pointer.

- `free <mix:size>`: Dynamically release the given size of stack space.

Storage space is reclaimed automatically when the function exits (either by returning or potentially by tail-call optimization.)



## Instruction Table

Arithmetic:

| Opcode | Arguments                    | Description                                                        |
|--------|------------------------------|--------------------------------------------------------------------|
| `add`  | `<t:dest> <m:src1> <m:src2>` | Adds src1 and src2, unsigned overflow                              |
| `sub`  | `<t:dest> <m:src1> <m:src2>` | Subtracts src2 from src1, unsigned underflow                       |
| `mul`  | `<t:dest> <m:src1> <m:src2>` | Multiplication                                                     |
| `divu` | `<t:dest> <m:src1> <m:src2>` | Divides src1 by src2 unsigned                                      |
| `divs` | `<t:dest> <m:src1> <m:src2>` | Divides src1 by src2 signed                                        |
| `modu` | `<t:dest> <m:src1> <m:src2>` | Modulus of src1 divided by src2 unsigned                           |
| `mods` | `<t:dest> <m:src1> <m:src2>` | Modulus of src1 divided by src2 signed                             |
| `zero` | `<t:dest>`                   | Sets the temporary to zero                                         |
| `inc`  | `<t:temp>`                   | Increments the temporary, unsigned overflow                        |
| `dec`  | `<t:temp>`                   | Decrements the temporary, unsigned underflow                       |
| `sxs`  | `<t:dest> <m:src>`           | Sign-extends a short value (copies bit 15 to upper 16 bits)        |
| `sxb`  | `<t:dest> <m:src>`           | Sign-extends a byte value (copies bit 7 to upper 24 bits)          |
| `trs`  | `<t:dest> <m:src>`           | Truncates the value to a short (zeroes upper 16 bits)              |
| `trb`  | `<t:dest> <m:src>`           | Truncates the value to a byte (zeroes upper 24 bits)               |

Logic:

| Opcode | Arguments                    | Description                                          |
|--------|------------------------------|------------------------------------------------------|
| `and`  | `<t:dest> <m:src1> <m:src2>` | Bitwise and                                          |
| `or`   | `<t:dest> <m:src1> <m:src2>` | Bitwise or                                           |
| `xor`  | `<t:dest> <m:src1> <m:src2>` | Bitwise xor                                          |
| `not`  | `<t:dest> <m:src>`           | Bitwise not (inverts all bits)                       |
| `shl`  | `<t:dest> <m:src1> <m:src2>` | Bitwise shift left (low to high)                     |
| `shru` | `<t:dest> <m:src1> <m:src2>` | Bitwise logical shift right (unsigned, high to low)  |
| `shrs` | `<t:dest> <m:src1> <m:src2>` | Bitwise arithmetic shift right (signed, high to low) |
| `rol`  | `<t:dest> <m:src1> <m:src2>` | Bitwise rotate left (low to high)                    |
| `ror`  | `<t:dest> <m:src1> <m:src2>` | Bitwise rotate right (high to low)                   |
| `mov`  | `<t:dest> <m:src>`           | Copies src to dest                                   |
| `bool` | `<t:dest> <m:src>`           | Sets dest to 1 if src is non-zero, 0 otherwise       |
| `isz`  | `<t:dest> <m:src>`           | Sets dest to 0 if src is non-zero, 1 otherwise       |
| `ltu`  | `<t:dest> <m:src1> <m:src2>` | Sets dest to 1 if src1 is less than src2 unsigned, 0 otherwise  |
| `lts`  | `<t:dest> <m:src1> <m:src2>` | Sets dest to 1 if src1 is less than src2 signed, 0 otherwise    |

Memory Access:

| Opcode | Arguments            | Description                                                      |
|--------|----------------------|------------------------------------------------------------------|
| `ldw`  | `<t:dest> <t:addr>`  | Loads a 4-byte word from memory (aligned)                        |
| `lds`  | `<t:dest> <t:addr>`  | Loads a 2-byte short from memory (aligned), zeroes upper 16 bits |
| `ldb`  | `<t:dest> <t:addr>`  | Loads a byte from memory, zeroes upper 24 bits                   |
| `stw`  | `<m:value> <t:addr>` | Stores a 4-byte word in memory (aligned)                         |
| `sts`  | `<m:value> <t:addr>` | Stores a 2-byte short in memory (aligned), ignores upper 16 bits |
| `stb`  | `<m:value> <t:addr>` | Stores a byte in memory, ignores upper 24 bits                   |

Stack allocation:

| Opcode     | Arguments                      | Description                                                       |
|------------|--------------------------------|-------------------------------------------------------------------|
| `var`      | `<t:temp> <i:size> <i?:align>` | Reserves stack space for a local variable                         |
| `alloc`    | `<t:temp> <m:size>`            | Dynamically allocates stack space                                 |
| `free`     | `<m:size>`                     | Frees dynamically allocated stack space                           |

Misc:

| Opcode     | Arguments                               | Description                                                       |
|------------|-----------------------------------------|-------------------------------------------------------------------|
| `call`     | `<t?:dest> <s/t:func> [<m:arg>...] end` | Call a function                                                   |
| `volatile` | `<t:temp>`                              | Forbid elision of memory access through temporary                 |

Control flow (end of block):

| Opcode  | Arguments                           | Description                                         |
|---------|-------------------------------------|-----------------------------------------------------|
| `ret`   | `<m?:value>`                        | Return from this function                           |
| `jmp`   | `<l:label>`                         | Jumps to the given label                            |
| `br`    | `<m:pred> <l:true> <l:false>`       | Branch to one of two labels based on the predicate  |



### Branch

```asm
br <mix:pred> <block:nonzero> <block:zero>
```

Jumps to basic block "nonzero" if the predicate is non-zero and "zero" otherwise. In other words, the first argument is the predicate, the second argument is the "true" branch and the third argument is the "false" branch.

The given destinations must use relative addressing (the `&` prefix) and therefore must be in the current symbol.

This is the only branch or conditional execution instruction in Onramp IR. Conditional jumps (e.g. jz, jnz) are not supported.



### Call

```asm
call <temp?:retval> <sym/temp:func> [<mix:arg>...] [varargs <mix:arg>...] end
```

Calls the given function.

The first argument is a temporary in which to store the return value, or `%` to ignore it.

The second argument is the function to call: either a function name prefixed with `^` (i.e. an absolute 32-bit invocation) for a direct call, or a temporary containing the function's absolute address for an indirect call (i.e. a function pointer.)

The remaining arguments are the arguments to pass to the function. An optional temporary pointing to a variadic argument list is preceded by the keyword `varargs`. The instruction is terminated by `end`.

The symbol to call can be an absolute label (e.g. `^strcmp`) or a temporary containing an absolute function address (to call a function pointer for example.)

Following the function name is an arbitrary-length sequence of mix-type arguments. The `end` keyword ends the argument list.

Example:

```asm
; result = strcmp(left, right);
call %result ^strcmp %left %right end
```

For a variadic call:

```asm
; printf(format, a, b, c);
call % ^printf %format varargs %a %b %c end
```

The order and meaning of arguments matches the standard Onramp calling convention. For example, structs and doubles are not passed directly; instead a pointer to their storage is passed. Similarly, if a function returns a struct or double by value, the caller must pass as the first argument a temporary containing the address of storage for the return value.

For example, to compute `sqrt(2)`:

```asm
var %%arg 8 %
call % ^__int_to_double %%arg 2 end
var %%result 8 %
call % ^sqrt %%result %arg %
```

example sequence of function calls:

```c
void* ptr = malloc(size);
use_pointer(ptr);
free(ptr);
```

The above C code is equivalent to the following IR:

```asm
call %ptr malloc %size end
var %%ptr 4 %
stw %ptr %%ptr
call % use_pointer %ptr end
call % free %ptr end
```

If a `call` is followed directly by a `ret` with the returned temporary, the code generator may attempt a tail call optimization. This is not guaranteed; for example it can fail if the function needs to pass arguments on the stack.



### `ret`

```asm
ret <mix?:value>
```

Returns the given value, exiting the current function.

If the value is the sentinel `%`, the return value is indeterminate. (`ret %` is emitted by the compiler to return from functions returning `void`.)



### `jmp`

```asm
jmp <block>
```

Jumps to the named basic block. The argument must be a label with relative addressing (i.e. `&`.)



### `mov`

```asm
mov <temp:dest> <mix:arg>
```

Note that there are no `imw` or `ims` instructions. `mov` can be used to load an arbitrary 32-bit number into a temporary.



### `ldw`

```asm
ldw <temp:dest> <temp:addr>
```

Note that `ldw` in IR takes only a single address argument. It does not take a base and offset as it does in assembly.



## Stack Allocation

### `var` - Get the Address of Reserved Stack Space

The `var` instruction defines a variable in the current stack frame and places the address of it in the given temporary.

```asm
var <temp:dest> <imm:size> <imm?:align>
```

The occurrence of the instruction reserves space in the stack frame at code generation time. If the instruction is run multiple times (in a loop for example), the same address is placed in the temporary each time.

This is used for virtually all declared variables, other l-values and many temporaries generated by the C compiler. The optimizer will try to eliminate as many of these as possible.

If the alignment is the sentinel `%`, a default alignment is used. (The default alignment on Onramp is 1 for sizes of 1 or 3 bytes, 2 for a size of 2 bytes, and 4 otherwise.)

For example, to define a `short` stack variable:

```asm
var %%x 2 %
```

To define a 16-byte word-aligned struct:

```asm
var %%s 16 %
```

To define an array of 8 shorts:

```asm
var %%a 16 2
```






<!--
Markdown link references follow.
-->

[var]: #var-get-the-address-of-reserved-stack-space
[alloc]: #alloc
[free]: #free
[jmp]: #jmp

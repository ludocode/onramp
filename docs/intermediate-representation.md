# Onramp Intermediate Representation

Onramp Intermediate Representation (IR) is a language used for code generation in the Onramp compiler. A `.oir` file contains Onramp IR.

The [final stage compiler (cci/2)](../core/cci/2-full/) consumes preprocessed C source code and produces Onramp IR. The [final stage code generator (cg/1)](../core/cci/1-full/) consumes Onramp IR and produces [Onramp Assembly](assembly.md).

(Note that earlier stages of the compiler and code generator do not use this IR. Instead they use a subset of Onramp Assembly as intermediate language.)

The primary task of the final stage code generator is to perform register allocation and stack manipulation. A secondary task is optimization. The intermediate representation is designed to facilitate these tasks.



## Overview / Comparison to Onramp Assembly

Onramp IR is very similar to Onramp Assembly, both in syntax and semantics. It is an untyped low-level assembly-like language. The main differences are:

- Instead of a finite set of registers, Onramp IR has unlimited temporaries. Temporaries are prefixed with `%`. Register allocation is performed by the code generator.

- Labels begin basic blocks. Control flow instructions can only occur at the end of basic blocks, and all basic blocks end in a control flow instruction.

- Stack manipulation is performed by the code generator. There are no `push`/`pop` or `enter`/`leave` instructions, no access to the `rsp` or `rfp` registers, etc. Instead `param`, `var` and `alloc` instructions reserve space in the stack frame.

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
| `x = &y`               | `mov %x %Y` \*        |
| `x = *y`               | `ldw %x %Y`           |
| `*x = y`               | `stw %y %X` \*        |
|------------------------|-----------------------|
| `goto L1`              | `jmp &L1`             |
|------------------------|-----------------------|
| `if x < y goto L1`     | `lts %_T1 %x %y`      |
| `goto L2`              | `br %_T1 &L1 &L2`     |
|------------------------|-----------------------|

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

Local variables in C are placed on the stack. They are declared by `var` instructions in the preamble of a function (before the first block label.)

The compiler tries to use the names of variables for the temporaries that contain their values. (This is not always possible due to shadowing, among other things.) By convention, it uses an additional `%` to indicate a pointer to the storage of a variable.

For example:

```c
void func(void) {
    int x = 5 + 3;
}
```

The above would be compiled to the following:

```asm
=func
  var %x 4 4
:_L1
  add %1 5 3
  stw %1 %x
```

In the above code:

- A variable is allocated on the stack and `%x` is a pointer to it.
- The computation is performed using generated temporary `%1`.
- The result `%1` is stored in the variable `%x`.

As seen above, the compiler generates numbered temporaries for anonymous intermediate r-values.



## Arguments

Function parameters are declared by `param` instructions in the preamble of a function (before the first block label.)

Parameters are given as *variables*: storage for them is allocated as though by `var` instructions and the parameter value is stored at that location. (This is done automatically for all parameters.)

Therefore, named parameters are actually *pointers* to the arguments given to the function.

For example, in a function like this:

```c
int add(int x, int y) {
    return x + y;
}
```

The compiler will emit code like this:

```asm
=add
  param %x
  param %y
:_L1
  ldw %1 %x
  ldw %2 %x
  add %3 %1 %2
  ret %3
```

In the above, `%x` is a pointer to the first parameter and `%y` is a pointer to the second. These are used by the compiler like ordinary variables: whenever the compiler changes the value of `x`, it will emit a store to `%x`, and whenever it uses the value of `x`, it will first emit a load of `%x`.

If the address of `x` is taken in the C code, `&x` is simply the value `%x`.

Parameters can be ignored with the sentinel `%`. This is usually used for unnamed parameters.

In the standard Onramp call convention, the first four arguments are passed in registers and additional arguments are passed on the stack. The code generator will allocate variables for the first four arguments; if they are unused, they can be optimized away. The compiler will map any additional arguments to their position above the frame pointer.

Note that in the standard calling convention, non-primitive types and types larger than 32 bits are passed indirectly: the caller allocates storage and passes a pointer to it. Therefore, the parameter variable is a pointer to the pointer. For example:

```c
struct point {int x, y};
int taxicab_length(struct point p) {
    return p.x + p.y;
}
```

For the above, the compiler will emit something like this:

```asm
=taxicab_length
  param %p
:_L1
  ldw %1 %p      ; %1 = p, also &p.x
  ldw %2 %1      ; %2 = p.x
  add %3 %1 4    ; %3 = &p.y
  ldw %4 %3      ; %4 = p.y
  add %5 %2 %4   ; %5 = p.x + p.y
  ret %5
```



## Variadic Arguments

Variadic arguments are declared with a `varargs` instruction in the function preamble of variadic functions, and passed with the `varargs` keyword in the argument list for variadic function calls.

If a function takes variadic arguments, the `varargs` instruction takes a temporary that contains the address of the first variadic argument. (The instruction must appear after any `param` instructions.) By convention this variable is named `%_Vargs` by the compiler. Calls to such functions must insert the keyword `varargs` before any variadic arguments.

For example:

```c
int foo(int x, ...) {
    va_list args;
    va_start(args, x);
    // ...
}

int main(void) {
    foo(1, 2, 3);
}
```

The above could be compiled as:

```asm
=foo
  param %x
  varargs %_Vargs
  var %args 4 4
:_L1
  stw %_Vargs %args
  ; ...

=main
:_L2
  call ^foo 1 varargs 2 3 end
```

In the called function, `%_Vargs` is the address of the first variadic parameter. The function typically stores in a variable and increments it to extract the variadic parameters.



## Sentinel

The plain `%` sigil without an identifier (i.e. followed by whitespace) is used as a sentinel value. It has several purposes:

- To ignore the return value of a function. For example:

```asm
; puts(str);
call % ^puts %str end
```

- To ignore a function parameter. For example:

```asm
; void free_sized(void* ptr, size_t) {free(ptr);}
=free_sized
    %ptr %
:_Lstart
    call % ^free %ptr end
    ret %
```

- To represent no value. For example, to return from a `void` function:

```asm
; return;
ret %
```

- As a default value, for example to use default alignment in a variable declaration:

```asm
; int x[4];
var %x 16 %
```

- To pass an uninitialized value to a function. For example:

```asm
; Suppose foo() takes two arguments and ignores its first argument if its
; second is zero. In this case we can pass it a sentinel, which means the first
; argument will be uninitialized. There is no way to do this in standard C.
call %ret ^foo % 0 end
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

Arguments have the following types:

- `t`: A temporary
- `m`: A mix-type argument, either a temporary or an immediate 32-bit integer (no range limit)
- `i`: An immediate 32-bit integer
- `s`: An absolute linker invocation (i.e. `^` and a symbol name)
- `l`: A relative linker invocation (i.e. `&` and a jump target)

A `/` symbol means multiple types are allowed, and `?` means the argument may be a sentinel.

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

| Opcode | Arguments              | Description                                                      |
|--------|------------------------|------------------------------------------------------------------|
| `sym`  | `<t:dest> <a:sym>`     | Gets the address of a symbol                                     |
| `ldw`  | `<t:dest> <t/s:addr>`  | Loads a 4-byte word from memory (aligned)                        |
| `lds`  | `<t:dest> <t/s:addr>`  | Loads a 2-byte short from memory (aligned), zeroes upper 16 bits |
| `ldb`  | `<t:dest> <t/s:addr>`  | Loads a byte from memory, zeroes upper 24 bits                   |
| `stw`  | `<m:value> <t/s:addr>` | Stores a 4-byte word in memory (aligned)                         |
| `sts`  | `<m:value> <t/s:addr>` | Stores a 2-byte short in memory (aligned), ignores upper 16 bits |
| `stb`  | `<m:value> <t/s:addr>` | Stores a byte in memory, ignores upper 24 bits                   |

Stack allocation:

| Opcode     | Arguments                      | Description                                                       |
|------------|--------------------------------|-------------------------------------------------------------------|
| `var`      | `<t:temp> <i:size> <i?:align>` | Reserves stack space for a local variable                         |
| `alloc`    | `<t:temp> <m:size>`            | Dynamically allocates stack space                                 |
| `free`     | `<m:size>`                     | Frees dynamically allocated stack space                           |

Misc:

| Opcode     | Arguments                               | Description                                                       |
|------------|-----------------------------------------|-------------------------------------------------------------------|
| `call`     | `<t?:dest> <t/s:func> [<m:arg>...] end` | Call a function                                                   |
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
var %arg 8 %
call % ^__int_to_double %arg 2 end
var %result 8 %
call % ^sqrt %result %arg %
```

example sequence of function calls:

```c
void* ptr = malloc(size);
use_pointer(ptr);
free(ptr);
```

The above C code is equivalent to the following IR:

```asm
ldw %1 %size
call %2 malloc %1 end
var %ptr 4 %
stw %2 %ptr
ldw %3 %ptr
call % use_pointer %3 end
ldw %4 %ptr
call % free %4 end
```

If a `call` is followed directly by a `ret` (with the returned temporary or none), the code generator may attempt a tail call optimization. This is not guaranteed; for example it can fail if the function needs to pass arguments on the stack.



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

Note that `ldw` in IR takes only a single address argument. It does not take a base and offset as it does in assembly. The argument may be an absolute symbol invocation, in which case the word is loaded from the symbol (`rpp` is added to it automatically.)



## Stack Allocation

### `var` - Create a variable

The `var` instruction defines a variable in the current stack frame and places the address of it in the given temporary.

```asm
var <temp:dest> <imm:size> <imm?:align>
```

The instruction must appear in the function preamble before the first block label.

This is used for virtually all declared variables, other l-values and many temporaries generated by the C compiler. The optimizer will try to eliminate as many of these as possible.

If the alignment is the sentinel `%`, a default alignment is used. (The default alignment on Onramp is 1 for sizes of 1 or 3 bytes, 2 for a size of 2 bytes, and 4 otherwise.)

For example, to define a `short` stack variable:

```asm
var %x 2 %
```

To define a 16-byte word-aligned struct:

```asm
var %s 16 %
```

To define an array of 8 shorts:

```asm
var %a 16 2
```





<!--
Markdown link references follow.
-->

[var]: #var-get-the-address-of-reserved-stack-space
[alloc]: #alloc
[free]: #free
[jmp]: #jmp

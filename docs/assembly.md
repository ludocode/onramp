# Onramp Assembly

Onramp Assembly is an assembly language used to produce object code for the Onramp virtual machine.

This document provides the specification of the assembly language. [A description of the bootstrapping stages and their implementation is here.](../core/as/README.md)



### Instruction Types

Onramp assembly has two different kinds of instructions: primitive instructions and compound instructions.

Primitive instructions are the instructions supported by the Onramp virtual machine. They are translated directly to their corresponding Onramp bytecode.

Compound instructions are converted by the assembler into one or more primitive instructions during assembly. For example, the compound instruction `ret` is converted to the primitive instruction `ldw rip 0 rsp` which is output as the bytecode `78 8F 00 8C`.

You can think of compound instructions as macros, except they are not defined in the assembly source; they are defined in the assembler program itself. (Unlike modern assemblers, there is no way to define macros in Onramp assembly.)



### Low-level Assembler

The first stage Onramp assembler recognizes only the following primitive instructions:

- Arithmetic: `add`, `sub`, `mul`, `div`
- Logic: `and`, `or`, `xor`, `ror`
- Memory: `ldw`, `stw`, `ldb`, `stb`
- Control: `ims`, `cmpu`, `jz`, `sys`

Each of these instructions takes three bytes of arguments and maps directly to an instruction in Onramp bytecode. The behaviour and required arguments of each is documented below.

In the first stage assembler, arguments can be passed as register or syscall names, as characters (provided they are in range), or as raw bytes.

In the full assembler, arguments can additionally be passed as numbers (decimal or hexadecimal) and a number argument may represent multiple bytes (for example as the 16-bit argument to `ims` or `jz`.)



### Registers

The Onramp assembler recognizes the following register names:

- `r0`, `r1`, `r2`, `r3`
- `r4`, `r5`, `r6`, `r7`
- `r8`, `r9`, `ra`, `rb`
- `rfp`, `rsp`, `rpp`, `rip`

Register names are replaced by their literal bytes (`80` through `8F`.) These must only be used as the argument to an instruction where a register is accepted. The early stage assemblers may not verify that a register name makes sense in the context in which it is used.



### Raw Hex Bytes

Raw hex bytes can be output by prefixing each one with `'`.

- `'<hex>`: A raw hex byte.

The `'` must be followed by two hexadecimal characters with no whitespace in between. The hexadecimal byte is output as-is (in plain text) without the leading `'`.

When outputting multiple bytes, each one must be prefixed with `'`.



### Comments

`;` starts a comment. The rest of the line is ignored.



### Debug Info

`#` starts a debug info annotation that runs until the end of the line. The assembler may pass the annotation verbatim to the output, or it may ignore it (treating it as a comment.)

Valid debug info consists of `#line` and `#pragma` directives. See the [Debug Info](debug-info.md) document for a description of the debug info format.



### Labels and Symbols

The Onramp assembly language uses the same syntax for labels and symbols as Onramp object code. Unlike most assemblers, the Onramp assembler does not actually resolve labels. They are passed directly through to the linker.

Here's a reference table:

| Syntax | Meaning                                     |
|--------|---------------------------------------------|
| `^`    | invocation: absolute, 32 bits               |
| `<`    | invocation: absolute, 16 high bits          |
| `>`    | invocation: absolute, 16 low bits           |
| `&`    | invocation: relative, 16-bit signed words   |
| `:`    | definition: label                           |
| `=`    | definition: global symbol                   |
| `@`    | definition: static symbol                   |
| `+`    | flag: zero symbol definition                |
| `?`    | flag: weak definition or invocation         |
| `{`    | flag: constructor                           |
| `}`    | flag: destructor                            |

For full usage details, see the [Onramp object code spec](object-code.md).



### String Literals

String literals can be provided by enclosing text in double quotes (`"`).

```
"Hello world!"
```

The bytes of the string are output directly by the assembler.

You can concatenate strings and raw bytes however you like. The above is equivalent to:

```
"Hello" '20 "world" '21
```

A string can contain any printable ASCII characters except for the double-quote (`"`) and the backslash (`\`). There are no escape sequences. To add other characters, concatenate the string with the raw bytes as above.

No null-terminator is appended. If you want one you have to append it (`'00`) yourself.

Typically you would place the string in its own symbol so it can be referenced by the rest of the code. For example, here's a typical C string with a trailing line feed and null-terminator:

```asm
=hello_world
    "Hello world!" '0A '00
```

Single-character strings can be used as instruction arguments. For example, to place the character 'a' in register r0:

```asm
mov r0 "a"
```



### Instructions

Instructions consist of an opcode and a number of arguments. Arguments can be integers, registers, quoted bytes, or single-character strings.

There are no commas separating arguments and whitespace is not significant (except where required to separate arguments.)

For most instructions (but not all), the destination register comes first, followed by the source values. For a few instructions (the stores and jumps), the source value comes first. This is done for simplicity and for symmetry with loads.

Compound instructions can assemble to different numbers of primitive instructions depending on the arguments and on the stage of the assembler. You should not assume the number of primitive instructions into which a compound instruction will expand.



#### Instruction Reference

Arguments have the following types:

- `r`: A register
- `m`: A mix-type byte, either a register or an immediate value in the range [-112,127]
- `b`: An immediate 8-bit byte
- `i`: An immediate 32-bit integer or equivalent (e.g. an absolute linker invocation, a number, four literal bytes)
- `c`: An absolute linker invocation (i.e. `^` and a symbol name)
- `j`: A relative linker invocation (i.e. `&` and a jump target)

The "Pr." column indicates which instructions are primitive. Only primitive instructions can use the scratch registers (`ra` and `rb`) as they are clobbered by most compound instructions.

Arithmetic:

| Pr. | Opcode | Arguments                    | Description                                                        |
|-----|--------|------------------------------|--------------------------------------------------------------------|
|  x  |`add`   | `<r:dest> <m:src1> <m:src2>` | Adds src1 and src2, unsigned overflow                              |
|  x  |`sub`   | `<r:dest> <m:src1> <m:src2>` | Subtracts src2 from src1, unsigned underflow                       |
|  x  |`mul`   | `<r:dest> <m:src1> <m:src2>` | Multiplication                                                     |
|  x  |`divu`  | `<r:dest> <m:src1> <m:src2>` | Divides src1 by src2 unsigned                                      |
|     |`divs`  | `<r:dest> <m:src1> <m:src2>` | Divides src1 by src2 signed                                        |
|     |`modu`  | `<r:dest> <m:src1> <m:src2>` | Modulus of src1 divided by src2 unsigned                           |
|     |`mods`  | `<r:dest> <m:src1> <m:src2>` | Modulus of src1 divided by src2 signed                             |
|     |`zero`  | `<r:dest>`                   | Sets the register to zero                                          |
|     |`inc`   | `<r:reg>`                    | Increments the register, unsigned overflow                         |
|     |`dec`   | `<r:reg>`                    | Decrements the register, unsigned underflow                        |
|     |`sxs`   | `<r:dest>` `<m:src>`         | Sign-extends the value to a short (copies bit 15 to upper 16 bits) |
|     |`sxb`   | `<r:dest>` `<m:src>`         | Sign-extends the value to a byte (copies bit 7 to upper 24 bits)   |
|     |`trs`   | `<r:dest>` `<m:src>`         | Truncates the value to a short (zeroes upper 16 bits)              |
|     |`trb`   | `<r:dest>` `<m:src>`         | Truncates the value to a byte (zeroes upper 24 bits)               |

Logic:

| Pr. | Opcode | Arguments                    | Description                                          |
|-----|--------|------------------------------|------------------------------------------------------|
|  x  |`and`   | `<r:dest> <m:src1> <m:src2>` | Bitwise and                                          |
|  x  |`or`    | `<r:dest> <m:src1> <m:src2>` | Bitwise or                                           |
|x \* |`xor`   | `<r:dest> <m:src1> <m:src2>` | Bitwise xor                                          |
|     |`not`   | `<r:dest> <m:src>`           | Bitwise not (inverts all bits)                       |
| \*  |`shl`   | `<r:dest> <m:src1> <m:src2>` | Bitwise shift left (low to high)                     |
| \*  |`shru`  | `<r:dest> <m:src1> <m:src2>` | Bitwise logical shift right unsigned (high to low)   |
|     |`shrs`  | `<r:dest> <m:src1> <m:src2>` | Bitwise arithmetic shift right signed (high to low)  |
|     |`rol`   | `<r:dest> <m:src1> <m:src2>` | Bitwise rotate left (low to high)                    |
|x \* |`ror`   | `<r:dest> <m:src1> <m:src2>` | Bitwise rotate right (high to low)                   |
|     |`mov`   | `<r:dest> <m:src>`           | Copies src to dest                                   |
|     |`bool`  | `<r:dest> <m:src>`           | Sets dest to 1 if src is non-zero, 0 otherwise       |
|     |`isz`   | `<r:dest> <m:src>`           | Sets dest to 0 if src is non-zero, 1 otherwise       |

Memory:

| Pr. | Opcode | Arguments                       | Description                                                      |
|-----|--------|---------------------------------|------------------------------------------------------------------|
|  x  |`ldw`   | `<r:dest> <m:base> <m:offset>`  | Loads a 4-byte word from memory (aligned)                        |
|     |`lds`   | `<r:dest> <m:base> <m:offset>`  | Loads a 2-byte short from memory (aligned), zeroes upper 16 bits |
|  x  |`ldb`   | `<r:dest> <m:base> <m:offset>`  | Loads a 1-byte byte from memory, zeroes upper 24 bits            |
|  x  |`stw`   | `<m:value> <m:base> <m:offset>` | Stores a 4-byte word in memory (aligned)                         |
|     |`sts`   | `<m:value> <m:base> <m:offset>` | Stores a 2-byte short in memory (aligned), ignores upper 16 bits |
|  x  |`stb`   | `<m:value> <m:base> <m:offset>` | Stores a 1-byte byte in memory, ignores upper 24 bits            |
|     |`push`  | `<m:value>`                     | Pushes a value to the stack                                      |
|     |`pop`   | `<r:reg>`                       | Pops the top stack value into the register                       |
|     |`popd`  | none                            | Pops the top stack value and discards it                         |

Control:

| Pr. | Opcode | Arguments                     | Description                                                  |
|-----|--------|-------------------------------|--------------------------------------------------------------|
|  x  |`ims`   | `<r:reg> <b:high> <b:low>`    | Shifts register up 16 bits, then loads a 16-bit immediate    |
|     |`imw`   | `<r:reg> <i:value>`           | Loads a 32-bit immediate                                     |
|x \* |`cmpu`  | `<r:dest> <m:src1> <m:src2>`  | Compares src1 to src2 unsigned, placing -1, 0 or 1 in dest   |
|     |`cmps`  | `<r:dest> <m:src1> <m:src2>`  | Compares src1 to src2 signed, placing -1, 0 or 1 in dest     |
|  x  |`jz`    | `<m:pred> <j:label>`          | Jumps if the predicate is zero                               |
|     |`jnz`   | `<m:pred> <j:label>`          | Jumps if the predicate is not zero                           |
|x \* |`je`    | \* `<m:pred> <j:label>`       | Jumps if the predicate is zero                               |
|     |`jne`   | \* `<m:pred> <j:label>`       | Jumps if the predicate is not zero                           |
|     |`jl`    | \* `<r:pred> <j:label>`       | Jumps if the predicate is -1                                 |
|     |`jg`    | \* `<r:pred> <j:label>`       | Jumps if the predicate is 1                                  |
|     |`jle`   | \* `<r:pred> <j:label>`       | Jumps if the predicate is not 1                              |
|     |`jge`   | \* `<r:pred> <j:label>`       | Jumps if the predicate is not -1                             |
|     |`jmp`   | `<j:label>` or `<c:function>` | Jumps unconditionally                                        |
|     |`call`  | `<c:function>`                | Calls a function (pushing the return address to the stack)   |
|     |`ret`   | none                          | Returns from a function call                                 |
|     |`enter` | none                          | Creates a stack frame                                        |
|     |`leave` | none                          | Destroys the current stack frame                             |
|  x  |`sys`   | `<b:number> '00 '00`          | Performs a system call                                       |

\* WARNING: The above tables will change significantly. `xor` and `ror` are currently primitive instructions but they will be replaced with `shl` and `shru` at some point. The comparison conditional jump instructions (`je`, `jne`, `jl`, `jg`, `jle`, `jge`) are currently designed to take the result of a comparison (`cmpu`, `cmps`) as predicate; they will be changed to take two source arguments and compare them internally. `sys`, `cmpu` and `cmps` may be removed entirely.

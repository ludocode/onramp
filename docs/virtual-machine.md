# Onramp Virtual Machine and Bytecode

The Onramp Virtual Machine is a simple virtual machine designed for portable bootstrapping.

This document specifies version 2 of the virtual machine and its bytecode. For a description of implementations, see [Onramp Virtual Machine Implementations](../platform/vm).

Here's an index of sections in this document:

- [Overview](#overview)
- [Registers](#registers)
- [Memory Layout](#memory-layout)
- [Process Info Table](#process-info-table)
- [Position-Independence](#position-independence)
- [Instructions](#instructions)
- [Negative Values](#negative-values)
- [Opcode Table](#opcode-table)
- [Opcode Specifications](#opcode-specifications)
    - [Add](#add)
    - [Subtract](#subtract)
    - [Multiply](#multiply)
    - [Divide Unsigned](#divide-unsigned)
    - [Bitwise And](#bitwise-and)
    - [Bitwise Or](#bitwise-or)
    - [Shift Left](#shift-left)
    - [Shift Right Unsigned](#shift-right-unsigned)
    - [Load Word](#load-word)
    - [Store Word](#store-word)
    - [Load Byte](#load-byte)
    - [Store Byte](#store-byte)
    - [Immediate Short](#immediate-short)
    - [Less Than Unsigned](#less-than-unsigned)
    - [Jump If Zero](#jump-if-zero)
- [Calling Convention](#calling-convention)
    - [External Function Calls](#external-function-calls)
- [System Calls](#system-calls)
    - [System Call Table](#system-call-table)
    - [System Call Quick Reference](#system-call-quick-reference)
    - [`halt`](#halt)
    - [`time`](#time)
    - [`panic`](#panic)
    - [`fopen`](#fopen)
    - [`fclose`](#fclose)
    - [`fread`](#fread)
    - [`fwrite`](#fwrite)
    - [`fseek`](#fseek)
    - [`ftell`](#ftell)
    - [`ftrunc`](#ftrunc)
    - [`dopen`](#dopen)
    - [`dclose`](#dclose)
    - [`dread`](#dread)
    - [`stat`](#stat)
    - [`rename`](#rename)
    - [`symlink`](#symlink)
    - [`unlink`](#unlink)
    - [`chmod`](#chmod)
    - [`mkdir`](#mkdir)
    - [`rmdir`](#rmdir)
    - [`spawn`](#spawn)
- [Filesystem](#filesystem)
    - [I/O Handles](#io-handles)
    - [Input/Output Streams](#inputoutput-streams)
        - [Input](#input)
        - [Output and Error](#output-and-error)
    - [Error Handling](#error-handing)
- [Debug Info](#debug-info)
- [File Format](#file-format)
- [Rationale](#rationale)



## Overview

The Onramp VM is a register-based reduced instruction set computer (RISC). It uses 32-bit words addressed over 8-bit memory.

A large contiguous region of memory is available to the program which stores both code and data. The memory region is initialized with the program code, along with a process info table (PIT) containing information about the environment: command-line arguments, environment variables and so on. Programs typically divide the remainder of memory into a heap and a stack.

Programs are stored as a series of 32-bit bytecode instructions in a binary file format. Each instruction is four bytes: the first byte is the opcode and the three remaining bytes are the arguments. Opcodes, register arguments, and most immediate values have distinct hexadecimal prefixes which makes it easy to read and write bytecode in hexadecimal.

There are 16 registers. Registers `r0` through `r9` are general purpose. Registers `ra` and `rb` are scratch registers reserved by the Onramp assembler. The last four registers are the stack pointer `rpp`, frame pointer `rfp`, program pointer `rpp` and instruction pointer `rip` respectively. There is no flags register. Carry and overflow must be detected manually, and conditional instructions use arbitrary registers as predicates.

There are 15 opcodes. All of them provide extremely basic functionality, such as adding two values into a register or storing a word at a memory address. High-level instructions such as pushing data onto the stack or calling a function do not have dedicated opcodes: they must be emulated with several low-level instructions. Emulation of high-level instructions is provided by the Onramp assembler using the scratch registers (`ra` and `rb`.)

There are (currently) no interrupts. The VM is designed for non-interactive computation. It does however optionally support input and time so it is possible to write interactive terminal applications.

All programs are position-independent. This makes it possible for programs to run other programs without the need for virtual memory. The program pointer (`rpp`) contains the base address of the currently running program.

The VM accesses the outside environment via system calls. These are provided by the VM to the program as a table of function pointers which the program calls using a special calling convention. System calls include input and output streams; file and directory access; time and more.

An Onramp VM can run hosted or freestanding. When hosted, the platform's filesystem is bridged into the virtual machine. This requires the VM to implement the file and directory system calls. When freestanding, an OS runs inside the Onramp VM. The contained OS receives most system calls and implements the filesystem.

When writing C programs and compiling them for Onramp, you do not need to worry about any of this. It is handled by the Onramp libc.



## Registers

There are sixteen registers numbered `0x80` to `0x8F`. All instructions can operate on all registers, but some registers have special behaviour (such as the instruction pointer and stack pointer) and others have strong conventions on their use (such as the frame pointer and program pointer.) These latter registers are given special names.

- Registers `r0`, `r1`, `r2` and `r3` are general-purpose caller-preserved registers that are used as function arguments and return values. See the Calling Convention section below.

- Registers `r4`, `r5`, `r6`, `r7`, `r8` and `r9` are general purpose caller-preserved registers. These are typically used for local variables in functions.

- Registers `ra` and `rb` are "scratch space" registers. They are clobbered not only by function calls but also by compound assembly instructions. They can be used for temporary space when writing bytecode by hand but they are best avoided when writing or emitting assembly.

- Register `rsp` is the stack pointer. It points to the last value pushed on the stack. The Onramp VM stack grows down. The stack pointer must always be aligned to a 4-byte boundary and must always have 128 bytes free under it for interrupts and syscalls. There is no [red zone](https://en.wikipedia.org/wiki/Red_zone_(computing)); it is an error to read or write to the stack area under the stack pointer.

- Register `rfp` is the frame pointer. It points to the start of the current function's stack frame. This is the location where the previous frame pointer was pushed, forming a linked list of stack frames.

- Register `rpp` is the program pointer. It points to the base address where the program has been loaded into memory. Onramp VM programs are position-independent and must use `rpp` to calculate the addresses of program code and data.

- Register `rip` is the instruction pointer. It points to the next instruction to be executed. Upon reading an instruction, the VM increments the instruction pointer past that instruction before executing it.

Here it is in table form:

| Name      | Hex       | Description                                                 | Preserved by |
|-----------|-----------|-------------------------------------------------------------|--------------|
| `r0`-`r3` | `80`-`83` | Function call arguments and return values                   | Caller       |
| `r4`-`r9` | `84`-`89` | Local variables                                             | Caller       |
| `ra`-`rb` | `8A`-`8B` | Scratch or compound assembly                                | Neither      |
| `rsp`     | `8C`      | Stack pointer (last data pushed onto stack)                 | Callee       |
| `rfp`     | `8D`      | Frame pointer (start of stack frame)                        | Callee       |
| `rpp`     | `8E`      | Program pointer (start of program)                          | Callee\*     |
| `rip`     | `8F`      | Instruction pointer (next instruction to be executed)       | Caller       |

\* The program pointer is callee-preserved, except on an external function call where it is altered by the caller and preserved by both. See the Calling Convention section below.

There is no status or flags register. Detecting overflow, underflow and other such conditions must be done manually. Instructions such as `ltu` (less than unsigned) and `jz` (jump if zero) can read or write the predicate in any register.



## Memory Layout

A large contiguous block of memory is provided for the program at an arbitrary 32-bit address range. This area is bounded by the initial values of `rpp` (the program pointer) and `rsp` (the stack pointer.) The program code is loaded into the start of this region, and the end of the program code is called the program break. Programs typically divide the remaining memory into a heap (which grows up) and a stack (which grows down.)

Information about the process and VM is also made available to the program. This includes the command-line arguments and environment variables of the process, as well as the VM's capabilities and input/output ports. This information is accessible to the program but sits outside its dedicated memory region.

Here's a diagram showing the regions of memory, the initial values of the registers, and some of the addresses in the information table:

```
       read only             read/write                            read/write/execute
  ~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  |  process info   |    |  command-line   |    |  program code     :                       :         |
  |    table,       |    |    args,        |    |      and          :         heap          :  stack  |
  |  capabilities   |    |  environ vars   |    |  static storage   :                       :         |
  ~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ^-- r0                        ^-- argv      ^-- rpp             ^-- break                         ^-- rsp
            ^-- exit       ^-- environ          ^-- rip
```

The entire memory region from `rpp` to the initial `rsp` is writable and executable. The process information table sits outside of this region and must not be written to.

There is no memory protection for the running program. If the program accesses memory outside of these ranges, or writes to a read-only memory region, the behaviour is undefined. (The VM may crash, the parent process may be corrupted, etc.)

The initial values of registers at program start are given in the following table. All registers not listed may have any value.

| Name     | Initial Value                                    |
|----------|--------------------------------------------------|
| r0       | Process Information Table                        |
| rsp      | End of available memory                          |
| rpp      | Start of program                                 |
| rip      | Start of program                                 |

Note that although `rip` is initially set to the start of the program, it will move past the first instruction before that instruction actually executes. `rpp` should be used as the start of the program.



## Process Info Table

The process information table is an array of 32-bit words. Here's a quick reference table of its contents:

| Index | Value                  | type   | Description
|-------|------------------------|--------|------------------------------------------------------------------------|
| 0     | Version                | int    | Always 2 for this version.                                             |
| 1     | Program Break          | void*  | Address of one past the last byte in the program.                      |
| 2     | System Call Table      | int    | Table of system calls.                                                 |
| 3     | Input stream handle    | int    | File handle of input stream, or -1 if input is not supported           |
| 4     | Output stream handle   | int    | File handle of output stream, can match input                          |
| 5     | Error stream handle    | int    | File handle of error stream, can match output and input                |
| 6     | Command-Line Arguments | char** | Null-terminated array of null-terminated strings.                      |
| 7     | Environment Variables  | char** | Null-terminated array of null-terminated strings of form "key=value".  |
| 8     | Working directory      | char*  | Directory in which the program is being run.                           |
| 9     | Capabilities           | int    | Flags indicating the capabilities and environment of the VM.           |



### Capabilities

The capabilities entry is an int containing a set of 1-bit flags. They are numbered from least to most significant bit:

| Bit Position | Value               |                                                            |
|--------------|---------------------|------------------------------------------------------------|
| 0            | Input Echo          | 0 if Onramp should echo output to the input.               |
| 1            | Input Blocks        | 1 if fread(input) blocks until input is available.         |
| 2            | Input Line-Oriented | 1 if VM buffers input in lines (i.e. POSIX canonical)      |

The parent process of a program (the VM or otherwise) must assemble this table somewhere in memory accessible to the program and pass a pointer to it in `r0`.

The version field contains the version of the Onramp VM. The information table is intended to be forward-compatible, so to perform a version check, ensure the version is at least as large as the version you need.

The program break is the address of one past the last byte of the program bytecode. In other words it's the start of the heap, which programs typically use for `malloc()`.

The syscall table is described in the section [System Call Table](#system-call-table).

Handles for the input, output and error streams may have any value (and may even all have the same value.) The program must use these handles to communicate with the outside world. If a handle has value 0xFFFFFFFF, this indicates that the stream does not exist, and the program should avoid using it.

Command-line arguments and environment variables are stored in null-terminated arrays of null-terminated strings. In a hosted environment, the command-line always has at least one string, which is the path to the program being run. Environment variables are typically key-value pairs delimited by `=`. The substring to the left of the first `=` is the key, while the substring to the right is the value.

The working directory is the base directory that the program should use for relative paths. Note that VM syscalls have no concept of a working directory: all paths must be absolute. It is up to the program to track its own working directory and append relative paths to it before calling VM syscalls. (This is handled by the Onramp libc.) (TODO this is not done yet, currently the VMs all support paths relative to the initial working directory.)

In a freestanding environment, the command-line, environment variables and working directory may all be null.

The capabilities field contains a set of flags describing what features are supported by the VM. The following flags exist, with bits numbered from low to high:

- bit 0: input echo. 1 if the input stream is echoed to the output; 0 otherwise. If possible the VM should not echo input.
- bit 1: input blocks. 1 if the read syscall blocks until a byte is available; 0 if it doesn't, instead reading zero bytes successfully when no input data exists. If possible the VM should not block on input. If you are unsure whether the input blocks, set this to 1 to prevent programs from setting the input to non-blocking.
- bit 2: input line-oriented (i.e. [POSIX canonical](https://en.wikipedia.org/wiki/POSIX_terminal_interface#Canonical_mode_processing)). 1 if input is only available once a full line has been processed; 0 if input is available immediately on each keystroke. If possible the VM should not line-buffer input. If you are unsure whether the input is line-oriented, set this to 1 to prevent programs from turning off canonical mode.

Note that the process info table and its associated information must not be written to except that command-line arguments and environment variables may be modified (for example with `strtok()`.) Any other changes are undefined behaviour, and may crash the VM or corrupt the parent process.




## Position-Independence

All Onramp VM bytecode programs are position-independent. Programs can be loaded at any address in memory.

A special register, called the *program pointer* (`rpp`), contains the base address of the program in memory. References to symbols in the program (such as functions and global variables) are relative to the start of the program. Their absolute address can be determined by adding `rpp`.

Several instructions take two parameters that are added together, such as the add instruction and the load and store instructions. These can be interpreted as a base and an offset. When accessing program data, typically `rpp` is passed as the base, and the address of the data within the program (i.e. the value of a label) is passed as the offset.

For example, to load a 32-bit global variable into register `r0`, you would load the address of the label into a temporary register (e.g. `ra`) and then load this address relative to `rpp`. In primitive assembly, this is:

```
ims ra <some_variable   ; load the program-relative address of some_variable into ra
ims ra >some_variable   ; ...
ldw r0 rpp ra           ; load [rpp + ra] into r0
```

The compound assembler provides an `imw `instruction to load a label in one step:

```
ims r9 ^some_variable
ldw r0 rpp r9
```

Similarly, to call a function, you must load its label into a temporary register and then add it to `rpp` to get the absolute address. The jump is performed by storing the result directly in the instruction pointer. Typically you would also push the return address in between:

```
imm ra <some_function    ; load the program-relative address of some_function into ra
imm ra >some_function    ; ...
sub rsp rsp 4            ; make space on the stack for the return address
add rb rip 8             ; calculate the return address into rb     -----.
stw rb rsp 0             ; place the return address on the stack         |
add rip rpp ra           ; jump to rpp + ra (by assigning to rip)        |
add rsp rsp 4            ; pop the return address from the stack    <----`
```

The compound assembler has a `call` instruction which expands to the above:

```
call ^some_function
```

(This would of course be done after preparing the arguments; see the Calling Convention section below.)

There is only one instruction that takes an address relative to the instruction pointer: the conditional jump-if-zero instruction (`jz`). It takes a signed 16-bit relative address, making it very useful for hand-writing loops in bytecode.



## Instructions

Onramp bytecode instructions are 32 bits in little-endian, or four bytes. All instructions must be aligned to a 32-bit boundary.

Each instruction opcode and each argument occupies one byte each, so each instruction has three arguments. Instructions are of the form:

```
[opcode] [argument-1] [argument-2] [argument-3]
```

For most instructions, argument 1 is an output register and arguments 2 and 3 are the inputs.

Opcodes, registers and single-byte immediate values have recognizable locations and hexadecimal prefixes:

- Opcodes start with `7`, and are always on a 32-bit boundary;
- Registers start with `8`, and are always *not* on a 32-bit boundary;
- Non-negative single-byte immediate values start with `0`-`7`;
- Negative single-byte immediate values start with `9`-`F`.

This makes it easy to read a hexdump of bytecode and to write it directly in [commented hexadecimal](hexadecimal.md).

Each instruction specifies the type of its arguments. Arguments can be one of several types:

- A "reg" (or `r`) argument is the name of a register. It must be a byte in the range `80`-`8F`.
- An "imm" (or `i`) argument is a literal byte. It can have any value.
- A "mix" (or `m`) argument is one byte that translates to a 32-bit value. How it is translated depends on its hexadecimal prefix:
    - If it is in the range `80`-`8F`, its value is the content of the named register.
    - If it is in the range `00`-`7F`, it is an immediate positive value with high bits set to zero.
    - If it is in the range `90`-`FF`, it is an immediate negative value; it is sign-extended, i.e. the high 24 bits are set.

Most instructions take mix-type arguments as input. This makes it easy to do math between registers and small immediate values without complicating the instruction set.

All instructions perform unsigned operations. (However, since any overflow is discarded, the result in most cases is the same as signed two's complement, so you can use signed two's complement operations if that's all you have. The exceptions have a `u` suffix to differentiate them from their signed `s` counterparts in compound assembly.)



## Negative Values

This specification does not prescribe any representation for signed numbers. There are no signed operations at the VM level, and a VM can be implemented using purely unsigned arithmetic with standard overflow behaviour. (A goal here is to maximize the kinds of alien architectures that can easily implement Onramp.)

The only thing the VM needs to be able to do in relation to negative numbers is sign extension. The VM must be able to copy the high bit of an 8-bit or 16-bit value to the upper bits of a 32-bit word. This must be done in two cases:

- When a mix-type byte is in the range `0x90`-`0xFF`, the upper bits must be set to 1 to extend it to 32 bits. (In order words, when a mix-type byte is not a register, the 8th bit must be copied to the upper 24 bits.)

- The 16th bit of the two-byte offset in a conditional jump instruction must be copied to the upper bits to extend it to a full 32-bit word. (It can then be added to the instruction pointer using an ordinary unsigned addition that wraps to 32 bits.)

Programs compiled by the Onramp compiler and assembler use two's complement to represent signed numbers. All signed operations are reduced to unsigned VM instructions by the assembler.



## Opcode Table

Opcodes are divided into four groups: arithmetic, logic, memory and control. Each group has four opcodes.

Arguments have the following types:

- `r`: A register
- `m`: A mix-type byte, either a register or an immediate value in the range [-112,127]
- `i`: An immediate byte (any value)

Here's a quick reference table for all supported instruction opcodes:

Arithmetic:

| Opcode        | Name              | Arguments                       | Operation                        |
|---------------|-------------------|---------------------------------|----------------------------------|
| `0x70` `add`  | [Add]             | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 + arg2`             |
| `0x71` `sub`  | [Subtract]        | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 - arg2`             |
| `0x72` `mul`  | [Multiply]        | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 * arg2`             |
| `0x73` `div`  | [Divide Unsigned] | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 / arg2` (unsigned)  |

Logic:

| Opcode         | Name                   | Arguments                       | Operation                                     |
|----------------|------------------------|---------------------------------|-----------------------------------------------|
| `0x74` `and`   | [Bitwise And]          | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 & arg2`                          |
| `0x75` `or`    | [Bitwise Or]           | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 \| arg2`                         |
| `0x76` `shl`   | [Shift Left]           | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 << arg2`                         |
| `0x77` `shru`  | [Shift Right Unsigned] | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 >> arg2` (unsigned)              |

Memory:

| Opcode        | Name         | Arguments                       | Operation                                  |
|---------------|--------------|---------------------------------|--------------------------------------------|
| `0x78` `ldw`  | [Load Word]  | `<r:dest> <m:base> <m:offset>`  | `dest = *(int*)(base + offset)`            |
| `0x79` `stw`  | [Store Word] | `<m:src> <m:base> <m:offset>`   | `*(int*)(base + offset) = src`             |
| `0x7A` `ldb`  | [Load Byte]  | `<r:dest> <m:base> <m:offset>`  | `dest = *(char*)(base + offset)`           |
| `0x7B` `stb`  | [Store Byte] | `<m:src> <m:base> <m:offset>`   | `*(char*)(base + offset) = src & 0xFF`     |

Control:

| Opcode       | Name                 | Arguments                       | Operation                                                |
|--------------|----------------------|---------------------------------|----------------------------------------------------------|
| `0x7C` `ims` | [Immediate Short]    | `<r:dest> <i:low> <i:high>`     | `dest = (dest << 16) \| (high << 8) \| low`              |
| `0x7D` `ltu` | [Less Than Unsigned] | `<r:dest> <m:arg1> <m:arg2>`    | `dest = (arg1 < arg2) ? 1 : 0` (unsigned)                |
| `0x7E` `jz`  | [Jump If Zero]       | `<m:pred> <i:low> <i:high>`     | `if !pred: rip += 4 * signext16((high << 8) \| low)`     |

All arithmetic and logic opcodes have the same format. They take a destination register and two mix-type arguments. They perform a mathematical operation on the arguments and place the result in the given register. All operations are unsigned.



## Opcode Specifications



### Add

- opcode: `0x70`
- assembly syntax: `add <r:dest> <m:arg1> <m:arg2>`
- behaviour: `dest = arg1 + arg2`

The `add` instruction adds two 32-bit values, placing the result in a register. The addition is performed with unsigned overflow and the carry is discarded.

One of the arguments is often the same as the destination in order to modify a register in-place. For example:

```asm
70 8C 8C 04    ; add rsp rsp 4
```

The above pops a word off the stack.

The add instruction is used for many things beyond addition. It is often used with one of the source arguments zero to copy a value from one register to another. For example, the compound assembly instruction `mov r0 r1` is assembled to the following:

```asm
70 80 81 00    ; add r0 r1 0    ; mov r0 r1
```

It is also used to initialize registers with small constant values in a single instruction. For example:

```asm
70 80 00 05    ; add r0 0 5          ; mov r0 5
70 81 7F 7F    ; add r1 127 127      ; mov r1 254
70 82 90 90    ; add r2 -112 -112    ; mov r2 -224
```

The add instruction is also used to perform some absolute jumps. For example:

```
ims ra <some_function
ims ra >some_function
add rip rpp ra
```

In the above, the program-relative address of the function is added to the program pointer. The result is the absolute address of the function in memory. The result is placed in the instruction pointer, thus jumping to it.



### Subtract

- opcode: `0x71`
- assembly syntax: `sub <r:dest> <m:arg1> <m:arg2>`
- behaviour: `dest = arg1 - arg2`

The `sub` instruction adds two 32-bit values, placing the result in a register. The subtraction is performed with unsigned overflow and any carry/borrow is discarded.

The `sub` instruction can also be used to initialize a few additional small constant values that are not possible with `add`. For example:

```asm
71 81 90 71    ; sub r1 -112 113     ; mov r1 -225
71 81 90 7F    ; sub r1 -112 128     ; mov r1 -239
```


### Multiply

- opcode: `0x72`
- assembly syntax: `mul <r:dest> <m:arg1> <m:arg2>`
- behaviour: `dest = arg1 * arg2`

The `mul` instruction multiplies two 32-bit values, placing the low 32 bits of the result in a register.

Programs compiled with Onramp try to use shifts in place of multiplications where possible. The shift instructions are assumed to be faster than multiplication.



### Divide Unsigned

- opcode: `0x73`
- assembly syntax: `divu <r:dest> <m:arg1> <m:arg2>`
- behaviour: `dest = arg1 / arg2` (unsigned)

The `divu` instruction divides the numerator `arg1` by the denominator `arg2`, placing the result in a register.

Note that a 32-bit two's complement signed division produces different results; you must be careful to perform unsigned division. Signed division is simulated by the `divs` instruction in compound assembly.

If arg2 is 0, the result is undefined; the VM may halt, corrupt memory, or produce incorrect results.

Programs compiled with Onramp try to use other instructions (shift, multiply) in place of divide where possible. The divide instruction is assumed to be the slowest opcode in an Onramp VM.



### Bitwise And

- opcode: `0x74`
- assembly syntax: `and <r:dest> <m:arg1> <m:arg2>`
- behaviour: `dest = arg1 & arg2`

For each bit in the result, if the corresponding bit at the same position in both arguments are set, the result bit is set.



### Bitwise Or

- opcode: `0x75`
- assembly syntax: `or <r:dest> <m:arg1> <m:arg2>`
- behaviour: `dest = arg1 | arg2`

For each bit in the result, if the corresponding bit at the same position in either argument is set, the result bit is set.



### Shift Left

- opcode: `0x76`
- assembly syntax: `shl <r:dest> <m:arg1> <m:arg2>`
- behaviour: `dest = arg1 << arg2`

Shifts bits in `arg1` by the number of positions given in `arg2` from least significant to most significant. The most significant bits shifted off the edge are discarded. The least significant bits shifted in are 0.

This is equivalent to multiplying `arg1` by two to the power of `arg2`.

The VM is allowed to assume that `arg2` is always in the range of 0 to 31 inclusive; for example, it may ignore all but the low five bits. (Debugging VMs halt the program and report an error if `arg2` is 32 or larger.)



### Shift Right Unsigned

- opcode: `0x77`
- assembly syntax: `shru <r:dest> <m:arg1> <m:arg2>`
- behaviour: `dest = arg1 >> arg2` (unsigned)

Shifts bits in arg1 by the number of positions given in `arg2` from most significant to least significant. The least significant bits shifted off the edge are discarded. The most significant bits shifted in are 0.

This is equivalent to dividing `arg1` by two to the power of `arg2`.

Note that a 32-bit two's complement arithmetic right shift produces different results; you must be careful to perform an unsigned (logical) shift. Signed right shift is simulated by the `shrs` instruction in compound assembly.

The VM is allowed to assume that `arg2` is always in the range of 0 to 31 inclusive; for example, it may ignore all but the low five bits. (Debugging VMs halt the program and report an error if `arg2` is 32 or larger.)



### Load Word

- opcode: `0x78`
- assembly syntax: `ldw <r:dest> <m:base> <m:offset>`
- behaviour: `dest = *(int*)(base + offset)`

Loads the word at the address given by the sum of `base` and `offset`, placing it in the destination register.

The VM is allowed to assume that the result of the address addition is always aligned to a 32-bit boundary; for example, it may ignore the low two bits. (Debugging VMs halt the program and report an error if the result is misaligned.) However, the `base` and `offset` do not themselves need to be aligned to a 32-bit boundary. The addition must be performed on all bits of `base` and `offset`.

There is no difference between the `base` and `offset` arguments. The addition is commutative so the two arguments are interchangeable. Their names indicate the convention by which they are typically used.



### Store Word

- opcode: `0x79`
- assembly syntax: `stw <m:src> <m:base> <m:offset>`
- behaviour: `*(int*)(base + offset) = src`

Stores the source word at the address given by the sum of `base` and `offset`.

The VM is allowed to assume that the result of the address addition is always aligned to a 32-bit boundary; for example, it may ignore the low two bits. (Debugging VMs halt the program and report an error if the result is misaligned.) However, the `base` and `offset` do not themselves need to be aligned to a 32-bit boundary. The addition must be performed on all bits of `base` and `offset`.

There is no difference between the `base` and `offset` arguments. The addition is commutative so the two arguments are interchangeable. Their names indicate the convention by which they are typically used.

Note that, unlike most instructions, the first argument is a source, not a destination. This was done to keep it symmetric with the load instructions. If the source is a register, it is unchanged by this instruction.

Since the source argument is mix-type, it is often used to store small constant values. For example, the program can directly store a zero to clear a word in memory.



### Load Byte

- opcode: `0x7A`
- assembly syntax: `ldb <r:dest> <m:base> <m:offset>`
- behaviour: `dest = *(char*)(base + offset)`

Loads the byte at the address given by the sum of `base` and `offset`, placing it in the low 8 bits of the destination register. The upper 24 bits of the destination register are cleared.

There is no difference between the `base` and `offset` arguments. The addition is commutative so the two arguments are interchangeable. Their names indicate the convention by which they are typically used.



### Store Byte

- opcode: `0x7B`
- assembly syntax: `stb <m:src> <m:base> <m:offset>`
- behaviour: `*(char*)(base + offset) = src & 0xFF`

Stores the low 8 bits of the source word at the address given by the sum of `base` and `offset`.

There is no difference between the `base` and `offset` arguments. The addition is commutative so the two arguments are interchangeable. Their names indicate the convention by which they are typically used.

Note that, unlike most instructions, the first argument is a source, not a destination. This was done to keep it symmetric with the load instructions. If the source is a register, it is unchanged by this instruction. (In particular, the upper bits that are discarded during the store operation are not modified in the source register.)

Since the source argument is mix-type, it is often used to store small constant values. For example, the program can directly store a zero to clear a word in memory.



### Immediate Short

- opcode: `0x7C`
- assembly syntax: `ims <r:dest> <i:low> <i:high>`
- behaviour: `dest = (dest << 16) \| (high << 8) \| low`

Shifts the contents of the destination register up (left) 16 bits, then loads the low sixteen bits with the given arguments. The `low` argument is placed in bits 0-7 and the `high` argument is placed in bits 8-15. The high 16 bits of that are shifted out of the destination register are discarded.

The immediate short instruction is almost always used in pairs to load all 32 bits of a register. For example:

```asm
7C 80 34 12
7C 80 78 56
```

The above places 0x12345678 in the r0 register. A VM may optimize for this case, detecting when two `ims` operations are used together and loading all 32 bits of the register at once. (In particular, the arguments of the first `ims` instruction are often zero.) However, there are still rare cases where this instruction is used alone. The VM must have a fallback that implements the instruction correctly.

The `ims` instruction is assumed to be fast due to this possible optimization. When compiling user code, if a register cannot be set to an immediate value in a single instruction, the Onramp toolchain prefers to output a pair of `ims` instructions.



### Less Than Unsigned

- opcode: `0x7D`
- assembly syntax: `ltu <r:dest> <m:arg1> <m:arg2>`
- behaviour: `dest = (arg1 < arg2) ? 1 : 0`

Sets the destination register to 1 if `arg1` is less than `arg2`. Sets the destination register to 0 otherwise.

The upper 31 bits of the destination register are always zero after this instruction. The low bit is set to the comparison result.

Note that a 32-bit two's complement comparison produces different results; you must be careful to perform an unsigned (logical) comparison. A signed comparison simulated by the `lts` instruction (and other `s`-suffixed instructions) in compound assembly.



### Jump If Zero

- opcode: `0x7E`
- assembly syntax: `<m:pred> <i:low> <i:high>`
- behaviour: `if !pred: rip += 4 * signext16((high << 8) \| low)`

Jumps by the given sign-extended 16-bit number of words if the predicate is zero. If any bit in the predicate is set, this does nothing.

If the predicate is zero, the `high` argument is shifted up by 8 bits and added to the `low` argument to form an offset. Then, if the high bit of the high argument is set, the upper 16 bits of the argument are set as well; this is called sign extension. Finally, the complete offset is shifted up by two bits (i.e. multiplied by 4) and added to the instruction pointer, discarding the carry.

The `low` and `high` arguments together form a 16-bit two's complement signed offset. This makes it possible to jump backwards with this instruction. Note that the VM does not actually need to handle any two's complement operations; it is equivalent to an unsigned addition with discarded carry.

Note that, unlike most instructions, the first argument is not a destination. If the predicate is a register it is unchanged by this instruction.

The predicate is mix-type. A predicate of 0 can be used to perform an unconditional jump. If the predicate is a non-zero, non-register constant, the instruction does nothing; this can be used as a "no operation" instruction.



## Calling Convention

Bytecode can use any mechanism for performing function calls, but there is a standard calling convention used by the Onramp C compiler and by most of the hand-written assembly and bytecode programs. This section describes the standard calling convention.

Arguments that are larger than a register (32 bits) are always passed on the stack, never in multiple registers. The first four register-sized or smaller arguments are passed in order in registers r0-r3 (even if they appear after larger arguments.) All other arguments are pushed on the stack in reverse order with their size rounded up to the nearest word.

If a function's return type is larger than a register, the caller must provide storage for the return value, and its address is pushed on the stack after all arguments (it is never passed in a register.) Finally, the return address is pushed last onto the stack before jumping into the callee.

The typical instruction sequence for the caller is:

- Push whatever registers you want to preserve to the stack;
- Push non-register arguments to the stack right-to-left;
- Place the first four word-size arguments left-to-right in `r0`-`r3`;
- Push `rip + 12` to the stack (the return address of the function)
- Load the program-relative address of the symbol into `ra`
- Jump to `ra + rpp`
- Move the return value from `r0` if necessary
- Restore registers from the stack
- Pop everything else from the stack

The callee typically begins by setting up a stack frame. The previous frame pointer (rfp) is pushed onto the stack, then the current stack pointer (rsp) is stored as the function's own frame pointer. The frame pointers therefore form a linked list of stack frames. The area above each frame pointer contains the stack-passed arguments and the area below it contains local variables. (Stack frame setup is done with the `enter` and `leave` assembly instructions; see the [assembly specification](assembly.md) for details.)

Registers r0-r9 (and ra-rb) are available for use within the function; the callee does not need to preserve them. The callee is also allowed to modify its non-register arguments on the stack, but it does not pop its arguments or return address. When the function returns, the stack pointer must have the same value as when it started (as do the frame pointer and program pointer.)

If the return type fits in a register, the return value is passed in register r0; otherwise, the return value is stored at the return address that was pushed to the stack by the caller.

This calling convention is designed to be simple at all stages of Onramp while still maintaining reasonable efficiency. Register-passing is by far the easiest mechanism for handwritten assembly, and the first stage C compiler only supports up to four arguments of at most register size, so these always pass all arguments in registers. The second stage C compiler adds structs but it cannot pass them by value, so it only needs to pass arguments 5 and later on the stack. The final stage C compiler supports passing and returning structs and 64-bit numbers as described above.

Handwritten bytecode programs that violate this convention describe the differences in their code comments. For example [ld/0](../core/ld/0-global) and [sh](../core/sh) consider `r9` to be a globally preserved register that points to global data tables.



### External Function Calls

An external function call is a function call that passes control to another program.

The external calling convention is used when making system calls. This allows system calls to be implemented by other programs running inside the Onramp VM or by the VM itself. For example, system calls are implemented by the VM, by the Onramp OS, and for child programs by the Onramp libc. (This could also in theory be used to implement dynamically linked libraries, although Onramp does not support them at this time.)

To make an external function call, the caller needs two words: the absolute address of the function and the base address of the program that contains it. The caller must set `rpp` to that of the external program before making the call. This also means it must preserve its own `rpp` along with its other registers so it can be restored afterwards. The caller typically pushes its `rpp` to the stack along with any other registers it preserves, and this must be done before pushing any function arguments and the return address. The caller then follows the normal calling convention, and restores its `rpp` afterwards along with its other registers.

From the perspective of the callee, an external function call is identical to a normal function call. The callee must preserve `rpp` as in a normal function call. (The caller is therefore allowed to repeat an external function call without resetting `rpp`.)

In other words, the only difference in an external function call is that both caller and callee preserve `rpp`. The caller simply needs to preserve and restore `rpp` along with its numbered registers.

The Onramp C compiler has no support for `rpp` manipulations or alternate calling conventions. External function calls must be implemented in assembly or bytecode.



## System Calls

A system call (or "syscall") is the mechanism by which a program accesses and modifies the outside environment.

A program makes system calls to perform input and output; access files and directories; get the current time; sleep; launch external programs; and more.

The VM provides the program with a table of system calls at startup. The program consults this table to determine what system calls are available, and then performs these calls to modify its environment.



### System Call Table

The process info table contains a pointer to the system call table. The system call table is an array of external function pointers; see the [external calling convention](#external-function-calls) above.

The length of the array is the number of syscalls. Each entry in the array is an external function pointer, which is two words. The first word is the absolute address of the function (i.e. the address to put in `rip`) and the second word is the base pointer of the external program (i.e. the address to put in `rpp`.)

For example, if the syscall table is at `0x2000`, the memory would look like this:

| Address   | Value             |
|-----------|-------------------|
| 0x2000    | `rip` of `exit`   |
| 0x2004    | `rpp` of `exit`   |
| 0x2008    | `rip` of `time`   |
| 0x200C    | `rpp` of `time`   |
| 0x2010    | `rip` of `panic`  |
| 0x2014    | `rpp` of `panic`  |
| 0x2018    | `rip` of `fopen`  |
| 0x201C    | `rpp` of `fopen`  |
| ...       | ...               |

If a syscall is unimplemented, the function address is `0`, and the base pointer may have any value.

To make a syscall, the program performs an external function call. It preserves its registers including `rpp`; pushes a return address; puts the syscall's base pointer into `rpp`; and puts the syscall's function address into `rip`. Upon returning, the program pops the stack and restores its `rpp` and other registers.

When syscalls are implemented by a parent program (such as the Onramp OS), typically the function address is the absolute address of the function within the parent, and the base pointer is the parent's `rpp`. All syscalls implemented by a particular parent program therefore typically have the same base pointer and different function addresses. The syscalls are then implemented as ordinary functions.

When syscalls are implemented by the VM, both the function address and base pointer can be set to arbitrary values (except that the function address cannot be 0.) Typically the function address is set to some sentinel value (e.g. `0xEEEEEEEE`) or to a mapped address with an invalid opcode (e.g. `7F` which is otherwise unused). The `rpp` is set to the syscall number. All syscalls implemented by VMs therefore typically have the same function address and different base pointers. When `rip` reaches the sentinel value, the VM loads the syscall number from `rpp`, performs the syscall, sets `r0` to the result and sets `rip` to the value at the top of the stack.

When a program is nested deep within other programs in a VM, the system call table will contain a mix of function pointers from various parents. For example, consider the Onramp assembler, running in the Onramp driver, running in the Onramp shell, running in the Onramp OS, running on a freestanding Onramp VM. Typically the direct parent (in this case the driver) will provide exit; the VM will implement time and fread/fwrite for terminal input/output; the OS will implement the file and directory syscalls; and both the OS and shell will proxy fread/fwrite to redirect to files or to the VM's terminal streams.



### System Call Quick Reference

All system calls return a word that contains either an error code, a return value, or 0 indicating success without a value. If a return value is omitted in the below table, the system call returns 0 on success and an error code on error.

Arguments are passed in `r0`, `r1`, `r2` and `r3`. The return value is placed in `r0`.

| Number | Required  | Name     | Arguments                | Return Value             |  Description                             |
|--------|-----------|----------|--------------------------|--------------------------|------------------------------------------|
| 0      | yes       | exit     | exit code                | n/a (doesn't return)     | exits the program                        |
| 1      |           | time     | out\_time[3]             |                          | gets the current time                    |
| 2      |           | panic    | exit code                | n/a (doesn't return)     | halts the VM                             |
| 3      | hosted    | fopen    | path, writeable          | handle                   | opens a file                             |
| 4      | hosted    | fclose   | handle                   |                          | closes a file                            |
| 5      | hosted    | fread    | handle, buffer, size     | bytes read               | reads from a file or stream              |
| 6      | yes       | fwrite   | handle, buffer, size     | bytes written            | writes to a file or stream               |
| 7      | hosted\*  | fseek    | handle, base, pos (x2)   |                          | seeks to a position in a file            |
| 8      | hosted\*  | ftell    | handle, out\_pos[2]      |                          | gets the current position in a file      |
| 9      |           | ftrunc   | handle, size (x2)        |                          | truncates a file                         |
| 10     | hosted    | dopen    | path                     | handle                   | opens a directory                        |
| 11     | hosted    | dclose   | handle                   |                          | closes a directory                       |
| 12     | hosted    | dread    | handle, buffer           |                          | reads one file entry from a directory    |
| 13     | hosted\*  | stat     | path, buffer             |                          | gets file metadata                       |
| 14     |           | rename   | path, path               |                          | renames a file                           |
| 15     |           | symlink  | path, path               |                          | creates a symlink                        |
| 16     | hosted    | unlink   | path                     |                          | deletes a file                           |
| 17     |           | chmod    | path, mode               |                          | changes permissions of a file            |
| 18     | hosted    | mkdir    | path                     |                          | creates a directory                      |
| 19     | hosted    | rmdir    | path                     |                          | deletes an empty directory               |
| 20     |           | spawn    | path, args, env, fds     | pid                      | runs a program outside the VM            |
| 21     |           | waitpid  | pid                      | exit code                | waits until an outside program exits     |
| 22     |           | debug    | address, path            |                          | loads debug info for a child program     |

\*: Entries marked "hosted\*" are currently required in a hosted environment but may not be required in future VM specs.

A description of each system call with a C-style prototype follows. (The C prototypes described below are declared by the libc in `#include <__onramp/__syscalls.h>`. They can be called as ordinary C functions, although such use is discouraged outside of the libc.)



### exit

```c
[[noreturn]] void __sys_exit(int exit_code);
```

- syscall number: 0
- argument in r0: exit code
- return value: n/a (does not return)

Exits the program with the given exit code.

This system call does not return.



### time

```c
int __sys_time(unsigned time[3]);
```

- syscall number: 1
- argument in r0: address at which to write the time
- return value in r0: always 0

Gets the current time, writing three words to the address given in r0.

The current time consists of a 64-bit number of seconds plus a 32-bit number of nanoseconds since the UNIX epoch (the start of January 1st, 1970.) These are written as three words to the given address:

- r0 + 0: The low 32 bits of the number of seconds
- r0 + 4: The high 32 bits of the number of seconds
- r0 + 8: The number of nanoseconds (0 to 999,999,999)

If this system call is implemented, it cannot fail. It must always set register r0 to 0. (TODO: allow this system call to be optional)



### panic

```c
int __sys_panic(int exit_code);
```

- syscall number: 2
- argument in r0: exit code

Halts the VM, exiting with the given non-zero error code.

This kills all programs running in the instance of the VM. It should only be used in case an error occurs that is unrecoverable by normal program exit. For example, this is called when memory corruption is detected by `malloc()`, because the memory of other processes in the VM may be corrupted as well.

The given exit code must be non-zero. (The VM is allowed to ignore this and does not need to check.)

This system call is optional and is not typically implemented by Onramp VMs. If not implemented, the Onramp libc uses the exit syscall instead. (This system call mainly exists so that programs can pass along the VM's exit to child programs so they can take down the whole VM if a critical error occurs.)



### fopen

```c
int __sys_fopen(const char* path, bool writeable);
```

- syscall number: 3
- argument in r0: address of a null-terminated string containing the path to open
- argument in r1: whether the file should be opened for writing
- return value in r0: file handle or error code

Opens the file at the given path, associating it with an integer file handle and returning it. The stream position is initially at the start of the stream.

The `writeable` argument (in r1) must be 0 or 1. If it is 1, the file will support writing (via `fwrite` and `ftrunc`), and will be created if it does not already exist.

If a file open for writing already exists, the contents are left intact. Since the initial position is at the start of the file, a subsequent write will overwrite the contents. To append to an existing file, the program must make an `fseek` call after opening it. To destroy the existing contents first, the program must make an `ftrunc` call.

If the file is a directory, the call fails. `dopen` should be used to open directories.

On success, a file handle is returned, which must not have the high bit set. This handle is valid only for file syscalls (i.e. those that start with `f` and take a `file_handle`.)



### fclose

```c
int __sys_fclose(int file_handle);
```

- syscall number: 4
- argument in r0: the handle of the file to close
- return value in r0: always 0

Closes the given file handle.

This can only be used to close files, not the input/output/error streams.

This system call must return 0; an `fclose` call cannot fail. If the given file handle is a standard stream or is invalid, the behaviour is undefined.



### fread

```c
int __sys_fread(int file_handle, void* buffer, int count);
```

- syscall number: 5
- argument in r0: the handle of the file or input stream from which to read
- argument in r1: address at which to store the read data
- argument in r2: the maximum number of bytes to read into the address at r1
- return value in r0: the number of bytes read or an error code

Reads up to `count` bytes into the given buffer, returning the number of bytes actually read or an error code if reading fails.

The **fread** syscall is used to read from files and from the input stream. When called on the input stream, it is intended to read terminal input, typically user keystrokes, into the program. The input should be in UTF-8 format and it may use [ANSI escape sequences](https://en.wikipedia.org/wiki/ANSI_escape_code) for special characters (such as arrow keys.)

Since platforms implement input differently, Onramp supports considerable variation in the implementation of fread. The behaviour of a VM's fread syscall must be accurately represented by the capabilities bits in the process info table as explained below.

Assuming the capabilities are accurately reported by the VM, the Onramp libc will simulate whatever behaviour is desired by the program where possible. For example, if the VM has non-blocking input and the program requests blocking input, the libc will perform blocking. However, if the VM is blocking and the program requests non-blocking input, the behaviour cannot be simulated so the libc will reject the request. If you are implementing a VM, follow the recommendations below to get maximum compatibility with programs running on Onramp.

If bytes are available, the VM must read at least one byte, but may read less than the number of bytes requested.

If no bytes are available, the VM should not wait for input, and should instead immediately return zero with no error. (Note this is different from POSIX which raises EAGAIN or EWOULDBLOCK.) If non-blocking input is not possible on the VM's platform, the VM may instead block until data is available; in this case it must set bit 2 in the capabilities field of the process info table.

When the user enters input, it should not be echoed to the output by the VM. If this is not possible on the VM's platform, the VM may instead echo input to the output; in this case it must set bit 0 in the capabilities field of the process info table.

The VM should make input keystrokes available immediately rather than waiting until the end of a line. If this is not possible on the VM's platform, the VM may instead wait until a full line has been processed before making it available to the fread syscall; in this case it must set bit 1 in the capabilities field of the process info table.



### fwrite

```c
int __sys_fwrite(int file_handle, void* buffer, int count);
```

- syscall number: 6
- argument in r0: the handle of the file or output/error stream in which to write
- argument in r1: address containing the data to write
- argument in r2: the maximum number of bytes to write from the address at r1
- return value in r0: the number of bytes read or an error code

Writes up to `count` bytes from the given buffer into the given file or stream, returning the number of bytes actually read or an error code if writing fails.

The `count` argument in r2 must be non-zero. The VM is allowed to assume it is never zero. (For example, it may ignore it and always write exactly one byte, although this would be inefficient.)

If space is available to write bytes, the VM must write at least one byte, but may write less than the number of bytes requested. If the output stream is full, 0 is returned.

This can only be called on the output stream, the error stream, or a file opened in writeable mode.



### fseek

```c
int __sys_fseek(int file_handle, int base, unsigned offset_low, int offset_high);
```

- syscall number: 7
- argument in r0: the handle of the file to seek
- argument in r1: the base position (0, 1, 2) from which to seek
- argument in r2: the low 32 bits of a 64-bit offset to add to the base position
- argument in r3: the high 32 bits of a 64-bit offset to add to the base position
- return value in r0: 0 on success or an error code

Sets the current position in the file to the given position.

The position is calculated as the given offset added to the given base.

- If `base` is 0, the offset is added to the start of the file. (In other words, it is an absolute offset into the file.)
- If `base` is 1, the offset is added to the current position.
- If `base` is 2, the offset is added to the end of the file (i.e. the start plus its size.)

If the VM's maximum file size is less than the range of a 32-bit word (i.e. 4GB), the `offset_high` parameter can be ignored. Otherwise, the file position must be stored as a 64-bit value.

This can only be called on files, not streams.



### ftell

```c
int __sys_ftell(int file_handle, unsigned position[2]);
```

- syscall number: 8
- argument in r0: the handle of the file from which to query the position
- argument in r1: the address at which to store the 64-bit position in the file
- return value in r0: always 0

Stores the current position in the given file to the given `position` address.

The VM must write two words: the low 32 bits of the position followed by the high 32 bits of the position.

The outputted value can be used in a call to `fseek` with base 0 to return to this position in the file.

If the VM's maximum file size is less than the range of a 32-bit word (i.e. 4GB), the VM must still write a second word to the output with value zero.

This system call cannot fail. It can only be used on file handles. If used on the input/output/error streams, or if the file handle is invalid, the behaviour is undefined.



### ftrunc

```c
int __sys_ftrunc(int file_handle, unsigned size_low, unsigned size_high);
```

- syscall number: 9
- argument in r0: the handle of the file to resize
- argument in r1: the low 32 bits of the 64-bit size to set
- argument in r2: the high 32 bits of the 64-bit size to set
- return value in r0: 0 on success or an error code

Sets the size of the file to the given size.

If the size is less than the current size of the file, the file is truncated: its size becomes that given and all data beyond that size is destroyed.

If the size is greater than the current size, the VM may ignore it and return 0xFFFFFFFC (not supported), or it may append zero bytes to the file until the size becomes that given. (VMs may internally optimize this to use sparse files.)

Returns zero if successful. In case of success, the file's size matches that given.

If the VM's maximum file size is less than the range of a 32-bit word (i.e. 4GB), the VM must return an error (such as 0xFFFFFFFC not supported) if the `size_high` argument is not zero.



### dopen

```c
int __sys_dopen(const char* path);
```

- syscall number: 10
- argument in r0: address of a null-terminated string containing the path to open
- return value in r0: directory handle or error code

Opens the directory at the given path, associating it with an integer directory handle and returning it.

On success, a directory handle is returned, which must not have the high bit set. This handle is valid only for directory syscalls (i.e `dread` and `dclose`.)

Directory handles are independent from file handles; directory handle 0 is different from file handle 0, and both may exist simultaneously. (The Onramp libc remaps them to separate POSIX file handles.)

The directory handle is used to read directory entries. A sequence of `dread` calls reads directory entries and `dclose` closes it.

If the directory is modified while a directory handle is open, the behaviour is undefined.



### dclose

```c
int __sys_dclose(int directory_handle);
```

- syscall number: 11
- argument in r0: the handle of the directory to close
- return value in r0: always 0

Closes the directory associated with the given handle.

This system call must return 0; a `dclose` call cannot fail. If the given directory handle is invalid, the behaviour is undefined.



### dread

```c
int __sys_dread(int directory_handle, char buffer[256]);
```

- syscall number: 12
- argument in r0: the handle of the directory to read
- argument in r1: address of a buffer in which to write the filename read
- return value in r0: 0 on success, error code otherwise

Reads the next file or subdirectory entry from the given directory into the given buffer as a null-terminated string.

If there are no more entries, an empty string is placed in the buffer (by writing a 0 byte to the first character) and 0 (success) is returned.



### stat

```c
int __sys_stat(const char* path, unsigned output[4]);
```

- syscall number: 13
- argument in r0: address of a null-terminated string containing the path to query
- argument in r1: address at which to write the file information

Queries information about the given path, writing it to the given address.

If a file, directory or symlink exists at the given path, the following words are written to the output address in order:

- r1 + 0: `type` -- 2 if the path is a symlink, 1 if the path is a directory, 0 if the path is a file
- r1 + 4: `mode` -- Either 493 (0755) if the file is executable, 420 (0644) if it is not, or 0 if it is not a file.
- r1 + 8: `size_low` -- The low 32 bits of the size of the file
- r1 + 16: `size_high` -- The high 32 bits of the size of the file

If the VM's maximum file size is less than the range of a 32-bit word (i.e. 4GB), the VM must write zero to the `size_high` field (provided the path exists and is a file.)



### rename

```c
int __sys_rename(const char* from, const char* to);
```

- syscall number: 14
- argument in r0: address of a null-terminated string containing the path of the source file or directory
- argument in r1: address of a null-terminated string containing the path of the destination file or directory
- return value in r0: 0 on success or an error code

Moves and/or renames a file or directory.

TODO define this better, probably we should require the destination to always be a full path (not a directory name), the libc should stat the destination and append the filename if it's a directory



### symlink

```c
int __sys_symlink(const char* from, const char* to);
```

- syscall number: 15
- argument in r0: address of a null-terminated string containing the path of the source file or directory
- argument in r1: address of a null-terminated string containing the path of the destination file or directory
- return value in r0: 0 on success or an error code

Creates a symlink.

If the destination already exists and is a file, this may overwrite it, or it may return an error. If the destination already exists and is a directory, this must return an error.

TODO explain symlinks
TODO symlink support should be optional.



### unlink

```c
int __sys_unlink(const char* path);
```

- syscall number: 16
- argument in r0: address of a null-terminated string containing the path of the file or symlink to delete
- return value in r0: 0 on success or an error code

Deletes the file or symlink at the given path. If this is used on a directory, an error is returned; `rmdir` must be used for directories.



### chmod

```c
int __sys_chmod(const char* path, int mode);
```

- syscall number: 17
- argument in r0: address of a null-terminated string containing the path of the file for which to change the executable flag
- return value in r0: 0 on success or an error code

Sets whether the file at the given path is executable in the host environment. Only two values are supported for mode:

- 493 (0755) -- The file is executable
- 420 (0644) -- The file is not executable

This is just used for better integration of wrapped binaries into the host system. It can be ignored.

TODO this should be optional, and if implemented should not be ignored.



### mkdir

```c
int __sys_mkdir(const char* path);
```

- syscall number: 18
- argument in r0: address of a null-terminated string containing the path of directory to create
- return value in r0: 0 on success or an error code

Creates an empty directory at the given path.

If the path already exists, this returns an error.

If the parent path does not exist, this returns an error. (It does not create directories recursively.)



### rmdir

```c
int __sys_rmdir(const char* path);
```

- syscall number: 19
- argument in r0: address of a null-terminated string containing the path of the empty directory to delete
- return value in r0: 0 on success or an error code

Deletes an empty directory at the given path.

If the directory is not empty, this returns an error. If the directory does not exist, this returns an error. If the path is not a directory, this returns an error.



### spawn


```c
int __sys_spawn(TODO);
```

- syscall number: 20

Spawns an external program in a hosted environment.

This is not yet implemented.



### debug

```c
int __sys_debug(const void* address, const char* /*nullable*/ executable_path);
```

Loads or unloads debug info for a child program at the given address.

If the given path is null, previously loaded debug info is unloaded.

If the given path is non-null, corresponding debug info is loaded for the executable at the given path.

Note that if a path is given, it must point to the executable, not to the debug info file. This allows VMs to store debug info in custom formats or locations. (The standard debug info format appends `.od` to the executable path.)

Returns 0 if successful and an error code otherwise.

This syscall is optional and most VMs do not implement it. The [c-debugger](../platform/vm/c-debugger) VM is the main consumer of this syscall.



## Filesystem

A filesystem is made up of directories and files. Directories can contain other directories and files. Files contain data of arbitrary type and length, and grow automatically as data is written to them.

The character `/` is used to delimit files and directories. A file or directory name can contain any character except `/` and a null byte. (Onramp itself only uses ASCII letters, numbers, and the characters `-`, `_` and `.` in its implementation. Programs compiled by Onramp may use any characters.)

The filesystem must have a root directory. A path is a string of up to 255 bytes that contains the hierarchy of directories that must be navigated from the root to reach a file. For example `/foo/bar` is a path to a file or directory called `bar` in a directory called `foo` in the root directory.

The VM must provide a directory to store temporary files. If it is not called `/tmp/`, a `TMPDIR` environment variable must be provided that contains its path.

The filesystem implemented by a VM resembles that of a POSIX system as described above. If your host filesystem is different, the VM must translate paths to make them appropriate for Onramp. For example if you have a path like `C:\Foo\Bar`, the VM should translate it to something like `/c/Foo/Bar`.



### I/O Handles

Input and output is done through "handles". A handle is a 32-bit integer that represents an open file, directory, or stream.

(These are similar to file descriptors in POSIX. We call them handles because the libc needs to translate them to POSIX-style file descriptors to simulate POSIX APIs.)

Up to three handles are reserved for the standard input/output streams (see below.)

If the VM is hosted, other handles should be available for the program to open files and directories on the filesystem. If the VM is freestanding, the write (and (optionally) read syscalls will only be used on the input/output streams, and all other I/O syscalls should not be implemented.



### Input/Output Streams

There are three I/O streams: input, output and error. These are intended for programs to interact with other programs and with a user.

#### Input

Implementation of the input stream is optional. If input is not supported, the input handle in the process info table should be set to -1. It is normal for a VM to have no input, for example when performing non-interactive bootstrapping.

If input is supported, the input stream should not never block. It should not wait for input to become available, and it should not wait until a particular state is reached (such as the end of a line.) A read on the input should immediately return any and all available data; if no data is available, the read should return success with zero bytes read.

The virtual machine should also not echo input, i.e. it should not print input characters to the output stream on its own. The Onramp libc will handle blocking, buffering and echo internally.

(On POSIX systems, this means the input file handle should be non-blocking, should be in non-canonical mode, and should have echo disabled.)

#### Output and Error

The output stream is intended for normal program output, that could for instance be consumed by another program.

The error stream is intended for displaying errors, warnings and other abnormal ouput to a user.

The output and error streams are otherwise identical. If no distinction is required between them, the VM can use the same I/O handle for both. (It can also use the same handle for input.)



### Error Handling

Most system calls return a 32-bit word. When a system call fails, it returns one of the following error codes:

- `0xFFFFFFFF` -- Generic error
- `0xFFFFFFFE` -- Path does not exist
- `0xFFFFFFFD` -- Input/output error
- `0xFFFFFFFC` -- Not supported (by VM or by host environment)

All error codes have the high bit set. Values that can be returned from successful system calls (such as file and directory handles) do not have the high bit set. If a system call does not return a value, it returns 0 on success.

If a system call is used incorrectly (e.g. an invalid argument value is passed), the behaviour is undefined. (An error-checking VM should detect this and halt.)



## Debug Info

Debug info can be emitted by the final stages of the Onramp toolchain (and some earlier stage tools as well.) It is enabled by passing `-g` to the driver or to individual tools.

For intermediate output types, debug info is intermixed with the output and generally consists of C-style `#line` and `#pragma` directives.

For an Onramp executable, debug info is stored in a corresponding debug file with an additional `.od` extension. (For an unwrapped `.oe` program, the extension is therefore `.oe.od`; for a program wrapped for POSIX, it's usually just `.od`.) This is a plain-text file that describes the symbols of the executable. See the [debug info specification](debug-info.md) for a description of this format.

Most VMs ignore debug info. The [`c-debugger`](../platform/vm/c-debugger/) VM loads it and uses it to display annotated stack traces and disassembly.



## File Format

An Onramp executable typically has a `.oe` extension and contains the raw bytecode of the program. It does not need any preamble and there are no segments or other metadata in the file. A VM can simply load the entire file into memory at an arbitrary (aligned) address, provide it with a process info table and start executing it.

However, by convention, Onramp VM programs start with the following instructions:

```
7E 4F 6E 72   ; jz 79 29294
7E 61 6D 70   ; jz 97 28781
7E 20 20 20   ; jz 32 8224
```

These are conditional jump-if-zero instructions, and since their predicates are non-zero constants, they do nothing. However, their encoding in ASCII is "`~Onr~amp~   `". This serves as a format indicator that identifies a file as containing an Onramp program. This preamble is not required but VM implementations may warn if a program does not start with it.

On some hosted platforms, Onramp bytecode can also be wrapped in a script that executes the Onramp virtual machine. This allows them to be executed like normal programs. For example, the POSIX wrapper looks like this:

```
#!/usr/bin/env onrampvm
# This is a wrapped Onramp program.
```

This script automatically launches the program in the VM (as long as it's on your PATH.)

VMs that support these platforms check for a script preamble (e.g. `#!` or `REM`). If found, they skip the first 128 bytes.



## Rationale

The Onramp VM is designed to balance the following constraints:

- The machine must be easy to implement:
    - In both raw machine code and in high-level languages;
    - In both freestanding and hosted environments;

- The bytecode must be easy to read and write by hand:
    - In raw hex bytes, in a powerful assembly language, and any step in between;

- The bytecode must be easy to produce as the output of a compiler.

It has a few additional requirements:

- The VM must bridge the filesystem in a hosted environment;

- The VM must make it easy for programs to run other programs.

Non-goals are efficiency, memory safety, and suitability for implementation in hardware. The VM is designed primarily for non-interactive bootstrapping of a real native compiler. Interrupts are also not a priority at this time, although there is the possibility of implementing them later.

Onramp's VM design takes inspiration from such projects as Robert Elder's [one page CPU](https://recc.robertelder.org/op-cpu-programmer-reference-manual.txt), the [TOY machine](https://introcs.cs.princeton.edu/java/62toy/) from Sedgewick and Wayne, the design of [MessagePack](https://msgpack.org/), classic architectures like PDP-11 (designed to be programmed directly in octal), and of course modern RISC ISAs like RISC-V. See the [inspiration](inspiration.md) page for details.


## Version History

Version 2: Added syscall table, replacing the exit address in the PIT (an address that was assigned to `rip` to exit the program.). Removed `sys` instruction. Renamed `halt` syscall to `exit`. Added `panic` syscall.

Version 1: Replaced `cmpu` instruction with `ltu`. (The `cmpu` instruction took a destination register and two source mix-type bytes. It performed a three-way comparison between the sources. It placed 1 in the register if the first source argument was greater than the second; 0xFFFFFFFF if the first was less than the second; and 0 if the source arguments matched.)

Version 0: Initial version.

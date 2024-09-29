# Onramp Virtual Machine and Bytecode

The Onramp Virtual Machine is a simple virtual machine designed for portable bootstrapping.

This document specifies the virtual machine and its bytecode. For a description of implementations, see [Onramp Virtual Machine Implementations](../platform/vm).



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

Non-goals are efficiency, memory safety, multithreading, interrupts, and suitability for implementation in hardware. The VM is designed primarily for non-interactive bootstrapping of a real native compiler.

Onramp's VM design takes inspiration from such projects as Robert Elder's [one page CPU](https://recc.robertelder.org/op-cpu-programmer-reference-manual.txt), the [TOY machine](https://introcs.cs.princeton.edu/java/62toy/) from Sedgewick and Wayne, the design of [MessagePack](https://msgpack.org/), classic architectures like PDP-11 (designed to be programmed directly in octal), and of course modern RISC ISAs like RISC-V. See the [inspiration](inspiration.md) page for details.



## Overview

The Onramp VM is a register-based RISC machine in a von Neumann architecture.

A large, contiguous region of memory is available to the program which initially contains its code. Programs typically divide the remainder into a heap and a stack.

Programs are stored as a series of 32-bit bytecode instructions in a binary file format. Opcodes and arguments each occupy individual bytes and have distinct prefixes. This makes it easy to read and write the bytecode in hexadecimal.

There are 16 registers. The first twelve are general purpose, the first four of which are used as function arguments and return values. The last four registers are the stack pointer, frame pointer, program pointer and instruction pointer respectively. There is no flags register. Carry and overflow must be detected manually, and compare and jump instructions use arbitary registers.

A small number of mostly orthogonal instructions are provided. Such basic functionality as pushing data onto the stack or jumping to an address do not have dedicated instructions. They must be emulated with more fundamental instructions, such as adding two values into a register or storing a word at a memory address. Emulation of higher level instructions is provided by an advanced assembler later on in the bootstrap process which reserves the last two general-purpose registers (`ra` and `rb`.)

There are (currently) no interrupts. The VM is designed for non-interactive computation. It does however support input and time so it is possible to write interactive terminal applications.

All programs are position-independent. This makes it possible for programs to run other programs without the need for memory protection. The program pointer (`rpp`) contains the base address of the currently running program.

An Onramp VM can run hosted or freestanding. When hosted, the platform's filesystem is bridged into the virtual machine. This requires the implementation of a number of system calls. When freestanding, an OS runs inside the Onramp VM. The VM passes most system calls over to the contained OS which implements the filesystem.

When writing C programs and compiling them for Onramp, you do not need to worry about any of this. It is handled by the Onramp libc.



## Registers

There are sixteen registers numbered `0x80` to `0x8F`. All instructions can operate on all registers, but some registers have special behaviour (such as the instruction pointer) and others have strong conventions on their use (such as the stack and program pointers.) These latter registers are given special names.

- Registers `r0`, `r1`, `r2` and `r3` are general-purpose caller-preserved registers that are used as function arguments and return values. See the calling convention section below.
- Registers `r4`, `r5`, `r6`, `r7`, `r8` and `r9` are general purpose caller-preserved registers. These are typically used for local variables in functions.
- Registers `ra` and `rb` are "scratch space" registers. They are clobbered not only by function calls but also by compound assembly instructions. They can be used for temporary space when writing bytecode or basic assembly by hand but they are best avoided when writing or emitting full assembly.
- Register `rsp` is the stack pointer. It points to the last value pushed on the stack. The Onramp VM stack grows down.
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
| `rpp`     | `8E`      | Program pointer (start of program)                          | Callee       |
| `rip`     | `8F`      | Instruction pointer (next instruction to be executed)       | Caller       |

There is no status or flags register. Detecting overflow, underflow and other such conditions must be done manually. Instructions such as `cmpu` (compare unsigned) and `jz` (jump if zero) access the predicate in a register of your choice.



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

The entire memory region from `rpp` to the initial `rsp` is writable and executable. The process information table typically sits outside of this region and must not be written to.

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

The process information table is an array of 32-bit words. It contains the following information:

| Index | Value                  |
|-------|------------------------|
| 0     | Version                |
| 1     | Program Break          |
| 2     | Exit Address           |
| 3     | Input stream handle    |
| 4     | Output stream handle   |
| 5     | Error stream handle    |
| 6     | Command-Line Arguments |
| 7     | Environment Variables  |
| 8     | Working directory      |
| 9     | Capabilities           |

The parent process of a program (the VM or otherwise) must assemble this table somewhere in memory accessible to the program and pass a pointer to it in `r0`.

The version field contains the version of the Onramp VM. The information table is intended to be forward-compatible, so to perform a version check, ensure the version is at least as large as the version you need. The current version is 0 so you can ignore this field.

The program break is the address of one past the last byte of the program bytecode. In other words it's the start of the heap, which programs typically use for `malloc()`.

The exit address contains an address that the program should assign to `rip` in order to exit. The program must first place an 8-bit unsigned exit code in `r0`.

Handles for the input, output and error streams may have any value (and may even all have the same value.) The program must use these handles to communicate with the outside world. If a handle has value 0xFFFFFFFF, this indicates that the stream does not exist, and the program should avoid using it.

Command-line arguments and environment variables are stored in null-terminated arrays of null-terminated strings. In a hosted environment, the command-line always has at least one string, which is the path to the program being run. Environment variables are typically key-value pairs delimited by `=`. The substring to the left of the first `=` is the key, while the substring to the right is the value.

The working directory is the base directory that the program should use for relative paths. Note that VM syscalls have no concept of a working directory: all paths must be absolute. It is up to the program to track its own working directory and append relative paths to it before calling VM syscalls. (This is handled by the Onramp libc.) (TODO this is not done yet, currently the VMs all support paths relative to the initial working directory.)

In a freestanding environment, the command-line, environment variables and working directory may all be null.

The capabilities field contains a set of flags describing what features are supported by the VM. (The capabilities bitmap is not yet defined. For now it's just zero.)

Note that the process information table and its associated information must not be written to, except that command-line arguments and environment variables may be modified (for example with `strtok()`.) Any other changes are undefined behaviour, and may crash the VM or corrupt the parent process.



## Position-Independence

All Onramp VM bytecode programs are position-independent. Programs can be loaded at any address in memory.

A special register, called the *program pointer* (`rpp`), contains the base address of the program in memory. References to symbols in the program (such as functions and and global variables) are relative to the start of the program. Their absolute address can be determined by adding `rpp`.

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

(This would of course be done after preparing the arguments; see the function call convention below.)

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

- A "reg" argument is the name of a register. It must be a byte in the range `80`-`8F`.
- An "imm" argument is a literal byte. It can have any value.
- A "mix" argument is one byte that translates to a 32-bit value. How it is translated depends on its hexadecimal prefix:
    - If it is in the range `80`-`8F`, its value is the content of the named register.
    - If it is in the range `00`-`7F`, it is an immediate positive value with high bits set to zero.
    - If it is in the range `90`-`FF`, it is an immediate negative value; it is sign-extended, i.e. the high 24 bits are set.

Most instructions take mix-type arguments as input. This makes it easy to do math between registers and small immediate values without complicating the instruction set. Conversely, the destination of instructions are almost always reg-type.



## Negative Values

This specification does not prescribe any representation for signed numbers. There are no signed operations at the VM level, and a VM can be implemented using purely unsigned arithmetic with standard overflow behaviour. (A goal here is to maximize the kinds of alien architectures that can easily implement Onramp.)

The only thing the VM needs to be able to do in relation to negative numbers is sign extension. The VM must be able to copy the high bit of an 8-bit or 16-bit value to the upper bits of a 32-bit word. This must be done in two cases:

- When a mix-type byte is in the range `0x90`-`0xFF`, the upper bits must be set to 1 to extend it to 32 bits. (In order words, when a mix-type byte is not a register, the 8th bit must be copied to the upper 24 bits.)

- The 16th bit of the two-byte offset in a conditional jump instruction must be copied to the upper bits to extend it to a full 32-bit word. (It can then be added to the instruction pointer using an ordinary unsigned addition that wraps to 32 bits.)

Programs compiled by the Onramp compiler and assembler use two's complement to represent signed numbers. All signed operations are reduced to unsigned VM instructions by the assembler.



### Opcodes

Opcodes are divided into four groups: arithmetic, logic, memory and control. Each group has four opcodes.

Arguments have the following types:

- `r`: A register
- `m`: A mix-type byte, either a register or an immediate value in the range [-112,127]
- `i`: An immediate byte (any value)

Here's a quick reference table for all supported instruction opcodes:

Arithmetic:

| Opcode        | Name          | Arguments                       | Operation              |
|---------------|---------------|---------------------------------|------------------------|
| `0x70` `add`  | Add           | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 + arg2`   |
| `0x71` `sub`  | Subtract      | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 - arg2`   |
| `0x72` `mul`  | Multiply      | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 * arg2`   |
| `0x73` `div`  | Divide        | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 / arg2`   |

Logic:

| Opcode         | Name                 | Arguments                       | Operation                                     |
|----------------|----------------------|---------------------------------|-----------------------------------------------|
| `0x74` `and`   | Bitwise And          | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 & arg2`                          |
| `0x75` `or`    | Bitwise Or           | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 \| arg2`                         |
| `0x76` `shl`   | Shift Left           | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 << arg2`                         |
| `0x77` `shru`  | Shift Right Unsigned | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 >> arg2` (unsigned)              |

Memory:

| Opcode        | Name          | Arguments                       | Operation                                  |
|---------------|---------------|---------------------------------|--------------------------------------------|
| `0x78` `ldw`  | Load Word     | `<r:dest> <m:base> <m:offset>`  | `dest = *(int*)(base + offset)`            |
| `0x79` `stw`  | Store Word    | `<m:src> <m:base> <m:offset>`   | `*(int*)(base + offset) = src`             |
| `0x7A` `ldb`  | Load Byte     | `<r:dest> <m:base> <m:offset>`  | `dest = *(char*)(base + offset)`           |
| `0x7B` `stb`  | Store Byte    | `<m:src> <m:base> <m:offset>`   | `*(char*)(base + offset) = src & 0xFF`     |

Control:

| Opcode        | Name          | Arguments                       | Operation                                                |
|---------------|---------------|---------------------------------|----------------------------------------------------------|
| `0x7C` `ims`  | Immediate     | `<r:dest> <i:low> <i:high>`     | `dest = (dest << 16) \| (high << 8) \| low`              |
| `0x7D` `cmpu` | Compare       | `<r:dest> <m:arg1> <m:arg2>`    | `dest = (-1 or 0 or 1) for arg1 (< or = or >) arg2`      |
| `0x7E` `jz`   | Jump If Zero  | `<m:pred> <i:low> <i:high>`     | `if !pred: rip = rip + 4 * signext((high << 8) \| low)`  |
| `0x7F` `sys`  | System Call   | `<i:syscall> 00 00`             | system call                                              |

All arithmetic and logic opcodes have the same format. They take a destination register and two mix-type arguments. They perform a mathematical operation on the arguments and place the result in the given register. All operations are unsigned.

WARNING: `cmpu` will be replaced by `ltu` soon, `sys` will probably be removed, and possibly other changes will be made as well. This will require changing all the VMs and bytecode programs. If you implement a VM now be aware that you will need to update it later.



## Function Call Convention

Bytecode can use any mechanism for performing function calls, but there is a standard calling convention used by the Onramp C compiler and by most of the hand-written assembly and bytecode programs. This section describes the standard calling convention.

Arguments that are larger than a register (32 bits) are always passed on the stack (never in multiple registers.) The first four register-sized or smaller arguments are passed in registers r0-r3 (even if they appear after larger arguments.) All other arguments are pushed on the stack right-to-left (with their size rounded up to the nearest word.)

If a function's return type is larger than a register, the caller must provide storage for the return value, and its address is pushed on the stack after all arguments (it is never passed in a register.) Finally, the return address is pushed last onto the stack before jumping into the callee.

The first thing most functions do is to set up a stack frame: they push the previous frame pointer (rfp) onto the stack, then store the current stack pointer (rsp) as their own frame pointer. The frame pointers therefore form a linked list of stack frames. The area above each frame pointer contains the stack-passed arguments and the area below it contains local variables. (Stack frame setup is done with the `enter` and `leave` assembly instructions; see the [assembly specification](assembly.md) for details.)

Registers r0-r9 (and ra-rb) are available for use within the function; the callee does not need to preserve them. Only rsp, rfp and rpp must be restored to their original values when the function returns, and the caller is responsible for popping the arguments from the stack. (A change to rpp is rare: it is only used when a program spawns another program inside the VM.)

If the return type fits in a register, the return value is passed in register r0; otherwise, the return value is stored at the return address that was pushed to the stack by the caller.

This calling convention is designed to be simple at all stages of Onramp while still maintaining reasonable efficiency. Register-passing is by far the easiest mechanism for handwritten assembly, and the first stage C compiler only supports up to four arguments of at most register size, so these always pass all arguments in registers. The second stage C compiler adds structs but it cannot pass them by value, so it only needs to pass arguments 5 and later on the stack. The final stage C compiler supports passing and returning structs and 64-bit numbers as described above.

Handwritten bytecode programs that violate this convention describe the differences in their code comments. For example [ld/0](../core/ld/0-global) and [sh](../core/sh) consider `r9` to be a globally preserved register that points to global data tables.



## System Calls

The `sys` instruction is used to perform a system call. This is a request to the VM to perform some special function.

In a hosted environment, these are typically implemented by the VM. A freestanding VM may implement only some system calls, passing most others to an OS running inside the VM.

Making a system call is similar to making a function call. The main difference is that you do not push a return address. Arguments are passed in registers and on the stack, not in the instruction itself; the additional two bytes in the `sys` instruction must be zero.

- Push whatever registers you want to preserve;
- Place the arguments in `r0`-`r3`;
- Push additional arguments to the stack right-to-left;
- Perform the `sys` instruction
- Retrieve the return value in `r0`-`r3`
- Restore your registers from the stack and clean it up.

Note that you do not push a return address as you would with a function call. System calls also preserve all registers (except for the instruction pointer and return value registers.)

TODO: The above will likely change soon; system calls will be more like function calls, requiring a return address and not preserving registers. The `sys` instruction might be removed as well, instead being replaced by a function call table passed in the process info table. This will require changes to a few bytecode programs and to the libc. These changes are necessary to simplify the Onramp OS.


### System Call Quick Reference

This is a quick reference table. All system calls return a word that contains either an error code, a return value, or 0 indicating success without a value. If a return value is listed as "none" in the below table, the system call returns 0 on success.

Misc:

| Hex | Name     | Arguments            | Return Value             |  Description                    |
|-----|----------|----------------------|--------------------------|---------------------------------|
| 00  | halt     | exit code            | n/a (doesn't return)     | halts the VM                    |
| 01  | time     | out\_time[3]         | none                     | gets the current time           |
| 02  | spawn    | path, in, out, err   | none                     | runs a program outside the VM   |

Files:

| Hex | Name     | Arguments                | Return Value             |  Description                    |
|-----|----------|--------------------------|--------------------------|---------------------------------|
| 03  | fopen    | path, writeable          | handle                   | opens a file                    |
| 04  | fclose   | handle                   | none                     | closes a file                   |
| 05  | fread    | handle, buffer, size     | number of bytes read     | reads from a file or stream     |
| 06  | fwrite   | handle, buffer, size     | number of bytes written  | writes to a file or stream      |
| 07  | fseek    | handle, base, pos (x2)   | none                     | seeks to a position in a file   |
| 08  | ftell    | handle, out\_pos[2]      | current position         | gets the current position in a file   |
| 09  | ftrunc   | handle, size (x2)        | none                     | truncates a file                |

Directories:

| Hex | Name     | Arguments            | Return Value             |  Description                          |
|-----|----------|----------------------|--------------------------|---------------------------------------|
| 0A  | dopen    | path                 | handle or error code     | opens a directory                     |
| 0B  | dclose   | handle               | none                     | closes a directory                    |
| 0C  | dread    | handle, buffer       | error code               | reads one file entry from a directory |

Filesystem:

| Hex | Name     | Arguments            | Return Value            |  Description                    |
|-----|----------|----------------------|-------------------------|---------------------------------|
| 0D  | stat     | path, buffer         | none                    | gets file metadata              |
| 0E  | rename   | path, path           | none                    | renames a file                  |
| 0F  | symlink  | path, path           | none                    | creats a symlink                |
| 10  | unlink   | path                 | none                    | deletes a file                  |
| 11  | chmod    | path, mode           | none                    | changes permissions of a file   |
| 12  | mkdir    | path                 | none                    | creates a directory             |
| 13  | rmdir    | path                 | none                    | deletes an empty directory      |



### Filesystem

A filesystem is made up of directories and files. Directories can contain other directories and files. Files contain data of arbitrary type and length, and grow automatically as data is written to them.

The character `/` is used to delimit files and directories. A file or directory name can contain any character except `/` and a null byte. (Onramp itself only uses ASCII letters, numbers, and the characters `-`, `_` and `.` in its implementation. Programs compiled by Onramp may use any characters.)

The filesystem must have a root directory. A path is a string of up to 255 bytes that contains the hierarchy of directories that must be navigated from the root to reach a file. For example `/foo/bar` is a path to a file or directory called `bar` in a directory called `foo` in the root directory.

The VM must provide a directory to store temporary files. If it is not called `/tmp/`, a `TMPDIR` environment variable must be provided that contains its path.

The filesystem implemented by a VM resembles that of a POSIX system as described above. If your host filesystem is different, the VM must translate paths to make them appropriate for Onramp. For example if you have a path like `C:\Foo\Bar`, the VM should translate it to something like `/c/Foo/Bar`.



### I/O Handles

Input and output is done through "handles". A handle is a 32-bit integer that represents an open file, directory, or stream.

(These are similar to file descriptors in POSIX. We call them handles because the libc needs to translate them to POSIX-style file descriptors to simulate POSIX APIs.)



### Error Handling

Most system calls return a 32-bit word. When a system call fails, it returns one of the following error codes:

- `0xFFFFFFFF` -- Generic error
- `0xFFFFFFFE` -- Path does not exist
- `0xFFFFFFFD` -- Input/output error
- `0xFFFFFFFC` -- Not supported (by VM or by host environment)

All error codes have the high bit set. Values that can be returned from successful system calls (such as file and directory handles) do not have the high bit set. If a system call does not return a value, it returns 0 on success.

If a system call is used incorrectly (e.g. an invalid argument value is passed), the behaviour is undefined. (An error-checking VM should detect this and halt.)



### System Call Specifications

A description of each system call with a C-style prototype follows.


```c
[[noreturn]] void halt(int exit_code);
```

Halts the VM, returning to a host environment (if any) with the given exit code.


```c
int time(unsigned time[3]);
```

Gets the current time.

The current time consists of a 64-bit number of seconds plus a 32-bit number of nanoseconds since the UNIX epoch (the start of January 1st, 1970.) These are written as three words to the given address:

- The low 32 bits of the number of seconds
- The high 32 bits of the number of seconds
- The number of nanoseconds (0 to 999,999,999)


```c
int spawn(TODO);
```

Spawns an external program in a hosted environment.


```c
int fopen(const char* path, bool writeable);
```

Opens the file at the given path, associating it with an integer file handle and returning it. The stream position is initially at the start of the stream.

The `writeable` argument must be 0 or 1. If it is 1, the file will support writing (via `fwrite` and `ftrunc`), and will be created if it does not already exist. If a file open for writing already exists, the contents are left intact.

Note that subsequent writes will overwrite data without truncating. To append to an existing file, the program must make a `fseek` call after opening it, and to destroy the existing contents first, the program must make a `ftrunc` call.

If the file is a directory, the call fails. (`dopen` should be used to open directories.)

On success, a file handle is returned, which must not have the high bit set. This handle is valid only for file syscalls (i.e. those that start with `f` and take a `file_handle`.)


```c
int fclose(int file_handle);
```

Closes the given file handle.

This system call must return 0; an `fclose` call cannot fail. If the given file handle is invalid, the behaviour is undefined.


```c
int fread(int file_handle, void* buffer, int count);
```

Reads up to `count` bytes into the given buffer, returning the number of bytes actually read or an error code if reading fails.

If bytes are available, the VM must read at least one byte, but may read less than the number of bytes requested. If the end of the file or stream is reached, 0 is returned.


```c
int fwrite(int file_handle, void* buffer, int count);
```

Writes up to `count` bytes into the given buffer, returning the number of bytes actually read or an error code if writing fails.

If space is available to write bytes, the VM must write at least one byte, but may write less than the number of bytes requested. If the output stream is full, 0 is returned.


```c
int fseek(int file_handle, int base, unsigned offset_low, int offset_high);
```

Sets the current position in the file associated with the given handle to the given position.

The position is calculated as the given offset added to the given base.

- If `base` is 0, the offset is added to the start of the file.
- If `base` is 1, the offset is added to the current position.
- If `base` is 2, the offset is added to the end of the file (i.e. the start plus its size.)

If the VM's maximum file size is less than the range of a 32-bit word (i.e. 4GB), the `offset_high` parameter can be ignored. Otherwise, the file position must be stored as a 64-bit value.


```c
int ftell(int file_handle, unsigned* position);
```

Writes the current position in the given file to the given `position` address.

The VM must write two words: the low 32 bits of the position followed by the high 32 bits of the position.

The outputted value can be used in a call to `fseek()` with `base` 0 to return to this position in the file.

If the VM's maximum file size is less than the range of a 32-bit word (i.e. 4GB), the VM must still write a second word to the output with value zero.


```c
int ftrunc(int file_handle, unsigned size_low, unsigned size_high);
```

Sets the size of the file to the given size.

If the size is less than the current size of the file, the file is truncated: its size becomes that given and all data beyond that size is destroyed.

If the size is greater than the current size, the VM may ignore it and return 0xFFFFFFFC (not supported), or it may append zero bytes to the file until the size becomes that given. (VMs may internally optimize this to use sparse files.)

Returns zero if successful. In case of success, the file's size matches that given.

If the VM's maximum file size is less than the range of a 32-bit word (i.e. 4GB), the VM must return an error if the `size_high` argument is not zero.


```c
int stat(const char* path, unsigned output[4]);
```

Outputs information about the given path. If a file, directory or symlink exists at the given path, the following words are written to the output address in order:

- `type` -- 2 if the path is a symlink, 1 if the path is a directory, 0 if the path is a file
- `mode` -- Either 493 (0755) if the file is executable, 420 (0644) if it is not, or 0 if it is not a file.
- `size_low` -- The low 32 bits of the size of the file
- `size_high` -- The high 32 bits of the size of the file


```c
int rename(const char* from, const char* to);
```

Moves and/or renames a file or directory.


```c
int symlink(const char* from, const char* to);
```

Creates a symlink if supported.


```c
int unlink(const char* path);
```

Deletes the file or symlink at the given path. If this is used on a directory, an error is returned; `rmdir` must be used for directories.


```c
int chmod(const char* path, int mode);
```

Sets whether the file at the given path is executable in the host environment. Only two values are supported for mode:

- 493 (0755) -- The file is executable
- 420 (0644) -- The file is not executable

This is just used for better integration of wrapped binaries into the host system. It can be ignored.


```c
int mkdir(const char* path);
```

Creates an empty directory at the given path.


```c
int rmdir(const char* path);
```

Deletes an empty directory at the given path.


```c
int dopen(const char* path);
```

Opens a directory. Returns an I/O handle that is valid for directory syscalls only (i.e. `dread` and `dclose`.)


```c
int dclose(int directory_handle);
```

Closes the directory associated with the given handle.

This system call must return 0; a `dclose` call cannot fail. If the given file handle is invalid, the behaviour is undefined.


```c
int dread(int directory_handle, char buffer[256]);
```

Reads the next file entry from the given directory into the given buffer as a null-terminated string.

If there are no more entries, an empty string is placed in the buffer and 0 (success) is returned.



## Debug info

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

# Onramp Virtual Machine

The Onramp Virtual Machine is a simple virtual machine designed for portable bootstrapping.

This document specifies an incomplete in-development version of the virtual machine (tentatively version 4.0.) For a description of implementations, see [Onramp Virtual Machine Implementations](../platform/vm).



## Index

- [Overview](#overview)
- [Registers](#registers)
- [Memory Layout](#memory-layout)
- [Process Info Table](#process-info-table)
    - [Major Version][p-maj]
    - [Heap Start Address][p-heap]
    - [System Call Table Address][p-sys]
    - [Standard Input Handle][p-in]
    - [Standard Output Handle][p-out]
    - [Standard Error Handle][p-err]
    - [Command-Line Arguments][p-arg]
    - [Environment Variables][p-env]
    - [Working Directory][p-wd]
    - [Capabilities][p-cap]
    - [Minor Version][p-min]
    - [Additional Memory Regions][p-mem]
- [Position-Independence](#position-independence)
- [Instructions](#instructions)
- [Negative Values](#negative-values)
- [Opcode Table](#opcode-table)
- [Opcode Specifications](#opcode-specifications)
    - [Add][add]
    - [Subtract][sub]
    - [Multiply][mul]
    - [Divide Unsigned][divu]
    - [Bitwise And][and]
    - [Bitwise Or][or]
    - [Shift Left][shl]
    - [Shift Right Unsigned][shru]
    - [Load Word][ldw]
    - [Store Word][stw]
    - [Load Byte][ldb]
    - [Store Byte][stb]
    - [Immediate Short][ims]
    - [Less Than Unsigned][ltu]
    - [Jump If Zero][jz]
- [Files](#files)
    - [Handles](#handles)
    - [File Types](#file-types)
        - [Regular Files](#regular-files)
        - [Streams](#streams)
        - [Devices](#devices)
        - [Directories](#directories)
- [Calling Convention](#calling-convention)
    - [System Call Convention](#system-call-convention)
- [System Calls](#system-calls)
    - [System Call Table](#system-call-table)
    - [System Call Quick Reference](#system-call-quick-reference)
    - [Error Handling](#error-handling)
    - [`exit`](#exit)
    - [`panic`](#panic)
    - [`time`](#time)
    - [`open`](#open)
    - [`close`](#close)
    - [`read`](#read)
    - [`write`](#write)
    - [`seek`](#seek)
    - [`tell`](#tell)
    - [`trunc`](#trunc)
    - [`dirent`](#dirent)
    - [`stat`](#stat)
    - [`rename`](#rename)
    - [`delete`](#delete)
    - [`chmod`](#chmod)
    - [`mkdir`](#mkdir)
    - [`debug`](#debug)
    - [`alloc`](#alloc)
    - [`free`](#free)
- [Debug Info](#debug-info)
- [File Format](#file-format)
- [Rationale](#rationale)
- [Version History](#version-history)



## Overview

The Onramp VM is a register-based reduced instruction set computer (RISC). It uses 32-bit little-endian words addressed over 8-bit memory.

A large contiguous region of memory is available to the program which stores both code and data. The memory region is initialized with the program code, along with a process info table (PIT) containing information about the environment: command-line arguments, environment variables and so on. Programs typically divide the remainder of memory into a heap and a stack.

Programs are stored as a series of 32-bit bytecode instructions in a binary file format. Each instruction is four bytes: the first byte is the opcode and the three remaining bytes are the arguments. Opcodes, register arguments, and immediate values have distinct hexadecimal prefixes which makes it easy to read and write bytecode in hexadecimal.

There are 16 registers. Registers `r0` through `r9` are general purpose. Registers `ra` and `rb` are scratch registers reserved by the Onramp assembler. The last four registers are the stack pointer `rpp`, frame pointer `rfp`, program pointer `rpp` and instruction pointer `rip` respectively. There is no flags register. Carry and overflow must be detected manually, and conditional instructions use arbitrary registers as predicates.

There are 15 opcodes. All of them provide extremely basic functionality, such as adding two values into a register or storing a word at a memory address. High-level instructions such as pushing data onto the stack or calling a function do not have dedicated opcodes: they must be emulated with several low-level instructions. Emulation of high-level instructions is provided by the Onramp assembler using the scratch registers (`ra` and `rb`.)

There are (currently) no interrupts. The VM is designed for non-interactive computation. It does however optionally support input and time so it is possible to write interactive terminal applications.

All programs are position-independent. This makes it possible for programs to run other programs without the need for virtual memory. The program pointer (`rpp`) contains the base address of the currently running program.

The VM accesses the outside environment via system calls (or "syscalls".) These are provided by the VM to the program as a table of function pointers which the program calls using a special calling convention. System calls include input and output streams; file and directory access; time and more.

An Onramp VM can run hosted or freestanding.

- When hosted, the VM is running inside an operating system (OS) which provides a filesystem for storage. The VM bridges this filesystem into the Onramp environment by implementing file and directory system calls that use it. Onramp programs can therefore interoperate and share files with other non-Onramp programs running on the OS. See the [Files](#files) section below.

- When freestanding, the VM is running directly on hardware (or on virtualized hardware) with no underlying operating system. It has either no permanent storage, or it has a simple block storage device (i.e. it is accessed as a simple array of fixed-size sectors.) In this case the Onramp operating system runs inside the Onramp VM. The Onramp OS receives most system calls and implements its own filesystem backed by the optional block storage device.

The [VM Info Tool (`vminfo`)](../extra/vminfo/) can be used to inspect the features and capabilities of an Onramp VM. It is highly recommended to use it to test while developing a VM.

When writing C programs and compiling them for Onramp, you do not need to worry about any of this. It is handled by the Onramp libc.



## Registers

There are sixteen registers numbered `0x80` to `0x8F`. Their names are `r0`-`r9`, `ra`, `rb`, `rsp`, `rfp`, `rpp` and `rip`.

All instructions can operate on all registers, but some registers have special behaviour (such as the instruction pointer and stack pointer) and others have strong conventions on their use (such as the frame pointer and program pointer.)

- Registers `r0` through `r9` are general-purpose caller-preserved registers. They are used for local variables. Some of them have additional special purposes:

    - Registers `r0`, `r1`, `r2` and `r3` are used as the first four function and syscall arguments.

    - Register `r0` is often used as a return value. It is set to the process info table at the start of the program.

    - Register `r9` is used as a context parameter for syscalls.

- Registers `ra` and `rb` are "scratch space" registers. They are clobbered not only by function calls but also by compound assembly instructions. They can be used for temporary space when writing bytecode by hand but they are best avoided when writing or emitting assembly.

- Register `rsp` is the stack pointer. It points to the last value pushed on the stack. The Onramp VM stack grows down. The stack pointer must always be aligned to a 4-byte boundary and must always have 128 bytes free under it for interrupts and syscalls. There is no [red zone][wp-redzone]; it is an error to read or write to the stack area under the stack pointer. The stack pointer is set to the end of the initial free memory region when the program starts.

- Register `rfp` is the frame pointer. It points to the start of the current function's stack frame. This is the location where the previous frame pointer was pushed, forming a linked list of stack frames.

- Register `rpp` is the program pointer. It points to the base address where the program has been loaded into memory. Onramp VM programs are position-independent and must use `rpp` to calculate the addresses of program code and data.

- Register `rip` is the instruction pointer. It points to the next instruction to be executed. Upon reading an instruction, the VM increments the instruction pointer past that instruction before executing it.

Here it is in table form:

| Name      | Hex       | Description                                                 | Preserved by | Initial Value      |
|-----------|-----------|-------------------------------------------------------------|--------------|--------------------|
| `r0`      | `80`      | Local variables, function call argument, return value       | Caller       | Process Info Table |
| `r1`-`r3` | `81`-`83` | Local variables, function call arguments                    | Caller       |                    |
| `r4`-`r8` | `84`-`88` | Local variables                                             | Caller       |                    |
| `r9`      | `89`      | Local variables, syscall context                            | Caller       |                    |
| `ra`-`rb` | `8A`-`8B` | Scratch or compound assembly                                | Neither      |                    |
| `rsp`     | `8C`      | Stack pointer (last data pushed onto stack)                 | Callee       | End of free memory |
| `rfp`     | `8D`      | Frame pointer (start of stack frame)                        | Callee       |                    |
| `rpp`     | `8E`      | Program pointer (start of program)                          | Callee       | Start of program   |
| `rip`     | `8F`      | Instruction pointer (next instruction to be executed)       | Caller       | Start of program   |

There is no status or flags register. Detecting overflow, underflow and other such conditions must be done manually. Instructions such as [`ltu` (less than unsigned)][ltu] and [`jz` (jump if zero)][jz] can read or write the predicate in any register.

Note that although `rip` is initially set to the start of the program, it will move past the first instruction before that instruction actually executes. `rpp` should be used as the start of the program.



## Memory Layout

Onramp uses a flat memory architecture in which instructions and data are stored in the same address space. All addresses are 32 bits, so the maximum amount of addressible memory is 4 GiB.

The VM loads the initial program and maps it to an arbitrary address in memory. The initial value of registers `rpp` and `rip` are the address of the start of the program, where bytecode execution begins. The address of the program must be aligned to a multiple of 4 (and cannot be 0.)

The VM must also provide at least one free memory region to the program. Programs typically divide a free memory region into a [heap][wp-heap] and a [stack][wp-stack]. The heap starts at the beginning of the memory region; the stack starts at the end and grows down. The start of the free memory region is therefore called the [heap start address](#heap-start-address) in the process info table, and the initial value of the stack pointer `rsp` is the end of the free memory region. The initial contents of the free memory region (and of all additional and allocated memory regions) are undefined.

If the VM implements the [`alloc` syscall](#alloc), or if it provides [additional memory regions][p-mem], the free memory region can be as little as 256 bytes. The program will use the additional memory regions or request additional memory via `alloc`. If the VM provides neither, it must provide a large amount of memory (ideally as much memory as possible) up front in one contiguous block.

The total amount of memory required depends on the program being run. 4 MiB is sufficient for the initial bootstrap but many programs will require more. 16 MiB is a reasonable default. (This needs to be updated later with requirements for bootstrapping and for compiling and running various programs.)

(WARNING: The `alloc` syscall is not used yet. At least 4 MiB should be provided for the bootstrap.)

Information about the process and VM is made available to the program in data tables in memory. This includes the command-line arguments and environment variables of the process; the VM's capabilities and input/output handles; and a table of system calls. This information must be accessible at addresses outside of the program image and outside of any free memory region provided to the program.

Here's a diagram showing the regions of memory, the initial values of the registers, and some of the addresses in the process info table:

```
               read only                 read/write/execute           read/write/execute           read/write/execute
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   ~~~~~~~~~~~~~~~~~~~~   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   ~~~~~~~~~~~~~~~~
  |  process info     command-line  |   |                  |   |       free memory region       |   |  additional  |
  |    table,           args,       |   |  program image   |   |                                |   |    memory    |...
  |  syscall table    environ vars  |   |                  |   |        (heap)        : (stack) |   |    region    |
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   ~~~~~~~~~~~~~~~~~~~~   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   ~~~~~~~~~~~~~~~~
        ^-- r0            ^-- argv      ^-- rpp                ^-- heap_start             rsp --^
   ^-- syscall_table  ^-- environ       ^-- rip
```

Note that the regions do not have to appear in this order in memory. The process info can be before, after, or in between the program and free memory; the program can be before or after the free memory; there can be gaps between all regions; the process info can be spread out in multiple regions; and so on. The VM may place any memory region at any address, as long as that address is not zero and is a multiple of four.

The address 0 is special: it is an invalid address for all purposes. An address of zero is often called a null pointer, and it is sometimes used to represent no value when an address is optional. Loading or storing data to address 0 is undefined behaviour, as is passing it to most syscalls. The VM must therefore not place anything at address zero. (The VM does not need to detect accesses to address zero. Some VMs have memory mapped starting at address zero and simply skip the first word before writing the process info table.)

(In many VM implementations, the process info, the program, and the free memory are all placed in one contiguous region of memory. A single large block is allocated; the process info is placed at the start; the program is loaded afterwards; and the remainder of the block is the program's free memory. The end of the program is therefore the start of the heap. It was historically called the ["program break"][wp-sbrk] for this reason, and is still called that by some VMs. This contiguous layout is not required but it is often a convenient way to implement a VM.)

The "read only" and "read/write/execute" labels show how the memory is used, and suggests potential access limitations the VM may place on memory regions. These limitations refer to the Onramp bytecode; in particular "execute" means the VM must be able to execute Onramp bytecode in this region, not its own native code. Note that the free memory region must be executable so that the program can load and run subprograms.

The VM does not need to enforce any such access restrictions. There is no memory protection for the running program. If the program accesses memory outside of these ranges, or writes to a read-only memory region, or reads or writes address zero, the behaviour is undefined. (The VM may crash, the parent process may be corrupted, the VM may simply allow the write, etc.)

The start and end of all accessible regions of memory must be aligned to a four byte boundary. This includes the program image, the initial heap, all additional memory regions, and all memory regions allocated with the [`alloc`](#alloc) syscall. Another way of putting it is, if the program can read or write any byte in memory, it must be able to read or write the word that contains that byte. (This is necessary for some optimizations, i.e. the vectorization of string function like `memcpy()` and `strlen()`.)



## Process Info Table

The process information table (PIT) is an array of twelve 32-bit words (48 bytes total.) Here's a quick reference table of its contents:

| Index | Value                              | type     | Description                                                              |
|-------|------------------------------------|----------|--------------------------------------------------------------------------|
| 0     | [Major Version][p-maj]             | `int`    | Always 4 for this version.                                               |
| 1     | [Heap Start Address][p-heap]       | `void*`  | Address of the free memory region provided by the VM.                    |
| 2     | [System Call Table Address][p-sys] | `void*`  | Table of system calls.                                                   |
| 3     | [Standard Input Handle][p-in]      | `int`    | Handle of standard input stream                                          |
| 4     | [Standard Output Handle][p-out]    | `int`    | Handle of standard output stream                                         |
| 5     | [Standard Error Handle][p-err]     | `int`    | Handle of standard error stream                                          |
| 6     | [Command-Line Arguments][p-arg]    | `char**` | Null-terminated array of null-terminated strings.                        |
| 7     | [Environment Variables][p-env]     | `char**` | Null-terminated array of null-terminated strings of form "key=value".    |
| 8     | [Working Directory][p-wd]          | `char*`  | Directory in which the program is being run.                             |
| 9     | [Capabilities][p-cap]              | `int`    | Flags indicating the capabilities and environment of the VM.             |
| 10    | [Minor Version][p-min]             | `int`    | Always 0 for this version.                                               |
| 11    | [Additional Memory Regions][p-mem] | `void*`  | Null-terminated array of additional regions of available memory.         |

The parent process of a program (the VM or otherwise) must assemble this table somewhere in memory accessible to the program. It then places its address in `r0` before executing the program. The memory may be read-only; the program must not attempt to modify the table.

The process info table and its associated information (system call table, command-line arguments, environment variables, working directory) cannot be written to or executed by the program. An attempt by the program to write to or execute bytecode from these addresses is undefined behaviour. (VMs typically do not enforce this, but this restriction may simplify certain VMs, and it is important in order to simplify certain tools including the shell in the bootstrap process.)

All addresses that point to a word, or to an array of words, must be aligned to a multiple of four bytes. In the process info table, this includes the heap start address, the system call table address, the command-line arguments list and the environment variables. This also includes the initial `r0`, `rpp`, `rsp` and `rip` of the program. The addresses of strings, namely the working directory and the individual command-line arguments and environment variables, do not need to be aligned.



### Major Version

The major version field contains the major version of the Onramp VM.

The major version of the Onramp VM described by this document is 4. This field must contain the value 4 as a 32-bit little-endian word (`04 00 00 00`).

This number is incremented whenever breaking changes are made to this spec. Compatibility between different major versions is limited. The core bootstrap process and the libc aim to remain compatible with version 2 VMs and later. Programs (usually via the libc) check the VM version on startup and will refuse to run if it is too old or too new.



### Heap Start Address

The heap start address is the address of the first word of the initial free memory region provided by the VM. It must be a multiple of four and cannot be zero. See [Memory Layout](#memory-layout).



### System Call Table Address

The system call table address field contains the address of the [System Call Table](#system-call-table). The system call table is how the program interacts with its outside environment (reads input, accesses files, prints output and so on.)



### Standard Input Handle

The standard input handle is a handle to a stream that provides input to the program. It is used as an argument to [`read`](#read) for the program to get input data. See the description of [streams](#streams) and [handles](#handles).

For programs run interactively, this is typically connected to an input terminal (such as a keyboard). For programs run non-interactively (perhaps as part of a pipeline), the input of the program may be an arbitrary file or the output of another program.

Standard input is optional. If standard input is not supported by the VM, or if standard input is otherwise unavailable to the program, this field contains 0xFFFFFFFF. This is normal for example for a minimal VM or for non-interactive bootstrapping in a freestanding environment.

Valid handles for standard input, output and error may have any value as long as the high bit is not set. The program can use these handles to communicate with its environment.

The VM may "echo" its input; i.e. it may copy all input to its own output. In this case the "echo" bit must be set in the capabilities field of the process info table. It is recommended that VMs not echo if possible. The Onramp libc can simulate echo internally for interactive programs.

The VM may also "line buffer" its input, i.e. it may withhold input from the program until the user inputs a newline character. In this case the "line-oriented" bit must be set in the capabilities field of the process info table. It is recommended that VMs not line-buffer if possible.

(On POSIX systems, this means the input handle should ideally be in non-canonical mode and should have echo disabled. This is not easy to do correctly on POSIX, so the VM can instead simply leave it in canonical mode and set the appropriate capabilities bits.)



### Standard Output Handle

The standard output handle is a handle to which the program can write its output. It is used as a handle to [`write`](#write) for the program to write output data. See the description of [streams](#streams) and [handles](#handles).

Standard output is intended for normal program output, that could for instance be displayed to a user or consumed by another program.

For programs run interactively, the output is displayed to the user (for example in a graphical terminal or on a printer.) For programs run as part of a pipeline, the output of the program may be redirected to an arbitrary file or may be the input of another program.

Standard output is optional. If standard output is not supported by the VM, or if standard output is otherwise unavailable to the program, this field contains 0xFFFFFFFF.



### Standard Error Handle

The standard error handle is a handle to which the program can write error messages. It is used as a handle to [`write`](#write) for the program to write error messages. See the description of [streams](#streams) and [handles](#handles).

Standard error is intended for displaying errors, warnings and other abnormal output to a user. Data printed to standard error is therefore usually in plain text. It is separate from the output stream so that programs in a pipeline will display error messages to the user instead of feeding their errors to the next program.

The standard output and error handles are otherwise identical. If no distinction is required between them, the VM can use the same handle for both, and it can also use the same handle for input. Note however that if the same handle is used for multiple streams, closing it once must not close it for all of them. For example closing the standard output stream must not also close the standard error stream; if the handles are the same, the VM should either require calling [`close`](#close) twice to close it, or it should ignore `close` on the standard streams altogether.

Standard error is optional. If a standard error handle is not supported by the VM, this field contains 0xFFFFFFFF. If this has value 0xFFFFFFFF and the standard output handle is valid, programs usually write error messages to the output stream handle instead.



### Command-Line Arguments

Command-line arguments are stored in a null-terminated array of null-terminated strings. These are used to pass arguments to the program.

The first command-line argument is the name of or path to the program being run. If it's a path, the program can use it to find its program image, and it can use the last component of this path to determine its name.

Arguments beyond the first are used to control the behaviour of the program. For example, a program that parses a file may expect an argument providing the path to the file to be parsed.

This field is optional. If not supported, it can be null (zero) or it can be an empty array (i.e. it can be the address of a zero word.) This indicates to the program that the program name and command line arguments are not available. If the program name is not available but there are additional arguments, the first argument must be the address of a zero byte (i.e. it must be an empty string.)



### Environment Variables

Environment variables are stored in a null-terminated array of null-terminated strings.

Environment variables are key-value pairs delimited by `=`. The substring to the left of the first `=` is the key, while the substring to the right is the value.

This field is optional. If not supported, or if there are no environment variables, this field can be null (zero) or it can be an empty array (i.e. it can be the address of a zero word.)



### Working Directory

The working directory is the initial base directory that the program should use for relative paths.

Note that VM syscalls have no concept of a working directory. It is up to the program to track its own working directory and append relative paths to it before calling VM syscalls. (This is handled by the Onramp libc.) (TODO this is not done yet, currently the VMs all support paths relative to the initial working directory.)

If a working directory is provided, Onramp programs will normalize all paths to it when making system calls. In this case all paths passed to system calls will be absolute.

The working directory may be null (zero.) In this case the Onramp VM may pass relative or absolute paths to system calls.



### Capabilities

The capabilities entry is a 32-bit word containing a set of 1-bit flags. They are numbered from least to most significant bit:

| Bit Position | Value               |                                                            |
|--------------|---------------------|------------------------------------------------------------|
| 0            | Input Echo          | 1 if the VM echoes input to the output.                    |
| 1            | Input Blocks        | 1 if read(input) blocks until input is available.          |
| 2            | Input Line-Oriented | 1 if VM buffers input in lines (i.e. POSIX canonical)      |
| 3            | Interactive         | 1 if standard input comes directly from a user             |

The capabilities field contains a set of flags describing what features are supported by the VM. The following flags exist, with bits numbered from low to high:

- bit 0: input echo. 1 if the input stream is echoed to the output; 0 otherwise. If possible the VM should not echo input.

- bit 1: input blocks. 1 if the read syscall blocks until a byte is available; 0 if it doesn't, instead reading zero bytes successfully when no input data exists. If possible the VM should not block on input. If you are unsure whether the input blocks, set this to 1 to prevent programs from setting the input to non-blocking.

- bit 2: input line-oriented (i.e. [POSIX canonical][wp-canon]). 1 if input is only available once a full line has been processed; 0 if input is available immediately on each keystroke. If possible the VM should not line-buffer input. If you are unsure whether the input is line-oriented, set this to 1 to prevent programs from turning off canonical mode. (This is the default behaviour of terminal programs on POSIX platforms and it is surprisingly difficult to turn off so most POSIX VMs should set this bit.)

- bit 3: interactive. 1 if the standard input and output streams are connected directly to a user, for example to a keyboard and display; 0 if they are not or if unsure. An interactive program that expects input must wait until the user enters that input, for example by pressing keys. (The keys pressed by the user will then be displayed back to them; see "echo" above.) A non-interactive program may get its input from or send its output to another program, a file, etc.

(The libc will simulate echo if "input echo" is 0 and "interactive" is 1.)

(On POSIX systems, interactive input is echoed and is line-oriented by default, and these are not easy to change. It is simpler to just set those bits in the capabilities field, and to set interactive if and only if `isatty(0) && isatty(1)`.)



### Minor Version

The minor version field contains the minor version of the Onramp VM.

The minor version of the Onramp VM described by this document is 0. This field must contain the value 0 as a 32-bit little-endian word (`00 00 00 00`).

The minor version is incremented whenever non-breaking changes are made to this spec. Examples would be the addition of new system calls or new entries in the process info table. Programs compiled for older minor versions can simply ignore the new features, and programs compiled for newer minor versions will know by the minor version that these features are unavailable.

As long as the major versions are compatible, programs should be able to run regardless off any mismatch in minor version. Note however that when a program runs a subprogram, the minor version given to the child will be limited to that supported by the parent. This is because the parent needs to know of all system calls (among other things) it gives to the child in order to proxy them correctly.



### Additional Memory Regions

The program may read, write, and execute bytecode from several regions of the address space. There are always at least two such regions:

- The program, i.e. the area the size of the program image starting at the initial `rpp`; and
- The initial free memory region, i.e. the area between the heap start address in the PIT and the initial `rsp`.

The VM may provide additional memory regions through the Additional Memory Regions field of the PIT. If no additional memory regions are available, this field is zero.

If provided, the additional memory region field contains the address of a null-terminated array of memory regions. Each entry is two words: the address of a memory region and its size.

For example, suppose the Additional Memory Regions field of the PIT is `0x3000`, and there are two additional memory regions. In this case the memory at that address would look like this:

| Address   | Value                            |
|-----------|----------------------------------|
| 0x3000    | Address of first memory region   |
| 0x3004    | Size of first memory region      |
| 0x3008    | Address of second memory region  |
| 0x300C    | Size of second memory region     |
| 0x3010    | 0                                |

When an entry's address is zero, this indicates the end of the list. An additional memory region cannot start at address zero. Its size also cannot be zero. The null-terminating address zero does not need to have an associated size. The full table in this example is therefore 20 bytes.

The memory regions listed in this table must not overlap with each other or with the program or heap. They also must not include any of the memory in which the PIT and its contents are stored. They must be distinct memory regions which are otherwise unused and which the program may use for any purpose.

This can be useful if the VM or parent program has several disjoint regions of address space that are available to the program. This is common in freestanding VMs in which the memory regions represent areas of address space available in a machine's physical memory map. It is also useful for parent programs on VMs that implement `alloc` and not `free`.

(The program may also allocate memory regions at runtime via the [`alloc`](#alloc) syscall. Such regions are the only ones that may be passed to [`free`](#free). The program may not free its own image, its heap, or any of the additional memory regions passed through this field.)



## Position-Independence

All Onramp bytecode programs are position-independent. Programs can be loaded at any address in memory.

A special register, called the *program pointer* (`rpp`), contains the base address of the program in memory. References to symbols in the program (such as functions and global variables) are relative to the start of the program. Their absolute address can be determined by adding `rpp`.

Several instructions take two parameters that are added together, such as the add instruction and the load and store instructions. These can be interpreted as a base and an offset. When accessing program data, typically `rpp` is passed as the base, and the address of the data within the program (i.e. the value of a label) is passed as the offset.

For example, to load a 32-bit global variable into register `r0`, you would load the address of the label into a temporary register (e.g. `ra`) and then load this address relative to `rpp`. In primitive assembly, this is:

```asm
ims ra <some_variable   ; load the program-relative address of some_variable into ra
ims ra >some_variable   ; ...
ldw r0 rpp ra           ; load [rpp + ra] into r0
```

The compound assembler provides an `imw `instruction to load a label in one step:

```asm
imw r9 ^some_variable
ldw r0 rpp r9
```

Similarly, to call a function, you must load its label into a temporary register and then add it to `rpp` to get the absolute address. The jump is performed by storing the result directly in the instruction pointer. Typically you would also push the return address in between:

```asm
ims ra <some_function    ; load the program-relative address of some_function into ra
ims ra >some_function    ; ...
sub rsp rsp 4            ; make space on the stack for the return address
add rb rip 8             ; calculate the return address into rb     -----.
stw rb rsp 0             ; place the return address on the stack         |
add rip rpp ra           ; jump to rpp + ra (by assigning to rip)        |
add rsp rsp 4            ; pop the return address from the stack    <----`
```

The compound assembler has a `call` instruction which expands to the above:

```asm
call ^some_function
```

(This would of course be done after preparing the arguments; see the Calling Convention section below.)

There is only one instruction that takes an address relative to the instruction pointer: the conditional [jump if zero instruction (`jz`)][jz]. It takes a sign-extended 16-bit relative address, making it very useful for hand-writing loops in bytecode.



## Instructions

Onramp bytecode instructions are 32 bits in little-endian, or four bytes. All instructions must be aligned to a 32-bit boundary.

Each instruction opcode and each argument occupies one byte each, so each instruction has three arguments. Instructions are of the form:

```
[opcode] [argument-1] [argument-2] [argument-3]
```

For most instructions, argument 1 is an output register and arguments 2 and 3 are the inputs.

Opcodes, registers and single-byte immediate values have recognizable locations and hexadecimal prefixes:

- Opcodes start with `7`, and are always on a 32-bit boundary.
- Arguments are *never* on a 32-bit boundary:
    - Registers start with `8`.
    - Non-negative single-byte immediate values start with `0`-`7`.
    - Negative single-byte immediate values usually start with `9`-`F` and rarely start with `8`.

This makes it easy to read a hexdump of bytecode and to write it directly in [commented hexadecimal](hexadecimal.md).

Each instruction specifies the type of its arguments. Arguments can be one of several types:

- A "reg" (or `r`) argument is the name of a register. It must be a byte in the range `80`-`8F`.
- An "imm" (or `i`) argument is a literal byte. It can have any value.
- A "mix" (or `m`) argument is one byte that translates to a 32-bit value. How it is translated depends on its hexadecimal prefix:
    - If it is in the range `80`-`8F`, its value is the content of the named register.
    - If it is in the range `00`-`7F`, it is an immediate positive value with the high 24 bits clear.
    - If it is in the range `90`-`FF`, it is an immediate negative value; it is sign-extended, i.e. the high 24 bits are set.

Most instructions take mix-type arguments as input. This makes it easy to do math between registers and small immediate values without complicating the instruction set.

All instructions perform unsigned operations. (However, since any overflow is discarded, the result in most cases is the same as signed two's complement, so a VM can implement them with signed two's complement operations if that's all it has available. The exceptions have a `u` suffix to differentiate them from their signed `s` counterparts in compound assembly.)



## Negative Values

This specification does not prescribe any representation for signed numbers. There are no signed operations at the VM level, and a VM can be implemented using purely unsigned arithmetic with standard overflow behaviour. (A goal here is to maximize the kinds of alien architectures that can easily implement Onramp.)

The only thing the VM needs to be able to do in relation to negative numbers is sign extension. The VM must be able to copy the high bit of an 8-bit or 16-bit value to the upper bits of a 32-bit word. This must be done in two cases:

- When a mix-type byte is in the range `0x90`-`0xFF`, the upper 24 bits must be set to 1 to extend it to 32 bits. In order words, when a mix-type byte is not a register, the 8th bit must be copied to the upper 24 bits.

- The high bit of the two-byte offset in the [jump if zero instruction (`jz`)][jz] must be copied to the upper 16 bits to extend it to a full 32-bit word. It can then be added to the instruction pointer using an ordinary unsigned addition that wraps to 32 bits.

Programs compiled by the Onramp compiler and assembler use two's complement to represent signed numbers. All signed operations are reduced to unsigned VM instructions by the assembler.



## Opcode Table

Opcodes are divided into four groups: arithmetic, logic, memory and control.

Arguments have the following types:

- `r`: A register
- `m`: A mix-type byte, either a register or an immediate value in the range [-112,127]
- `i`: An immediate byte (any value)

Here's a quick reference table for all supported instruction opcodes:

Arithmetic:

| Opcode        | Name                    | Arguments                       | Operation                        |
|---------------|-------------------------|---------------------------------|----------------------------------|
| `0x70` `add`  | [Add][add]              | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 + arg2`             |
| `0x71` `sub`  | [Subtract][sub]         | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 - arg2`             |
| `0x72` `mul`  | [Multiply][mul]         | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 * arg2`             |
| `0x73` `div`  | [Divide Unsigned][divu] | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 / arg2` (unsigned)  |

Logic:

| Opcode         | Name                         | Arguments                       | Operation                                     |
|----------------|------------------------------|---------------------------------|-----------------------------------------------|
| `0x74` `and`   | [Bitwise And][and]           | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 & arg2`                          |
| `0x75` `or`    | [Bitwise Or][or]             | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 \| arg2`                         |
| `0x76` `shl`   | [Shift Left][shl]            | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 << arg2`                         |
| `0x77` `shru`  | [Shift Right Unsigned][shru] | `<r:dest> <m:arg1> <m:arg2>`    | `dest = arg1 >> arg2` (unsigned)              |

Memory:

| Opcode        | Name              | Arguments                       | Operation                                  |
|---------------|-------------------|---------------------------------|--------------------------------------------|
| `0x78` `ldw`  | [Load Word][ldw]  | `<r:dest> <m:base> <m:offset>`  | `dest = *(int*)(base + offset)`            |
| `0x79` `stw`  | [Store Word][stw] | `<m:src> <m:base> <m:offset>`   | `*(int*)(base + offset) = src`             |
| `0x7A` `ldb`  | [Load Byte][ldb]  | `<r:dest> <m:base> <m:offset>`  | `dest = *(char*)(base + offset)`           |
| `0x7B` `stb`  | [Store Byte][stb] | `<m:src> <m:base> <m:offset>`   | `*(char*)(base + offset) = src & 0xFF`     |

Control:

| Opcode       | Name                      | Arguments                       | Operation                                                |
|--------------|---------------------------|---------------------------------|----------------------------------------------------------|
| `0x7C` `ims` | [Immediate Short][ims]    | `<r:dest> <i:low> <i:high>`     | `dest = (dest << 16) \| (high << 8) \| low`              |
| `0x7D` `ltu` | [Less Than Unsigned][ltu] | `<r:dest> <m:arg1> <m:arg2>`    | `dest = (arg1 < arg2) ? 1 : 0` (unsigned)                |
| `0x7E` `jz`  | [Jump If Zero][jz]        | `<m:pred> <i:low> <i:high>`     | `if !pred: rip += 4 * signext16((high << 8) \| low)`     |

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

The behaviour is undefined if `arg2` is 32 or larger. (The VM can assume that `arg2` is always in the range of 0 to 31 inclusive; for example, it may ignore all but the low five bits. The VM may also crash or corrupt memory if `arg2` is 32 or larger. Debugging VMs halt and report an error if `arg` is 32 or larger.)



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

Since the source argument is mix-type, it is often used to store small constant values. For example, the program can directly store a zero to clear a byte in memory.



### Immediate Short

- opcode: `0x7C`
- assembly syntax: `ims <r:dest> <i:low> <i:high>`
- behaviour: `dest = (dest << 16) \| (high << 8) \| low`

Shifts the contents of the destination register up (left) 16 bits, then loads the low sixteen bits with the given arguments. The `low` argument is placed in bits 0-7 and the `high` argument is placed in bits 8-15. The 16 bits that are shifted out of the destination register are discarded.

The immediate short instruction is almost always used in pairs to load all 32 bits of a register. For example:

```asm
7C 80 34 12
7C 80 78 56
```

The above places 0x12345678 in the r0 register. A VM may optimize for this case, detecting when two `ims` operations are used together and loading all 32 bits of the register at once. (In particular, the arguments of the first `ims` instruction are often zero.) However, there are still rare cases where this instruction is used alone. The VM must implement the instruction correctly in this case.

The `ims` instruction is assumed to be fast due to this possible optimization. When compiling user code, if a register cannot be set to an immediate value in a single instruction, the Onramp toolchain prefers to output a pair of `ims` instructions.



### Less Than Unsigned

- opcode: `0x7D`
- assembly syntax: `ltu <r:dest> <m:arg1> <m:arg2>`
- behaviour: `dest = (arg1 < arg2) ? 1 : 0`

Sets the destination register to 1 if `arg1` is less than `arg2`. Sets the destination register to 0 otherwise.

The upper 31 bits of the destination register are always zero after this instruction. The low bit is set to the comparison result.

Note that a 32-bit two's complement comparison produces different results; you must be careful to perform an unsigned (logical) comparison. A signed comparison is simulated by the `lts` instruction (and other `s`-suffixed instructions) in compound assembly.



### Jump If Zero

- opcode: `0x7E`
- assembly syntax: `<m:pred> <i:low> <i:high>`
- behaviour: `if !pred: rip += 4 * signext16((high << 8) \| low)`

Jumps by the given sign-extended 16-bit number of words if the predicate is zero. If any bit in the predicate is set, this does nothing.

If the predicate is zero, the `high` argument is shifted up by 8 bits and added to the `low` argument to form an offset. Then, if the high bit of the high argument is set, the upper 16 bits of the argument are set as well; this is called sign extension. Finally, the complete offset is shifted up by two bits (i.e. multiplied by 4) and added to the instruction pointer, discarding the carry.

The `low` and `high` arguments together form a 16-bit two's complement signed offset. This makes it possible to jump backwards with this instruction. Note that the VM does not actually need to handle any two's complement operations; it is equivalent to an unsigned addition with discarded carry.

Note that, unlike most instructions, the first argument is not a destination. If the predicate is a register it is unchanged by this instruction.

The predicate is mix-type. A predicate of 0 can be used to perform an unconditional jump. If the predicate is a non-zero, non-register constant, the instruction does nothing; this can be used as a "no operation" instruction (see [File Format](#file-format) below.)



## Files

Files are the means by which an Onramp program can interact with its environment.

A _file_ is a named resource with which a program can interact. The collection of all files exposed to the program is called the _filesystem_.

Each file has a [_path_](#paths) which uniquely identifies the file.

The program interacts with files using system calls. Some system calls, like [`delete`](#delete), operate directly on files named by their paths. Other system calls operate through [handles](#handles); see below.



### Paths

A path is a string of bytes that uniquely identifies a file.

The maximum length of a path is 4095 bytes. Paths cannot contain a null byte.

VMs must support paths with at least the following ASCII characters:

- forward slash ("/")
- letters ("A" to "Z" and "a" to "z")
- numbers ("0" to "9")
- hyphen ("-")
- underscore ("\_")
- full stop (".")

Additionally, VMs are allowed to treat filenames as case insensitive: uppercase letters may be considered equivalent to their lowercase counterparts for the purpose of comparing paths.

(The Onramp source code uses only the above characters in its filenames. Programs compiled by Onramp may use any other bytes.)

The "/" (forward slash) character has special meaning in a path; see [Directories](#directories) below.



### Handles

A _handle_ is a 32-bit integer that represents an open file. Input and output of a file is done through a handle.

The program is given up to three handles in its process info table: standard input, standard output, and standard error. To get a handle to other files, the program must open the file using the [`open`](#open) system call. Most system calls take a handle as the first parameter; this handle indicates the open file on which the system call operates.

(Handles are called "file descriptors" on POSIX systems. The Onramp libc provides an implementation of POSIX file descriptors that wraps Onramp handles.)



### File Types

There are four types of files: regular files, streams, directories, and devices.


#### Regular Files

A _regular file_ is a file that stores data of arbitrary type and length and supports random access to its contents.

Regular files are the most common type of file. All of Onramp's data is stored in regular files, including source code, compiled programs, and documentation. Regular files typically persist indefinitely even when the machine storing them is powered off.

Regular files have a size. When a regular file is first created, it has size zero. Files grow automatically to accomodate data written to them. A file's size can also be changed with the `trunc` system call.

A handle referring to a regular file has a "position". When a regular file is opened, the handle has position zero. When the `read` or `write` system calls are called on such a handle, bytes are read or written starting at the handle's position, and the handle's position is incremented by the number of bytes read or written.

If `read` is called on a regular handle whose file position is the size of the file (i.e. it is at the end of the file), the VM returns `ERROR_END_OF_FILE`. If `write` is called on a regular handle whose position is the size of the file (i.e. it is at the end of the file), the file grows to accomodate the additional data.

(Note that position is a property of a handle, not a file. When a handle is closed, its position is discarded; if the same file is opened later, the new handle's position is zero. If a file is opened multiple times, each individual handle has its own separate position; reading or writing on one handle does not affect the position of other handles even if they refer to the same file.)

The size of a regular file and the position of a handle are represented by 64-bit values. Implementations may limit this arbitrarily; for example regular file sizes may be limited to 32 bits, in which case the upper 32 bits of file sizes and handle positions are always zero. Regular file sizes are otherwise limited only by available storage.


#### Streams

A _stream_ is a file that only supports reading, writing, or both. It is used to transfer data from one program to another.

Streams do not have a size. Handles referring to streams do not have a position and do not support seeking. `tell` and `seek` return an error when called on a stream handle.

A stream may support reading, writing, or both. If a stream only supports reading, `write` on the stream returns an error, and vice versa. A writable stream must be opened in read-write mode in order to write to it.

There are three standard streams: input, output and error. These are intended for programs to interact with other programs and with a user. The standard input, standard output, and standard error handles, if they exist, are given to the program in the process info table.

Other streams may exist on the filesystem depending on the VM implementation. These are VM-specific and are not specified here.


#### Directories

A _directory_ is a container for files (including other directories.) Directories are used to organize and enumerate files on the filesystem.

Support for directories is optional. A VM may ignore directories; it can simply store all files in a flat set in which each file is named by its full path. However, without directories, programs will have no way of enumerating files. Onramp can be bootstrapped without directories, but some programs that can be compiled by Onramp require directory support. Such programs will not work on VMs that do not implement directories.

In a path, the "/" (forward slash) character delimits _path components_. A path component is never empty, so paths never have consecutive forward slashes (i.e. "//" never appears in a path.) Path components have a length of 1 to 255 bytes. The final component of a path is called the _file name_. All other components are directories. This creates a hierarchical organization of files in the filesystem.

Paths typically start with "/" (a forward slash) and do not end in "/". The initial forward slash is called the _root directory_, in which all other files are ultimately nested. Path components never contain a forward slash, with the exception of the root directory whose name and path is "/".

For example, consider the file with path "/dev/urandom". The three components of this path are "/", "dev", and "urandom". The directory "/" contains the directory "dev" which contains the device "urandom".

The filesystem must have a _root_ directory. The path of this directory is `/`. All directories have a parent directory except the root. A directory cannot be a child of itself, even indirectly; following the parents of a directory always leads to the root. In other words, the filesystem forms a directed acyclic graph in which the inner nodes are non-empty directories and the leaves may be any file type.

The filesystem implemented by a VM resembles that of a POSIX system as described above. If your host filesystem has a different shape but you still want it to interoperate with Onramp, the VM must translate paths to make them appropriate for Onramp. For example, on a system where `\` delimits directories, a path like `C:\Foo\Bar`  must be translated it to something like `/c/Foo/Bar` to be used with Onramp.

(In the Onramp filesystem, the special directory name "." is a reference to the current directory, and ".." is a reference to the parent directory. A VM does not need to support these. The Onramp libc generates these where needed and resolves them before passing paths to system calls.)

The [`mkdir`](#mkdir) system call is used to create a directory. The [`dirent`](#dirent) system call is used to enumerate one. Since directories are files, several other system calls can manipulate them as well, including [`open`](#open), [`delete`](#delete), [`stat`](#stat), [`rename`](#rename) and more. Exceptions include [`read`](#read) and [`write`](#write) which cannot be used on directories.


#### Devices

A _device_ is a file with a specific name and special behaviour.

Support for devices is optional. The VM does not need to implement this and can ignore this section.

The VM may interpret certain filenames as "devices". A program opens a device with `open` and closes it with `close` like any other file. It interacts with the device with other handle syscalls (e.g. `read`, `write`.) Devices may or may not support any of the system calls that operate on handles; whether such system calls are supported depends on the device and is specified below.

Only one device is defined so far:

- `/dev/urandom`: A device that provides high quality cryptographically secure random numbers. A call to `read` on this device provides random bytes; no other system calls are supported. The device must be opened read-only.

(The VM may or may not implement `/dev/` as a directory.)



### Blocking and Non-Blocking

When reading from a handle, data may not be available yet, but might become available later. For example, if a program's input stream is connected to a keyboard, there will be no data available to read until the user presses keys.

The VM's behaviour on reading from such a handle can be characterized as "blocking" or "non-blocking".

If the program calls `read` and data is not currently available, the VM may handle it in one of two ways. If the handle is *blocking*, the VM suspends execution of the program until data becomes available, then completes the system call. If the handle is *non-blocking*, the VM returns `ERROR_TRY_LATER` without waiting for data to become available.

File writing may also be blocking or non-blocking. For example a file may have a fixed size buffer which takes time to flush; if the buffer is full, the VM will not be able to write until space becomes available. In this case a call to `write` either blocks until data can be written or returns `ERROR_TRY_LATER`.

It is recommended that VMs provide non-blocking reading and writing for streams. Programs generally assume that regular files will block (since they always do on contemporary operating systems) but they are allowed to be non-blocking as well.

(On POSIX systems, this means the VM should always poll a file descriptor to determine whether data is available before reading from it.)



## Calling Convention

Bytecode can use any mechanism for performing function calls, but there is a standard calling convention used by the Onramp C compiler and by most of the hand-written assembly and bytecode programs. This section describes the standard calling convention.

Arguments that are larger than a register (32 bits) are always passed on the stack, never in multiple registers. The first four non-struct register-sized or smaller arguments are passed in order in registers r0-r3 (even if they appear after larger arguments.) All other arguments are pushed on the stack in reverse order with their size rounded up to the nearest word.

If a function's return type is a struct or is larger than a register, the caller must provide storage for the return value. Its address is pushed on the stack after all arguments (it is never passed in a register.) Finally, the return address is pushed last onto the stack before jumping into the callee.

The typical instruction sequence for the caller is:

- Push whatever registers you want to preserve to the stack;
- Push non-register arguments to the stack right-to-left;
- Place the first four word-size arguments left-to-right in `r0`-`r3`;
- Load the program-relative address of the symbol into `ra`
- Push a return address to the stack
- Jump to `ra + rpp`
- Move the return value from `r0` if necessary
- Restore registers from the stack
- Pop everything else from the stack

The callee typically begins by setting up a stack frame. The previous frame pointer (rfp) is pushed onto the stack, then the current stack pointer (rsp) is stored as the function's own frame pointer. The frame pointers therefore form a linked list of stack frames. The area above each frame pointer contains the stack-passed arguments and the area below it contains local variables. (Stack frame setup is done with the `enter` and `leave` assembly instructions; see the [assembly specification](assembly.md) for details.)

Registers r0-r9 (and ra-rb) are available for use within the function; the callee does not need to preserve them. The callee is also allowed to modify its non-register arguments on the stack, but it does not pop its arguments or return address. When the function returns, the stack pointer must have the same value as when it started (as do the frame pointer and program pointer.)

If the return type is not a struct and fits in a register, the return value is passed in register r0; otherwise, the return value is stored at the return address that was pushed to the stack by the caller.

This calling convention is designed to be simple at all stages of Onramp while still maintaining reasonable efficiency. Register-passing is by far the easiest mechanism for handwritten assembly and bytecode. The first stage C compiler only supports up to four arguments and only supports primitive types so it always passes all arguments in registers. The second stage C compiler adds structs but it cannot pass them by value, so it only needs to pass arguments 5 and later on the stack. The final stage C compiler supports passing and returning structs and 64-bit values as described above.

Handwritten bytecode and assembly functions that violate this convention describe the differences in their code comments. Such functions cannot be called from C.



### System Call Convention

The system call convention is used when making system calls. See [System Calls](#system-calls) below.

From the perspective of the caller, a system call is the same as a regular function call except that a special "context" value is passed in register r9. The meaning of the context parameter is irrelevant to the caller; it is up to the implementer of the system call to decide what it means.

The Onramp C compiler has no support for the system call convention. Instead, syscall wrapper functions are implemented in assembly in the Onramp libc. These wrapper functions can be called from C.



## System Calls

A system call (or "syscall") is the mechanism by which a program accesses and modifies the outside environment.

A program makes system calls to perform input and output; to access files and directories; to get the current time; and more.

The VM provides the program with a table of system calls at startup. The program consults this table to determine what system calls are available, and then performs these calls to modify its environment.



### System Call Table

The process info table contains a pointer to the system call table. The system call table is an array of function pointers, each of which has an additional context argument to be passed in r9. See the [system call convention](#system-call-convention) above.

The length of the array is the number of syscalls. Each entry in the array is two words. The first word is the absolute address of the function (i.e. the address to put in `rip`) and the second word is the context argument (i.e. the value to put in `r9`.)

For example, if the syscall table is at `0x2000`, the memory at that address would look like this:

| Address   | Value             |
|-----------|-------------------|
| 0x2000    | `rip` of `exit`   |
| 0x2004    | `r9` of `exit`    |
| 0x2008    | `rip` of `panic`  |
| 0x200C    | `r9` of `panic`   |
| 0x2010    | `rip` of `time`   |
| 0x2014    | `r9` of `time`    |
| 0x2018    | `rip` of `open`   |
| 0x201C    | `r9` of `open`    |
| ...       | ...               |

If a syscall is unimplemented, the function address is `0`.

To make a syscall, the program performs a function call with the syscall call convention. In other words, it preserves its registers; places the syscall arguments in r0-r3 and the context in r9; pushes a return address; and puts the syscall's function address into `rip`. Upon returning, the program pops the stack and restores its registers.

The meaning of the `r9` context is decided by the implementer of the syscall. There are several ways a VM can implement syscalls:

- The `rip` of each syscall can be some sentinel value (e.g. `0xAAAAAAAA`), and `r9` can contain the syscall number. The VM detects when `rip` is the sentinel value and performs the syscall in `r9`. (This technique is used by the [c-debugger](../platform/vm/c-debugger/) VM.)

- The `rip` of each syscall can be the syscall number plus some offset (e.g. `0x80000000`), and `r9` can be ignored. The VM detects when `rip` is above this value and subtracts the offset to recover the syscall number.

- The `rip` of each syscall can be an address that contains a custom opcode (typically `0x7F`), and `r9` can contain the syscall number. When the program attempts to execute the custom opcode, the VM performs the syscall in `r9`. (This is typically the fastest way for a VM to implement syscalls since it does not require checking `rip` for validity. This technique is used by most VMs include the [C89](../platform/vm/c89/), [Python](../platform/vm/python/) and [POSIX Shell](../platform/vm/sh/) VMs.)

When syscalls are implemented by a parent program, `rip` is the address of a function to handle (or proxy) the syscall, and `r9` is typically the address of a struct or stack frame containing context about the parent and child. The parent will recover its `rpp` from the context, perform the syscall, then restore `rpp` afterwards.

In any case, after the syscall is performed, the implementer of the syscall must place a return value in r0 and then return control to the address at the top of the stack. In other words, a VM returns control to the program by loading the address pointed to by `rsp` into `rip`. For a syscall implemented in a parent program, this is the ordinary mechanism by which a function returns to the caller. (The only exception is `exit` and `panic` which do not return control to the program.)

When a program is nested deep within other programs in a VM, the system call table will contain a mix of function pointers from various parents. For example, consider the [Onramp assembler](../core/as/), running in the [Onramp driver](../core/cc), running in the [Onramp shell](../core/sh/), running in the [Onramp OS](../core/os/), running on a freestanding Onramp VM. Typically the direct parent (in this case the driver) will implement `exit`; the VM's `exit` will be provided as `panic`; and the OS will implement all of the file and directory syscalls. The driver and shell also proxy `open` and `close` (among others) to clean up their children's leaks.



### System Call Quick Reference

All system calls (except for `exit` and `panic`) return a 32-bit word. It contains either an error code, a return value, or 0 to indicate success without a value. If a return value is omitted in the below table, the system call returns 0 on success and an error code on error.

Arguments are passed in `r0`, `r1`, `r2` and `r3`, plus the context in `r9`. The return value is placed in `r0`.

The system call table currently has **25** entries (200 bytes) with a total of 20 defined system calls. Entries not listed below are unused and reserved for future versions of this specification.

| Number | Required  | Name     | Arguments                | Return Value             |  Description                             |
|--------|-----------|----------|--------------------------|--------------------------|------------------------------------------|
| 0      | hosted    | exit     | exit code                | n/a (doesn't return)     | exits the program                        |
| 1      |           | panic    | exit code                | n/a (doesn't return)     | halts the VM                             |
| 2      |           | time     | out\_time[3]             |                          | gets the current time                    |
| 3      | hosted    | open     | path, writable           | handle                   | opens a file                             |
| 4      | hosted    | close    | handle                   |                          | closes a file                            |
| 5      | hosted    | read     | handle, buffer, size     | bytes read               | reads from a file or stream              |
| 6      | hosted    | write    | handle, buffer, size     | bytes written            | writes to a file or stream               |
| 7      | hosted    | seek     | handle, base, pos (x2)   |                          | seeks to a position in a file            |
| 8      | hosted    | tell     | handle, out\_pos[2]      |                          | gets the current position in a file      |
| 9      |           | trunc    | handle, size (x2)        |                          | truncates a file                         |
| 12     |           | dirent   | handle, buffer           |                          | reads one file entry from a directory    |
| 13     |           | stat     | path, out\_size[2]       | file type                | gets the type and size of a file         |
| 14     |           | rename   | path, path               |                          | renames a file                           |
| 16     | hosted    | delete   | path                     |                          | deletes a file                           |
| 17     |           | chmod    | path, mode               |                          | changes permissions of a file            |
| 18     |           | mkdir    | path                     |                          | creates a directory                      |
| 22     |           | debug    | address, path            |                          | loads debug info for a child program     |
| 23     |           | alloc    | size, out_address        | actual size              | allocates a block of memory              |
| 24     |           | free     | address, size            |                          | frees an allocated block of memory       |

In the required column, system calls marked "hosted" must be implemented in a hosted environment. All other system calls are optional.

In a freestanding environment, all system calls are optional, although it is highly recommended to support `write` for the standard output and error streams in order to receive debug output from the program.

A description of each system call with a C-style prototype follows. (The C prototypes described below are declared by the libc in `#include <__onramp/__syscalls.h>`. They can be called as ordinary C functions, although such use is discouraged outside of the libc.)



### Error Handling

The system calls `exit` and `panic` do not return control to the program. There is no possibility of error handling for them.

All other system calls return a 32-bit result to the program in `r0`. When we say that a system call "returns" a value, we mean that it places this value in register `r0` before resuming execution of the program.

- In case of success, the value in r0 is a 31-bit result: the high bit of `r0` is always clear on a successful system call. Some system calls return a meaningful value such as a file or directory handle. For those system calls that do not, the return value on success is 0.

- In case of failure, the VM must return an error code. All error codes have the high bit set. Programs test for system call errors by checking if the high bit of `r0` is set.

Error codes are numbered in descending order starting from `0xFFFFFFFF`, the largest possible 32-bit value. (These correspond to negative integers descending from -1 in two's complement. These are given in the below table for reference only; a VM does not need to implement negative numbers.)

Here's a quick reference table of possible error codes:

| Optional  | Name                   | Hex Value     | Decimal Value    | Two's Complement Value  |
|-----------|------------------------|---------------|------------------|-------------------------|
|           | `ERROR_GENERIC`        | `0xFFFFFFFF`  | 4294967295       | -1                      |
| \*        | `ERROR_NO_SUCH_PATH`   | `0xFFFFFFFE`  | 4294967294       | -2                      |
| \*        | `ERROR_IO`             | `0xFFFFFFFD`  | 4294967293       | -3                      |
| \*        | `ERROR_UNSUPPORTED`    | `0xFFFFFFFC`  | 4294967292       | -4                      |
|           | `ERROR_TRY_LATER`      | `0xFFFFFFFB`  | 4294967291       | -5                      |
|           | `ERROR_END_OF_FILE`    | `0xFFFFFFFA`  | 4294967290       | -6                      |
| \*        | `ERROR_OVERFLOW`       | `0xFFFFFFF9`  | 4294967289       | -7                      |
| \*        | `ERROR_IN_USE`         | `0xFFFFFFF8`  | 4294967288       | -8                      |

For error codes marked optional (\*) the VM can return `ERROR_GENERIC` instead. The error codes not marked optional are required to be implemented properly.

The two required error codes are `ERROR_GENERIC` and `ERROR_END_OF_FILE`. If non-blocking I/O is available, `ERROR_TRY_LATER` is also required. Other error codes are optional, but can provide more precise error handling and error reporting to users. (A VM can be greatly simplified by returning `ERROR_GENERIC` for almost all exceptional conditions, though its error reporting will be limited, and programs will have to make additional syscalls to try to determine the reasons for errors.)

The error codes are specified as follows:

- `ERROR_GENERIC` (`0xFFFFFFFF`): An unspecified error condition. It can be used for exceptional conditions where no other error codes are appropriate. It can also be used in nearly all cases instead of a more specific error code in order to simplify the VM. This must not be used to indicate the end of a file or an empty or full stream; see `ERROR_END_OF_FILE` and `ERROR_TRY_LATER`.

- `ERROR_NO_SUCH_PATH` (`0xFFFFFFFE`): The given file or directory does not exist. It is returned by system calls that take a path that may be expected to exist, such as `open`, `stat`, `delete`, and so on.

- `ERROR_IO` (`0xFFFFFFFD`): An input/output error occurred. This is used to indicate an unspecified and typically unrecoverable failure to transfer data. For example if the storage device malfunctions, the VM can return this code. This must not be used to indicate the end of a file or an empty or full stream; see `ERROR_END_OF_FILE` and `ERROR_TRY_LATER`.

- `ERROR_UNSUPPORTED` (`0xFFFFFFFC`): The request is not supported. Some system calls may be partially implemented by the VM, and certain behaviours may not be permitted by this specification or by the host environment. If a given combination of parameters is not supported, the VM may return this error code. For example, the `delete` system call may return this to indicate that the the given file cannot be deleted. (If no part of a system call is supported, it is better to not implement it at all, and instead put zero in the corresponding `rip` field in the system call table.)

- `ERROR_TRY_LATER` (`0xFFFFFFFB`): Reading or writing is currently not possible but may become possible later. If non-blocking files are supported, this must be returned to indicate that the program should wait and try again. For `read`, the program should wait for more data to become available, and for `write`, the program should wait for output space to become available before trying again.

- `ERROR_END_OF_FILE` (`0xFFFFFFFA`): The end of the file or stream has been reached. This must be returned from `read` on a file when the file's position is the end of the file. It may also be returned from `read` or `write` on streams when the other end of the stream (the sender of input or the recipient of output) has closed it.

- `ERROR_OVERFLOW` (`0xFFFFFFF9`): Not enough resources are available to satisfy the request. For example, this can be returned from `alloc` when no more memory is available, or from `open` if there are too many open files.

- `ERROR_IN_USE` (`0xFFFFFFF8`): The requested resource is already in use. For example, this can be returned from `open` on a file that is already open, or from `mkdir` on a path that already exists.

All error codes have the high bit set. Values that can be returned from successful system calls (such as file handles) do not have the high bit set. Programs check for errors by testing whether the high bit is set.

When an error occurs on an existing file handle, the handle always remains open. A handle can only be closed by a call to `close` or by exiting the program.

(Some system calls indicate that certain combinations of arguments are undefined behaviour. In such cases, the VM does not need to return an error code; in fact it does not need to check for such incorrect usage at all. It may check however, and if detected, it is reasonable to return an error code (such as `ERROR_UNSUPPORTED`) or to halt the program and report the bug to the user.)



### `exit`

```c
[[noreturn]] void __sys_exit(int exit_code);
```

- syscall number: 0
- argument in r0: exit code
- return value: n/a (does not return)

Exits the program with the given exit code.

This system call does not return.

Onramp programs use an exit code of 0 for success and any other value as failure.

Most platforms restrict the exit code to a maximum of 7 or 8 bits. Such platforms may ignore the high bits, so for example a return value of 256 may incorrectly be treated as success. Moreover, an exit code of 125 is used by some Onramp VMs to indicate an illegal operation by the program, and higher values have platform-specific meanings as well (for example some shells use 127 and 126 to indicate a failure to run a command.) Programs are recommended to use small values in the range of 1-124 (typically 1) to indicate errors.

This system call is optional in freestanding. It must be implemented in a hosted environment. Freestanding VMs typically only implement this if they are able to halt the machine.



### `panic`

```c
[[noreturn]] int __sys_panic(int exit_code);
```

- syscall number: 1
- argument in r0: non-zero exit code
- return value: n/a (does not return)

Halts the VM, exiting with the given non-zero error code.

This kills all programs running in the instance of the VM. It should only be used in case an error occurs that is unrecoverable by normal program exit. For example, this is called when memory corruption is detected by `malloc()`, because the memory of other processes in the VM may be corrupted as well.

The given exit code must be non-zero. (The VM is allowed to ignore this and does not need to check.) See the notes on exit codes in [exit](#exit) above.

This system call is optional and is not typically implemented by Onramp VMs. If not implemented, programs will typically use the `exit` syscall instead. (This system call mainly exists so that programs can pass along the VM's exit to child programs so they can take down the whole VM if a critical error occurs.)



### `time`

```c
int __sys_time(unsigned time[3]);
```

- syscall number: 2
- argument in r0: address at which to write the time
- return value in r0: always 0

Gets the current time, writing three words to the address given in r0.

The current time consists of a 64-bit number of seconds plus a 32-bit number of nanoseconds since the UNIX epoch (the start of January 1st, 1970.) These are written as three words to the given address:

- r0 + 0: The low 32 bits of the number of seconds
- r0 + 4: The high 32 bits of the number of seconds
- r0 + 8: The number of nanoseconds (0 to 999,999,999)

If this system call is implemented, it cannot fail. It must always set register r0 to 0.



### `open`

```c
int __sys_open(const char* path, bool writable);
```

- syscall number: 3
- argument in r0: address of a null-terminated string containing the path to open
- argument in r1: whether the file should be opened for writing
- return value in r0: handle or error code

Opens the file at the given path, associating it with an integer handle and returning it. The stream position is initially at the start of the file.

The returned integer must not have been in use by another handle. (The returned integer may have been returned by previous calls to `open` if and only if each time was eventually followed by a corresponding `close`.) Note that the handles for the standard input, output and error streams are in use (if provided) at the start of the program.

The `writable` argument (in r1) must be 0 or 1. If it is 1, the file will support writing (via `write` and `trunc`), and will be created if it does not already exist. Directories may not be opened for writing. The [Devices](#devices) section specifies which devices may or must be opened for writing.

If a file being opened for writing already exists, the contents are left intact. Since the initial position is at the start of the file, subsequent writes will overwrite the contents. To append to an existing file, the program must make a [`seek`](#seek) call after opening it. To destroy the existing contents after opening, the program must make a [`trunc`](#trunc) call.

The VM may limit a file from being opened simultaneously by multiple handles. It may allow only a single handle to a file; or it may allow multiple read-only handles but zero or one writable handles; or it may allow any number of read and write handles simultaneously to the same file. (It is recommended that the VM permit opening multiple handles to the same file if possible.)

In case the VM supports multiple handles to the same file, each handle must have a different integer value. Each handle also has an independent stream position, but operations on one handle must reflected immediately in the other handles. For example, a `write` to one handle will be immediately visible by a `read` to another handle to the same file. Similarly, a shortening `trunc` on one handle must immediately limit the stream position of other handles to the new bounds of the file.

On success, a new handle is returned, which must not have the high bit set. (The returned value cannot be an existing handle; it must be an integer value that is not currently in use.)

If a limit has been reached on the number of open files, this returns `ERROR_OVERFLOW` or `ERROR_GENERIC`.

If the file does not exist and the writable flag is not set, this returns `ERROR_NO_SUCH_PATH` or `ERROR_GENERIC`.

If the file does not exist, the writable flag is set, and the file cannot be created (perhaps due to a permission issue), this returns `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.

If the file is a directory and writable is set, this returns `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.

If the file is a directory and the VM does not support directories, this returns `ERROR_UNSUPPORTED` or `ERROR_NO_SUCH_PATH` or `ERROR_GENERIC`.

If the file cannot be opened due to a failure of the storage device or other data corruption, this returns `ERROR_IO` or `ERROR_GENERIC`.

If the file is already open by another handle and the VM does not support opening an additional handle to it concurrently in the given mode, this returns `ERROR_IN_USE` or `ERROR_GENERIC`.

If the file cannot be opened for other reasons (perhaps due to a permission issue), this returns `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.



### `close`

```c
int __sys_close(int handle);
```

- syscall number: 4
- argument in r0: the handle of the file to close
- return value in r0: always 0

Closes the given handle or input/output/error stream.

Once called, the given handle can no longer be used by the program (though its integer value may be re-used by the VM as the return value of a subsequent call to `open`.)

A request to close the input, output and error streams may not be meaningful on the underlying platform. In this case the VM may ignore it and return 0.

A request to close a valid handle cannot fail. It must always return 0.

If the given handle is invalid, the behaviour is undefined. (The VM may return an error code in this case, but the program is already misbehaving and it is likely to ignore it; if such an error is detected it is generally better to abort the program.)



### `read`

```c
int __sys_read(int handle, void* buffer, int count);
```

- syscall number: 5
- argument in r0: the handle of the file or input stream from which to read
- argument in r1: address at which to store the read data
- argument in r2: the maximum number of bytes to read into the address at r1
- return value in r0: the non-zero number of bytes read or an error code

Reads up to `count` bytes into the given buffer, returning the number of bytes actually read or an error code if reading fails.

The **read** syscall is used to read from files and from the input stream. When called on the input stream, it is intended to read interactive input (from a user) or the output of another program being streamed (or piped) to it.

The `count` argument in r2 must be non-zero. The VM is allowed to assume it is never zero. (For example, the VM may ignore the count and always read exactly one byte. This would be inefficient but would nonetheless be a correct implementation.)

The maximum number of bytes that can be read is 2147483647 (2^31-1) bytes, as larger sizes would have the high bit set which would be interpreted as an error. The VM reads at most this number or `count`, whichever is smaller.

If bytes are available, the VM must read at least one byte, but may read less than the number of bytes requested. In this case it returns a non-zero number of bytes read.

If no bytes are available, the VM should return an error code. The error code to use depends on the reason for the failure.

If the end of the file has been reached or the input stream is closed, the VM must return `ERROR_END_OF_FILE`.

If no bytes are currently available from an input stream but may become available later, the VM must return `ERROR_TRY_LATER` to tell the program to wait for input (and possibly do something else in the meantime) before trying again. This could be returned while waiting for the user to type keystrokes for example.

(For the standard input handle, this is called non-blocking input. If non-blocking input is not possible on the VM's platform, the VM may instead block on this system call until data is available; in this case it must set bit 2 in the capabilities field of the process info table. Unlike most platforms, Onramp allows non-blocking reads on *all* handles, even ordinary files on disk.)

If some other error occurred in reading data, for example the storage device malfunctioned or file data is corrupted, the VM must return `ERROR_IO` or `ERROR_GENERIC`.

This can only be called on the standard input stream or a file opened with `open`. If this is called on the standard output or standard error streams, the behaviour is undefined.

If the program attempts to read from a special file for which only writing is supported by the platform, the VM may return `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.

If the error is not otherwise specified, the VM may return `ERROR_GENERIC`.

Since platforms implement input differently, Onramp supports considerable variation in the implementation of read. The behaviour of a VM's read syscall must be accurately represented by the capabilities bits in the process info table as explained below.

Assuming the capabilities are accurately reported by the VM, the Onramp libc will simulate whatever behaviour is desired by the program where possible. For example, if the VM has non-blocking input and the program requests blocking input, the libc will perform blocking. However, if the VM is blocking and the program requests non-blocking input, the behaviour cannot be simulated so the libc will reject the request. If you are implementing a VM, follow the recommendations below to get maximum compatibility with programs running on Onramp.

User input from a terminal should be in UTF-8 format and it may use [ANSI escape sequences][wp-ansiesc] for special characters (such as arrow keys.) (This needs to be moved to its own section and greatly expanded.)

When the user enters input, it should not be echoed to the output by the VM. If this is not possible on the VM's platform, the VM may instead echo input to the output; in this case it must set bit 0 in the capabilities field of the process info table.

The VM should make input keystrokes available immediately rather than waiting until the end of a line. If this is not possible on the VM's platform, the VM may instead wait until a full line has been processed before making it available to the read syscall; in this case it must set bit 1 in the capabilities field of the process info table. (This is the default behaviour of terminal programs on POSIX platforms.)

The VM must never return zero from this system call. (Earlier versions of this spec recommended returning zero under various circumstances. For backwards compatibility reasons, the Onramp libc and some bootstrap programs contain code that attempts to interpret the meaning of a zero return on `read`. This behaviour must not be relied upon.)



### `write`

```c
int __sys_write(int handle, void* buffer, int count);
```

- syscall number: 6
- argument in r0: the handle of the file or output/error stream in which to write
- argument in r1: address containing the data to write
- argument in r2: the maximum number of bytes to write from the address at r1
- return value in r0: the number of bytes written or an error code

Writes up to `count` bytes from the given buffer into the given file or stream, returning the number of bytes actually written or an error code if writing fails.

The `count` argument in r2 must be non-zero. The VM is allowed to assume it is never zero. (For example, the VM may ignore the count and always write exactly one byte. This would be inefficient but would nonetheless be a correct implementation.)

If space is available to write bytes, the VM must write at least one byte, but may write less than the number of bytes requested.

The maximum number of bytes that can be writtten is 2147483647 (2^31-1) bytes, as larger sizes would have the high bit set which would be interpreted as an error. The VM writes at most this number or `count`, whichever is smaller.

If the output is a stream which has been closed by the other end, this returns `ERROR_END_OF_FILE`.

If the output does not have enough space to store more data but space may become available later (for example when the other end of the stream consumes some of the written data, or if data is flushed to storage asynchronously), this returns `ERROR_TRY_LATER`. The program will typically wait and try to write the same bytes again.

If the output handle is full and more space cannot be made available (for example the storage device is full or the file has reached the maximum supported size), the VM returns `ERROR_IO` or `ERROR_GENERIC` (and *not* `ERROR_END_OF_FILE`.)

If some other error occurred in reading data, for example the storage device malfunctioned or file data is corrupted, the VM returns `ERROR_IO` or `ERROR_GENERIC`.

This can only be called on the output stream, the error stream, or a file opened in writable mode. If this is called on the standard input stream or on a handle opened only for reading, the behaviour is undefined. (The VM may crash or corrupt data, or return `ERROR_UNSUPPORTED` or `ERROR_GENERIC`, or abort the program.)



### `seek`

```c
int __sys_seek(int handle, int base, unsigned offset_low, int offset_high);
```

- syscall number: 7
- argument in r0: the handle of the file to seek
- argument in r1: the base position (0, 1, 2) from which to seek
- argument in r2: the low 32 bits of a 64-bit offset to add to the base position
- argument in r3: the high 32 bits of a 64-bit offset to add to the base position
- return value in r0: 0 on success or an error code

Sets the current position in the file to the given position.

The position is calculated as the given offset added to the given base.

- If `base` is 0, the offset is added to the start of the file. (In other words, the offset is the absolute position within the file.)
- If `base` is 1, the offset is added to the current position.
- If `base` is 2, the offset is added to the end of the file (i.e. the start plus its size.)

If the VM's maximum file size is less than the range of a 32-bit word (i.e. 4GB), the `offset_high` parameter can be ignored. Otherwise, the file position must be stored within the VM as a 64-bit value.

If successful, this returns 0.

If this is called on one of the standard input/output/error streams, the behaviour is undefined.

If the file is not seekable (for platform-specific reasons), this returns `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.

If some other error occurred in reading data, for example the storage device malfunctioned or file data is corrupted, the VM returns `ERROR_IO` or `ERROR_GENERIC`.



### `tell`

```c
int __sys_tell(int handle, unsigned position[2]);
```

- syscall number: 8
- argument in r0: the handle of the file from which to query the position
- argument in r1: the address at which to store the 64-bit position in the file
- return value in r0: 0 on success or an error code

Stores the current position in the given file to the given `position` address.

If successful, the VM stores two words at the address in r1: the low 32 bits of the position followed by the high 32 bits of the position.

The outputted value can be used in a call to `seek` with base 0 to return to this position in the file.

If the VM's maximum file size is less than the range of a 32-bit word (i.e. 4 GiB), the VM must still write a second word to the output with value zero.

If successful, this returns 0.

If this is called on one of the standard input/output/error streams, the behaviour is undefined.

If the file is not seekable (for platform-specific reasons), this returns `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.

If some other error occurred in reading data, for example the storage device malfunctioned or file data is corrupted, the VM returns `ERROR_IO` or `ERROR_GENERIC`.



### `trunc`

```c
int __sys_trunc(int handle, unsigned size_low, unsigned size_high);
```

- syscall number: 9
- argument in r0: the handle of the file to resize
- argument in r1: the low 32 bits of the 64-bit size to set
- argument in r2: the high 32 bits of the 64-bit size to set
- return value in r0: 0 on success or an error code

Sets the size of the file to the given size.

If the requested size is less than the current size of the file, the file is truncated: its size becomes that given and all data beyond that size is lost.

If the requested size is greater than the current size, the VM may ignore it and return `ERROR_UNSUPPORTED` or `ERROR_GENERIC`, or it may append zero bytes to the file until the size becomes that given. (VMs may internally optimize this to use sparse files.)

Returns zero if successful. In case of success, the file's size matches that given.

If the requested size is too large, the VM returns `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.

If some other error occurred in reading data, for example the storage device malfunctioned or file data is corrupted, the VM returns `ERROR_IO` or `ERROR_GENERIC`.



### `dirent`

```c
int __sys_dirent(int handle, char buffer[256]);
```

- syscall number: 12
- argument in r0: the handle of the directory to read
- argument in r1: address of a buffer in which to write the next directory entry
- return value in r0: 0 on success, error code otherwise

Reads the name of the next file in the given directory into the given buffer as a null-terminated string.

On success, a null-terminated string is written to the given buffer, and 0 (success) is returned. The string cannot be blank, and cannot contain the character `/` (the path separator) or a null byte (since it indicates the end of the string.)

If there are no more entries in the directory, `ERROR_END_OF_FILE` is returned.

The returned files may be in an arbitrary order, but as long as a directory is unchanged, the order of its contents must be consistent for the life of the program. Directory entries may be reordered arbitrarily when a file is added, removed or renamed. (This behaviour is required for the libc to simulate directory seeking.)

The VM may or may not return the special entries "." and "..". (The Onramp libc ignores them and simulates its own where appropriate.)

If the given handle is not a directory, `ERROR_UNSUPPORTED` or `ERROR_GENERIC` is returned.

If the storage device malfunctions or the filesystem is corrupted, `ERROR_IO` or `ERROR_GENERIC` is returned.

The VM may provide or omit the special filenames "." and ".." for any directory. (The Onramp libc ignores such entries and generates its own where appropriate. See [Directories](#directories).)



### `stat`

```c
int __sys_stat(const char* path, unsigned size[2]);
```

- syscall number: 13
- argument in r0: address of a null-terminated string containing the path to query
- argument in r1: address at which to write the 64-bit file size

Queries the type and size of the named file. See [File Types](#file-types).

If a file exists at the given path, the file's size is stored at the given address if possible, and the file's type is returned as one of the following values:

- 0: unknown type
- 1: [regular file](#regular-files)
- 2: [directory](#directories)
- 3: [device](#devices)
- 4: [stream](#streams)

The value 0 can be returned if the file exists and can be opened but the VM cannot determine the type of the file.

The given `size` address must point to the location of two words (8 bytes) to which the size can be written. If the size of the file is available, the VM stores at this address the low and high 32 bits of the size of the file in that order. The two words together form a 64-bit little-endian file size.

If the path is not a regular file, or if the size is not known, the VM can either ignore the size parameter and write nothing, or it can store 0xFFFFFFFFFFFFFFFF (all bits set) as the size, whichever is most convenient. (If the program is interested in the size, it should place this value at the location pointed to by the size parameter before calling this syscall in order to detect when the size is unavailable. The size argument cannot be null.)

If the VM's maximum file size is less than the range of a 32-bit word (i.e. 4GB), the VM must still write zero to the second word if it writes a valid size to the first. It must write both words or none at the address pointed to by `size`.

If the given path does not exist, this returns `ERROR_NO_SUCH_PATH` or `ERROR_GENERIC`.

If the storage device malfunctions or the filesystem is corrupted, `ERROR_IO` or `ERROR_GENERIC` is returned.

On any other error, `ERROR_GENERIC` is returned.

This system call is optional, although functionality may be limited without it. The simplest implementation of this system call is to ignore the size argument and return 0 if a file exists and `ERROR_GENERIC` if it does not.

This system call exists so that programs can check whether files exist and determine their properties without opening them, which may have side effects especially for devices. If this system call is not implemented, the libc will try to open files to check whether they exist and will use other syscalls (`seek`, `tell`, `dirent`) to determine their size and type.



### `rename`

```c
int __sys_rename(const char* source, const char* destination);
```

- syscall number: 14
- argument in r0: address of a null-terminated string containing the path of the source file or directory
- argument in r1: address of a null-terminated string containing the path of the destination file or directory
- return value in r0: 0 on success or an error code

Moves and renames a file or directory.

The file or directory named by the source path is moved and renamed to the destination path. If directories are supported, the file's parent directory becomes the base path of the destination and its filename becomes the last path component of the destination.

If the destination path already exists and is not a directory, the VM may delete the original file at the destination path and replace it with the source file, or it may leave the original file in place and return `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.

If the destination path is a directory, the VM should return `ERROR_UNSUPPORTED` or `ERROR_GENERIC`. (The libc should avoid this; TODO maybe we can make this undefined behaviour.)

If any directory component of the destination path does not exist, the VM may return `ERROR_NO_SUCH_PATH` or `ERROR_GENERIC`, or it may create the missing directories and succeed, or it may succeed without creating any directories. (A VM does not need to support directories.)

For example, if the file `/aaa/bbb/ccc` is renamed to `/ddd/eee/fff`, its parent directory becomes `/ddd/eee/` and its filename becomes `fff`. The destination directory `/ddd/eee/` must exist and must be a directory.

This system call is optional. If it is not implemented, the libc will provide this functionality by copying the file to the destination and deleting the source. This fallback behaviour may not be correct for streams or devices.



### `delete`

```c
int __sys_delete(const char* path);
```

- syscall number: 16
- argument in r0: address of a null-terminated string containing the path of the file to delete
- return value in r0: 0 on success or an error code

Deletes the file at the given path. If the file is a directory, it must be empty.

Returns 0 on success or an error code on error.

If the given path does not exist, this returns `ERROR_NO_SUCH_PATH` or `ERROR_GENERIC`.

If the given path is a non-empty directory, or if permission is not granted to delete the file, `ERROR_UNSUPPORTED` or `ERROR_GENERIC` is returned.

If the file cannot be deleted (perhaps due to a permission issue), this returns `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.

If the storage device malfunctions or the filesystem is corrupt, `ERROR_IO` or `ERROR_GENERIC` is returned.



### `chmod`

```c
int __sys_chmod(const char* path, int mode);
```

- syscall number: 17
- argument in r0: address of a null-terminated string containing the path of the file for which to change the executable flag
- return value in r0: 0 on success or an error code

Sets whether the file at the given path is executable in the host environment.

Only two values are supported for mode:

- 493 (0o755) -- The file is executable
- 420 (0o644) -- The file is not executable

Passing any other value for mode is undefined behaviour. (The VM may consider only the 7th least-significant bit as the executable flag. In other words, a bitwise *and* of the mode with 0o100 (256) yields the executable flag.)

This is only used for better integration of executables into the host system. Some operating systems require that files be marked executable before they can be executed. On such systems, programs that produce wrapped executables (like the Onramp compiler) and programs that produce native executables (like a compiler compiled by Onramp) can use `chmod` to mark their output executable.

This system call is optional. Some filesystems do not support an executable flag, and some operating systems do not require such a flag to execute programs. This syscall should only be implemented if the VM can do something meaningful with it. (In particular, it should be implemented on POSIX systems and it should not be implemented on Windows.)

If the VM does not have permission to modify the file, this returns `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.

If the given path does not exist, this returns `ERROR_NO_SUCH_PATH` or `ERROR_GENERIC`.

If the storage device malfunctions or the filesystem is corrupted, this returns `ERROR_IO` or `ERROR_GENERIC`.

If some other error occurs, this returns `ERROR_GENERIC`.

If the given path is a directory, the behaviour is undefined. (In POSIX, the execute bit permits searching directories. This behaviour is not supported in Onramp. The libc will try to prevent attempts to use `chmod` on directories.)



### `mkdir`

```c
int __sys_mkdir(const char* path);
```

- syscall number: 18
- argument in r0: address of a null-terminated string containing the path of directory to create
- return value in r0: 0 on success or an error code

Creates an empty directory at the given path.

The parent path consists of all but the last component of the path (see [Paths](#paths).) The last component of the path is the name of the directory to be created in the parent path.

Returns 0 on success or an error code on error.

If the path already exists as a directory, this returns `ERROR_IN_USE` or 0 (success) or `ERROR_GENERIC`.

If the path already exists as some other kind of file, this returns `ERROR_IN_USE` or `ERROR_GENERIC`.

If the parent path does not exist, this returns `ERROR_NO_SUCH_PATH` or `ERROR_GENERIC`. (This does not create directories recursively.)

If the parent path exists but is not a directory, this returns `ERROR_UNSUPPORTED` or `ERROR_NO_SUCH_PATH` or `ERROR_GENERIC`.

If the directory cannot be created due to a failure of the storage device, this returns `ERROR_IO` or `ERROR_GENERIC`.

If the directory cannot be created for other reasons (perhaps due to a permission issue), this returns `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.

This system call is optional. (The VM does not need to have a concept of directories; it can treat the filesystem as a flat set of files where `/` is simply part of the filename. On such a VM, directories only exist implicitly based on the existence of corresponding files.)



### `debug`

```c
int __sys_debug(const void* address, const char* /*nullable*/ executable_path);
```

- syscall number: 22
- argument in r0: address of an executable loaded into memory
- argument in r1: address of a null-terminated string containing the path of the loaded executable, or null if it is being unloaded
- return value in r0: 0 on success or an error code

Loads or unloads [debug info](#debug-info) for a child program at the given address.

If the given path is null, previously loaded debug info for the given address is unloaded.

If the given path is non-null, corresponding debug info is loaded for the executable at the given path.

Note that if a path is given, it must point to the executable, not to the debug info file. This allows VMs to store debug info in custom formats or locations. (The standard debug info format appends `.od` to the executable path.)

Returns 0 if successful.

If the given executable path is non-null and the file does not exist, the VM returns `ERROR_NO_SUCH_PATH` or `ERROR_GENERIC`.

If the given executable was found but debug info could not be found for it, the VM returns `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.

If the given path is null but debug info was not loaded at the given address, the VM returns `ERROR_UNSUPPORTED` or `ERROR_GENERIC`.

If the VM has insufficient resources to load additional debug info, the VM returns `ERROR_OVERFLOW` or `ERROR_GENERIC`.

If loading of the debug info fails for any other reason (e.g. the storage device fails, the debug info is corrupt, etc.), the VM returns `ERROR_IO` or `ERROR_GENERIC`.

If debug info is loaded successfully at the given address, the program is expected to unload the debug info at the same address later before reusing this memory for another program. If loading debug info returns an error code, the program does not need to follow up with a call to unload before reusing the memory, but it may attempt to unload debug info even if loading failed.

This syscall is optional and most VMs do not implement it. The [c-debugger](../platform/vm/c-debugger) VM is the main implementor of this syscall. See the [Debug Info](debug-info.md) specification for a description of the standard debug info language generated by the Onramp compiler toolchain.



### `alloc`

```c
int __sys_alloc(size_t size, void** /*out*/ address);
```

- syscall number: 2
- argument in r0: requested size
- argument in r1: address of a word at which to store the address of the allocation
- return value in r0: actual size or an error code

Allocates a large contiguous block of memory of at least the given size. The minimum size is 4 and the maximum size is 2147483647 (2^31-1). (Larger sizes would have the high bit set which would be interpreted as an error.)

The VM may allocate a block of any size up to the maximum as long as it is at least the given size. For example, the VM may round up the size to a multiple of some number (e.g. 4 KiB, 1 MiB, etc.)

If successful, the address of the allocation is stored at the address in r1, and the actual size of the allocation is returned.

The address of the allocation must be aligned to a word boundary, i.e. it must be a multiple of 4. (In other words the bottom two bits of the address must be zero.) The actual size does not have to be a multiple of 4, although programs will typically request multiples of 4 and will round the resulting size down to a multiple of 4 ignoring any extra few bytes.

On failure, nothing is stored to the address in r1 and an error code is returned in r0:

- If insufficient memory is available, or if no contiguous region of address space is available to satisfy the allocation, `ERROR_OVERFLOW` or `ERROR_GENERIC` is returned.

- If some other error occurs, `ERROR_UNSUPPORTED` or `ERROR_GENERIC` is returned.

The Onramp libc uses this to request large blocks of memory to back program calls to `malloc()`. It generally does not request blocks of memory smaller than 1 MB, although the bootstrap process may request as little as 256 kB. VMs typically round up the requested size to a multiple of 1 MB.

This system call is optional. The VM may instead simply provide memory at start large enough to accomodate the program's needs.



### `free`

```c
int __sys_free(void* address, size_t size);
```

Frees a block of memory previously allocated with `alloc` of the given actual size.

The value passed as the size parameter must exactly match the actual size (not the requested size) from the corresponding call to `alloc`. The VM may ignore it if it already knows the size, or it may use it without checking whether it matches the original size of the chunk. (The host program is required to store the unrounded size so that the VM doesn't have to. This is handled internally by the Onramp libc.)

This always returns 0. If the given address and size do not match a previously allocated block, the behaviour is undefined.

This system call is optional even if alloc is implemented. It is typical for hosted VMs to implement alloc and not free.

TODO: The name of this syscall is confusing: the "free" syscall releases a memory region, but we also use "free memory region" to refer to the initial region provided for the heap and stack of the program. We should either rename this to something else (dealloc?) or rename the initial free memory region.



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

These are conditional [jump if zero][jz] instructions, and since their predicates are non-zero constants, they do nothing. However, their encoding in ASCII is "`~Onr~amp~   `". This serves as a format indicator that identifies a file as containing an Onramp program. This preamble is not required but VM implementations may warn if a program does not start with it.

On some hosted platforms, Onramp bytecode can also be wrapped in a script that executes the Onramp virtual machine. This allows them to be executed like normal programs. For example, the POSIX wrapper looks like this:

```
#!/usr/bin/env onrampvm
# This is a wrapped Onramp program.
```

This script automatically launches the program in the VM (as long as it's on your PATH.) VMs that support these platforms check for a script preamble (e.g. `#!` or `REM`); if found, they skip the first 128 bytes.



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

Onramp's VM design takes inspiration from such projects as Robert Elder's [one page CPU][onepagecpu], the [TOY machine][toymachine] from Sedgewick and Wayne, the design of [MessagePack][msgpack], classic architectures like PDP-11 (designed to be programmed directly in octal), and of course modern RISC ISAs like RISC-V. See the [inspiration](inspiration.md) page for details.



## Version History

This spec has undergone several changes in its history. Version changes are listed here.

The libc and bootstrap code will attempt to remain compatible with versions 2 and later (when the syscall table was added) to avoid breaking existing VMs. Note however that several system calls were documented in versions 3 and earlier that were never used. The libc and bootstrap code will never call such syscalls on version 3 and earlier VMs even if they are used in version 4. Such system calls include `stat`, `rename`, `dirent`, `alloc` and `free`.


#### Version 4.0

The current version.

There have been many changes in version 4 but almost all of them are backwards compatible. The bootstrap process will continue to support version 3 and even version 2 VMs so existing VMs do not need to upgrade (unless they want to make use of the new features.)

Changes include:

- Changes to the process info table:
    - The System Call Count and Process Info Count fields have been replaced by a single [Minor Version][p-min] field (taking the place of the System Call Count.) This is much simpler for VMs to implement and for the bootstrap and libc to use.
    - A new [Additional Memory Regions][p-mem] field is added (taking the place of the Process Info Count field.)
    - All contents of the process info table, including command-line arguments, environment variables, the syscall table and the working directory, are now read-only. (This change is backwards-compatible for VMs; it's a new restriction on programs only.)
    - The "interactive" bit has been added to the capabilities field. It is optional and can be left zero.
- Some system call changes:
    - The prototype for `alloc` has changed. The old function was never used.
    - `dread` has been renamed to `dirent`.
    - `unlink` has been renamed to `delete`.
    - `symlink` (15), `spawn` (20), and `waitpid` (21) have been removed.
    - `dopen` (10), `dclose` (11), and `rmdir` (19) have been removed. `open`, `close`, and `delete` are now used for directories as well.
    - The `f` prefix has been dropped from many syscalls: `fopen`, `fclose`, `fread`, `fwrite`, `fseek`, `ftell`, `ftrunc`.
    - `read` and `write` can no longer return zero. The end of a file or a closed stream is indicated by `ERROR_END_OF_FILE`. A non-blocking stream without data or buffer space is indicated by `ERROR_TRY_LATER`.
    - `dirent` can no longer return an empty string. The end of a directory is indicated by `ERROR_END_OF_FILE`.
- Cleaned up syscall error codes:
    - Added `ERROR_END_OF_FILE` and `ERROR_TRY_LATER` to clearly differentiate between closed and non-blocking streams.
    - Added `ERROR_OVERFLOW` for situations in which the VM runs out of resources.
    - All error codes except `ERROR_GENERIC` and `ERROR_END_OF_FILE` are now optional. The spec now fully documents the expected error codes for all exceptional conditions in all syscalls.
- Added a definition of devices and the device `/dev/urandom`.


#### Version 3

- Added System Call Count and Process Info Count fields to the Process Info Table.
- Added `alloc` and `free` system calls (although these were never used.)
- The program is now allowed to close the standard input, output and error streams. (Closing the standard streams was forbidden in v2 and earlier.) The VM may ignore such requests.

The latest spec for version 3 is in commit [c0110ef5][v3-spec]. Note that it documents several syscalls that were never used, and some of those have had their API changed.


#### Version 2

- Added the System Call Table.
    - The System Call Table address replaces the Exit Address in the Process Info Table.
        - The Exit Address was a value that the program would assign to `rip` to exit. This mechanism is now used for all system calls.
    -  Removed `sys` instruction. (The opcode `0x7F` is now unused and will remain so because some VMs continue to use it to implement the syscall mechanism.)
- Some syscalls have changed:
    - `halt` has been renamed to `exit`.
    - `panic` has been added, displacing `time`.

The latest spec for version 2 is in commit [d313b92c][v2-spec]. Like version 3, several syscalls were never used, and some of those have had their API changed.


#### Version 1

- Instructions have changed:
    - `ltu` replaces `cmpu`. (The `cmpu` instruction performed a three-way comparison returning -1, 0 or 1.)
    - `shru` replaces `ror`. (The `ror` instruction performed a 32-bit rotate (i.e. without carry) instead of a shift.)
    - `shl` replaces `xor`. (The `xor` instruction performed a bitwise XOR similar to the `and` and `or` instructions.)

The new instructions are simpler for VMs to implement and are much more useful to programs.

The bump to version 1 was actually made a few months after the `shru` and `shl` instructions were added to the document. I'm listing them here as a version 1 change anyway. The difference between version 0 and 1 is a bit fuzzy because version 0 was still experimental; the spec was not versioned seriously until version 1.

The spec for version 1 is in commit [36bb35db][v1-spec].


#### Version 0

This was the initial experimental version of the Onramp VM. Several breaking changes were made without bumping the version number; in most cases it is impossible for the program to tell on which "version 0" it is running so there is no way for a program to be portable to all version 0 VMs.

Some changes during version 0 are:

- Syscalls were renumbered. For example, `fwrite` was originally 19; it was later changed to 6.
- `fread` and `fwrite` originally had to read or write exactly the number of bytes requested (if possible). This was relaxed so they could read or write less.
- `ftell` was added and its redundant functionality was removed from `fseek`.
- `ftrunc` was added and the behaviour of `fopen` was simplified, allowing programs to support both truncate and append without needing more VM `fopen` modes.
- `chmod` was changed to operate on a path rather than a handle.

Most of these changes were made to simplify VMs at the expense of some complexity in the libc and bootstrap.

The best commit representing version 0 is probably [8ace5628][v0-spec], although changes were made before and after this commit with the same version number. The spec at that commit is assumed by the vminfo tool for version 0.



<!--
Markdown link references follow.
-->

<!-- opcodes -->
[add]: #add
[sub]: #subtract
[mul]: #multiply
[divu]: #divide-unsigned
[and]: #bitwise-and
[or]: #bitwise-or
[shl]: #shift-left
[shru]: #shift-right-unsigned
[ldw]: #load-word
[stw]: #store-word
[ldb]: #load-byte
[stb]: #store-byte
[ims]: #immediate-short
[ltu]: #less-than-unsigned
[jz]: #jump-if-zero

<!-- pit fields -->
[p-maj]: #major-version
[p-heap]: #heap-start-address
[p-sys]: #system-call-table-address
[p-in]: #standard-input-handle
[p-out]: #standard-output-handle
[p-err]: #standard-error-handle
[p-arg]: #command-line-arguments
[p-env]: #environment-variables
[p-wd]: #working-directory
[p-cap]: #capabilities
[p-min]: #minor-version
[p-mem]: #additional-memory-regions

<!-- urls -->
[msgpack]: https://msgpack.org/
[onepagecpu]: https://recc.robertelder.org/op-cpu-programmer-reference-manual.txt
[toymachine]: https://introcs.cs.princeton.edu/java/62toy/
[v0-spec]: https://github.com/ludocode/onramp/blob/8ace5628af12329c02f7bcd46f4f00b47f82beb0/docs/virtual-machine.md
[v1-spec]: https://github.com/ludocode/onramp/blob/36bb35db73732e55a74b56f982679f72243797c6/docs/virtual-machine.md
[v2-spec]: https://github.com/ludocode/onramp/blob/d313b92c79a6a5cab5553fde3a8c85e0e59759e2/docs/virtual-machine.md
[v3-spec]: https://github.com/ludocode/onramp/blob/c0110ef5e7de3a0d3f8136ffd82b1c893ae670be/docs/virtual-machine.md
[wp-ansiesc]: https://en.wikipedia.org/wiki/ANSI_escape_code
[wp-canon]: https://en.wikipedia.org/wiki/POSIX_terminal_interface#Canonical_mode_processing
[wp-heap]: https://en.wikipedia.org/w/index.php?title=Heap_(programming)
[wp-redzone]: https://en.wikipedia.org/wiki/Red_zone_(computing)
[wp-sbrk]: https://en.wikipedia.org/wiki/Sbrk
[wp-stack]: https://en.wikipedia.org/wiki/Call_stack

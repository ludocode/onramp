# Onramp Virtual Machine Implementations

The Onramp Virtual Machine is a simple machine designed for bootstrapping.

This README concerns the various implementations of the VM. For a definition of the machine and its bytecode, see [Onramp Bytecode](../../docs/virtual-machine.md).



### Freestanding

Freestanding VMs are implemented in hexadecimal machine code and are designed to run on bare metal. They implement very few syscalls, instead passing control over to the contained [Onramp OS](../../core/os/).

No such VMs have been implemented yet. I intend to implement `x86-bios` and `x86_64-uefi` VMs at some point based on the existing `x86_64-linux` VM. See [`x86-bios/`](x86-bios/) for ideas of how this might work.



### Hosted Machine Code

These VMs are written in hexadecimal machine code in an executable container for a particular OS.

Currently only [`x86_64-linux`](x86_64-linux/) has been implemented. It uses an ELF container and makes Linux syscalls. There is very little error checking and things like signals are not handled correctly, but this is not that important as Onramp is mainly intended to do non-interactive bootstrapping of a real toolchain.

I intend to reduce this VM to `x86-linux`, then convert it to a Windows executable. The ELF container will have to be converted to PE and it will need to link against MSVCRT.DLL rather than directly making syscalls as the Linux VM does.

Assuming this is successful, it should be straightforward to write VMs in machine code for several other platforms. It would be nice to have an `aarch64-linux` VM that can bootstrap Onramp on a Raspberry Pi or an Android phone.

The `x86_64-linux` machine code VM happens to be written in a single stage but it isn't necessary for all such VMs to be one stage. You could instead implement a small platform-specific "assembler" to calculate jump offsets or replace keywords with bytes (like the early [stage0](https://github.com/oriansj/stage0) tools), then use that to implement the VM. It may be worthwhile to try this for a RISC-V VM for example.

You could also write the VM in a real platform-specific assembly language, provided you have a way to bootstrap that assembler from source. (If the assembler is written in C, this obviously isn't going to work.)



### Compiled Languages

These VMs are written in compiled languages. Their speed is comparable to (or better than) machine code VMs, but of course they depend on an existing compiler.

[`c89`](c89/) is the only VM so far written in a compiled language. It is useful if you have an older system that only has an ANSI C compiler and you want to bootstrap a more modern C11 or later compiler with Onramp.



### Interpreted Languages

The [`python`](python/) VM is written in an interpreted language. Interpreted VMs tend to be much slower than their compiled or machine code counterparts, but the code is very simple and easy to understand. This is the easiest way to bootstrap Onramp.

They are useful on systems that come with an interpreter by default but do not otherwise have a way of compiling code. For example macOS comes with Python out-of-the-box so you can use this to bootstrap a C compiler without needing anything else.

(The [`sh`](sh/) VM is interpreted but it is currently way too slow to be useful and it doesn't implement most syscalls necessary to make Onramp work.)



### Debugger

The `c-debugger` is a special VM that has an integrated debugger. It can load debug info to determine source file and line information for every instruction in a program. It can step through disassembled bytecode, displaying the state of registers, the stack, and heap memory. It tracks function calls and prints stack traces when something goes wrong.

It is incomplete but it's useful enough that it is the primary VM used for developing Onramp.

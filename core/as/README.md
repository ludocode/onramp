# Onramp Assembler

`as` is the Onramp assembler. It takes Onramp assembly language as input and outputs Onramp object files.

This is a description of the implementation stages of the assembler. For a specification of the input to the assembler, see [Onramp assembly](../../docs/assembly.md). For a specification of the output of the assembler, see [Onramp object code](../../docs/object-file.md).



## Stages

The Onramp assembler is implemented in the following stages:

- [`0-basic`](0-basic/): The basic assembler supports quoted bytes, raw strings, and keyword replacement for primitive opcodes, registers and syscalls. It supports the same label syntax as Onramp object files.

- [`1-compound`](1-compound/): The compound assembler adds all compound instructions necessary for bootstrapping (e.g. `push`/`pop`, `call`/`ret`.) These are assembled into multiple primitive instructions in bytecode. It also adds decimal numbers which expand contextually to the appropriate number of bytes. It greatly improves error-checking over the previous stage as it verifies that opcodes are used correctly.

- [`2-full`](2-full/): The full assembler resolves relative labels, greatly reducing link time and link memory usage. It also adds additional compound instructions (e.g. `ldwu`, `stwu`) as well as optimizations for some compound instructions. It further improves error checking, gives better error messages, and outputs debug info for easier debugging.

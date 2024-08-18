# Onramp Assembler

`as` is the Onramp assembler. It takes Onramp assembly language as input and outputs Onramp object files.

This is a description of the implementation stages of the assembler. For a specification of the input to the assembler, see [Onramp assembly](../../docs/assembly.md). For a specification of the output of the assembler, see [Onramp object code](../../docs/object-file.md).



## Stages

The Onramp assembler is implemented in the following stages:

- [`0-basic`](0-basic/): The basic assembler supports quoted bytes, raw strings, and keyword replacement for primitive opcodes, registers and syscalls. It supports the same label syntax as Onramp object files.

- [`1-compound`](1-compound/): The compound assembler adds compound instructions, e.g. `push`/`pop`, `call`/`ret`. These are assembled into multiple primitive instructions in bytecode. It also adds decimal numbers which expand contextually to the appropriate number of bytes. It greatly improves error-checking over the previous stage as it verifies that opcodes are used correctly.

- [`2-full`](2-full/): The full assembler adds debug info and optimizations for some compound instructions. It further improves error checking and gives better error messages.

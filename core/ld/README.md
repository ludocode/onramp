# Onramp Linker

`ld` is the Onramp linker. It takes Onramp VM object files as input. It resolves symbol and label addresses and outputs an Onramp bytecode program that can be run in an Onramp virtual machine.

This is a description of the implementation stages of the linker. For a specification of the input to the linker, see [Onramp object code](../../docs/object-code.md). For a specification of the output of the linker, see [Onramp bytecode](../../docs/virtual-machine.md).



## Stages

The linker is written in three stages:

- `0-global` has only rudimentary label support. It treats all labels and symbols the same, so all labels and symbols must be globally unique. It is sufficient to link hand-written object or assembly files with at most one compiled C source file. It is implemented directly in hexadecimal bytecode so it is kept as simple as possible.

- `1-c89` adds support for file scope for static symbols and labels. This allows it to link multiple C source files together without clashes in generated label names. It also lifts previous restrictions on symbol and label count. It is sufficient for our omC and opC compilers. It is written in a single omC file so it can be linked using the previous stage.

- `2-full` adds support for weak symbols, zero symbols and constructor/destructor lists. It allows us to implement `__attribute__((weak))`, `__attribute__((constructor))`, `-fcommon`, and everything else we need for full C plus extensions. It performs garbage collection of unused symbols to reduce binary size. It also generates full debug info.

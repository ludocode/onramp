# Onramp Linker -- First Stage

This is the implementation of the Onramp first stage linker. It is written directly in hexadecimal [Onramp bytecode](../../../docs/virtual-machine.md). It uses techniques for [coding without labels](../../docs/coding-without-labels.md) such as fixed function addresses with padding and address assertions.

This linker does not differentiate between labels and symbols, except that symbols are padded to a word boundary. It does not support file scope so labels and static symbols must be globally unique. (This restriction is lifted by the second stage linker, just in time for the omC compiler to generate label names.)

The linker supports multiple input files. It performs two passes over all input. In the first pass it collects label addresses, and in the second pass it outputs the final bytecode. Unfortunately, since there is no way to link (or even concatenate) multiple files together until this tool exists, this implementation must be all in this one large file.

## Implementation Details

The linker stores labels and symbols in a large fixed-size FNV-1a hashtable with open addressing and linear probing. Each bucket is two words: a pointer to the symbol name and the address with which to replace it. Hashing only adds a couple dozen instructions over a flat linear search and (I assume) it significantly improves performance since larger programs have hundreds of labels (e.g. cci/0 is nearly 700.) The linker limits the total number of labels to half the hashtable size to prevent it from devolving into a linear search.

The linker reads labels directly into heap space at the program break. When defining a label, the program break is bumped, thereby "allocating" it. Defined labels are never freed; all the memory is reclaimed by the parent process when the linker exits.

The linker includes robust error checking. It correctly handles nearly all invalid input and it ensures that it does not run out of memory when allocating strings.

We modify the standard calling convention to treat r9 as globally preserved. We use it to point to a table of global variables that we allocate at startup.

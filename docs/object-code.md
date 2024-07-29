# Onramp Object Code

**Onramp object code** is relocatable [Onramp bytecode](virtual-machine.md) in hexadecimal plain text. An **Onramp object file** is a file containing Onramp object code. These files are the input to the Onramp linker to produce a runnable Onramp program.

This document provides the specification of Onramp object code. [The implementation of the linker is described here.](../core/ld/README.md)

This document also includes the specification of Onramp static libraries. [The implementation of the archiver is described here.](../core/ar/README.md)


## Overview

Onramp object code is Onramp hexadecimal bytecode annotated with symbols, labels, comments and debug info. It is essentially just relocatable Onramp bytecode.

Onramp object files are analogous to native object files in a conventional C compilation process. They are the input to the Onramp linker, which takes one or more object files, resolves addresses and produces a runnable program.

Nevertheless, Onramp object files are plain text files. They may be hand-written for bootstrapping purposes or viewed in plain text for debugging. Once the Onramp assembler is bootstrapped, it converts human-readable assembly instructions into Onramp object files.

Onramp static libraries are essentially just concatenated object files with metadata to delimit them. The metadata allows the archive contents to be modified and allows the linker to correctly treat each contained object file as individually scoped. Onramp archives do not have indexes, although the tools pretend that they do.

The Onramp object file format is described here.



## Quick Reference

| Syntax | Meaning                                       |
|--------|-----------------------------------------------|
| `;`    | line comment                                  |
| `#`    | debug info                                    |
| `%`    | static library member info                    |
| `^`    | invocation: absolute, 32 bits                 |
| `<`    | invocation: absolute, 16 high bits            |
| `>`    | invocation: absolute, 16 low bits             |
| `&`    | invocation: relative, 16-bit signed words     |
| `:`    | definition: label                             |
| `=`    | definition: global symbol                     |
| `@`    | definition: static symbol                     |
| `+`    | flag: zero symbol                             |
| `?`    | flag: weak definition or invocation           |
| `{`    | flag: constructor, optional priority 0-65535  |
| `}`    | flag: destructor, optional priority 0-65535   |



## Syntax

An Onramp object file consists primarily of hexadecimal plain text bytes. It is annotated throughout with labels and symbols, debug info, and comments and whitespace.



### Whitespace

Whitespace is ignored, except that a carriage return or line feed ends a comment or debug marker.



### Comments

A `;` character starts a line comment. All bytes until the following carriage return or line feed character are ignored.



### Hexadecimal Bytes

A hexadecimal character adds a byte to the current symbol. A byte consists of two hexadecimal characters in uppercase or lowercase.

After parsing the input for label and symbol addresses, hex bytes (that are not part of stripped symbols) are converted to raw binary and written to the output.



### Identifiers

All remaining syntax has the same form: a set of control characters followed by an identifier (with nothing in between).

- `<control characters>` `<identifier>`

An identifier is a sequence of arbitrary length consisting of letters, digits, underscores or a dollar signs, with the exception that it cannot start with a digit.

Identifiers starting with two underscores or an underscore and an uppercase letter are reserved for the implementation.



### Label Definitions

- `:` `<identifier>` defines a label with the given name.

In the first stage linker, labels have global scope, and symbols are treated the same as labels. You must make sure to use globally unique names for labels. (Typically this is done by prepending the name of the function that contains them.)

In later stages of the linker, labels are local to object files, as in a conventional assembler. Labels can be re-used for different meanings in different object files.



### Symbol Definitions

A symbol is a named group of bytes and labels in an object file.

- `=` `<identifier>` defines a global symbol with the given name.
- `@` `<identifier>` defines a static symbol with the given name.

A global symbol is visible to all files being linked by the linker. At most one global symbol can be defined with the given name (unless the definition is weak; see below.)

A static symbol is only visible within its object file. The same name can be re-used for unrelated static symbols or labels in different object files.

All subsequent bytes, label definitions, invocations and debug information are considered to be part of this symbol until the next symbol definition. No such data is permitted before the first symbol definition.

Symbols may be stripped from the linked program, for example if they are unused or if they are weak and redundant.

The first symbol in the first file given to the linker must be a global symbol called `__start`. This is the entry point of the program. The `__start` symbol is never stripped. (Once the libc is bootstrapped, `__start` is provided by the libc and the compiler driver automatically inserts it as the first file.)

Symbols are padded as necessary so that each symbol starts on a four byte boundary. (The last symbol may not be padded. There is no guarantee that the padding is zero or even readable.)



### Invocations

Invocations are used to insert the address of a label or symbol into the program code.

- `^` `<identifier>` invokes the full 32 bits of a label or symbol. It is replaced by the four bytes of the absolute address of the definition in little-endian order.

- `<` `<identifier>` invokes the high 16 bits of a label or symbol. It is replaced by the high two bytes of the absolute address of the definition in little-endian order.

- `>` `<identifier>` invokes the low 16 bits of a label or symbol. It is replaced by the low two bytes of the absolute address of the definition in little-endian order.

- `&` `<identifier>` invokes the 16-bit relative address of a label or symbol. It is replaced by the signed difference in words between the address after the invocation and the address of the definition in little-endian order.

The high and low invocations are virtually always used together to load the complete address of a label into a register. For example:

```asm
7C 8A <malloc   ; ims ra <malloc
7C 8A >malloc   ; ims ra >malloc
```

This loads the address of `malloc()` into register `ra`. If the address of malloc is `0x12345678`, the linker replaces the above with:

```asm
7C 8A 34 12
7C 8A 78 56
```

The 32-bit invocation is used to insert label and symbol addresses as constant data, not as part of an instruction. This is used to create arrays of string literals for example.

The relative invocation is used for relative jumps with the conditional jump instruction. For example:

```asm
7E 80 &foo   ; jz r0 &foo   ; if r0 is zero, jump to foo
```

The relative address of the label must fit within a 16-bit signed number. (The first stage linker may not diagnose this error but the later stage linkers do.)

Note that the relative difference is *in words*, not in bytes. The address difference between the invocation and label it references must be a multiple of 4. This is always the case when the relative invocation is used correctly: it must appear at the end of an instruction and must reference a label between instructions.



### Weak Symbols

A global symbol definition or an absolute invocation can be prefixed with `?` to make it weak.

- `=` `?` `<identifier>` defines a weak global symbol with the given name.
- `^` `?` `<identifier>` invokes the full 32 bits of a definition, or four zero bytes if the definition does not exist.
- `<` `?` `<identifier>` invokes the high 16 bits of a definition, or two zero bytes if the definition does not exist.
- `>` `?` `<identifier>` invokes the low 16 bits of a definition, or two zero bytes if the definition does not exist.

A global symbol definition that is not weak is called strong. If a strong definition of a symbol exists, the weak definitions are stripped. Otherwise, if multiple weak definitions exist, only the first is kept and the others are stripped.

A weak invocation can be used whether or not the label or symbol exists. If the symbol exists, the invocation functions normally. If the symbol does not exist, the invocation is replaced with zero.

Static symbol definitions, label definitions, and relative invocations cannot be weak.



### Zero Symbols

A global or static symbol definition can be prefixed with `+` to make it zero.

- `=` `+` `<identifier>` defines a zero global symbol with the given name.
- `@` `+` `<identifier>` defines a zero static symbol with the given name.

A zero symbol does not contain literal data. Instead, it must contain exactly four hex bytes in little-endian order which define the size of the symbol.

The linker moves all zero symbols to the end of the program. It assigns each one a unique address starting at the end of the program, i.e. after the program break. The linker defines the following symbol internally and adds it to the end of the program:

- `__onramp_zero_size` -- The combined size of all zero symbols

This symbol contains a single little-endian word which is the size of the area to reserve for zero symbols. On program start, the Onramp libc increments the program break by this size and zeroes the area, thus allocating space for all zero symbols in the program.

This is analogous to the [bss section](https://en.wikipedia.org/wiki/.bss) of an object file or executable on UNIX and Windows systems.

The weak and zero flags can both be provided on a global symbol definition in either order. (In such a case, it is strongly recommended that all instances of a weak zero symbol have the same size.)



### Constructors and Destructors

Symbols can be marked *constructor* to run automatically at program start, and *destructor* to run automatically at normal program exit.

- `=` `{` `[priority]` `<identifier>` defines a symbol and inserts its address into the constructor list.
- `=` `}` `[priority]` `<identifier>` defines a symbol and inserts its address into the destructor list.

The priority is optional. If provided, it must be one to five decimal digits in the range 0 to 65535. If not provided, the priority is 65535.

The linker defines the following symbols and adds them to the program:

- `__onramp_constructors` -- The list of constructors
- `__onramp_destructors` -- The list of destructors

Each of these symbols contains a null-terminated list of 32-bit values, where each value is the address of a symbol marked constructor or destructor. The constructor list is sorted by priority (lower first); constructors with equal priority are in the order they were encountered in the input files. The destructor list is in reverse order.

The Onramp libc uses these symbols to run all static constructors and destructors in a program. (`__onramp_constructors` is analogous to the `init_array` section of an ELF file or the `.CRT$XCU` section in Microsoft Visual C++.)

Symbols added to the constructor or destructor lists are never stripped (even if they appear in a file within a static library archive that is otherwise unused.)

Weak or zero symbols cannot be marked constructor or destructor.

The Onramp C compiler outputs these flags for global initializers and for functions marked  `__attribute__((constructor))` and `__attribute__((destructor))`. Note that `__attribute__((constructor(<priority>)))` accepts values in the range 101-65535; smaller values are reserved. Constructor functions for global variable initializers are emitted with priority 50.



### Debug Info

A `#` character starts a debug line.

Debug lines are output verbatim to the debug file interspersed with the number of bytes between them. Debug lines belong to symbols so they are output only for symbols that are not stripped.

More formally:

When outputting a symbol, upon encountering a `#`, the linker checks the number of bytes written to the output since the last `#` (or the start of the output.) If this number is not zero, it is written to the debug file in decimal plain-text with a trailing line feed. Then, the `#` character and all subsequent bytes up to and including the newline are written to the debug file.

The first stage linker treats debug info as comments.



### Archive Info

A `%` character starts a line describing a member of a static library.

- `%` `[filename]`

The archive info runs until the end of the line. The rest of the line is reserved for future archive metadata.

This instructs the linker to treat this line as a file break for the purpose of scoping labels and static symbols. This allows multiple object files to be concatenated together into a static library.

An Onramp static library is essentially a set of Onramp object files concatenated together. It has the file extension `.oa` and can be manipulated with the [`ar` archiver tool](../core/ar). Aside from the addition of `%` lines to delimit the contents, it is identical to an Onramp object file.

The filename is used by the linker for error reporting and by the archiver for manipulating the archive. It does not affect the output.

The first stage linker does not support file scope. It treats archive info as comments.

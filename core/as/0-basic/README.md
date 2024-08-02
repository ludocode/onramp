# Onramp Assembler -- First Stage

This is the implementation of the first stage (primitive) assembler. It is written in [Onramp object code](../../../docs/object-file.md).

It takes primitive [Onramp assembly](../../../docs/assembly.md) as input and outputs Onramp object code.

The primitive assembler does basic keyword replacement. It contains a list of the names of registers, opcodes and syscalls, and each one is mapped to a byte. When the assember encounters an identifier, it simply finds the keyword in the list by linear seach and outputs the corresponding byte. That's it.

This does not check that the keyword makes sense in the context in which it is used. It has almost no error checking at all in fact. The later stage assemblers, in combination with manual auditing, should be used to verify that an assembly file has the correct syntax.

This assembler can parse strings and quoted hexadecimal bytes, outputting both as hexadecimal. It can also parse linker directives, outputting them verbatim. It discards comments and debug lines.

This stage does not support decimal numbers or compound instructions. Numbers must be written as quoted hexadecimal bytes; stack frame setup and function calls must be done manually; and so on. Handwritten primitive assembly is structured very much like object code and shares the same limitations. The only real difference is that keywords are used in place of raw bytes for opcode and register names.

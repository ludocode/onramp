# Onramp Shell

`sh` is the Onramp shell. It's used for running scripts or (in theory) running programs interactively on the Onramp VM.

This document describes the implementation. For a specification of the shell language, see [Onramp shell](../docs/shell.md).

The Onramp shell has only one stage. There is no need for any other features to bootstrap the full C compiler. Once we have our C compiler we can replace the Onramp shell with a real POSIX shell.

The implementation does not have a real memory allocator. It reads from the input file directly into heap space at the program break. It collects whitespace-delimited tokens, keeping the start of each token in an array of command-line arguments. When it finds the end of a line (an unescaped newline, a comment or the end of the file), it checks the first token and executes the corresponding command. After running the command, it resets the heap pointer and starts over with the next line.

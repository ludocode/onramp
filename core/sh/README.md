# Onramp Shell

`sh` is the Onramp shell. It's used for running bootstrap scripts on the Onramp VM.

This document describes the implementation of the shell. For a specification of the shell language, see [Onramp shell](../../docs/shell.md).



## Setup

This tool must be built in a platform-specific way. Relative to the root of the Onramp source tree, the output must be placed in:

```
output/intermediate/sh/sh.oe
```

See the [Setup Guide](../../../docs/setup-guide.md) for more information.



## Functionality

Only the following commands are supported:

- `echo` -- prints the arguments to standard output
- `set -e` -- exits on error
- `mkdir -p` -- creates the given directory recursively if it does not exist
- `cp` -- copies a file
- `onrampvm` -- runs an Onramp program
- `sh` -- runs a script in a subshell

The shell can run commands from standard input but it is not designed to run interactively. It does not have a prompt and it does not have enough functionality to be usable as a shell. It is missing basic commands such as `cd` and `ls`.



## Implementation

The Onramp shell has only one stage. There is no need for any other features to bootstrap the full C compiler. Once we have our C compiler we can replace the Onramp shell with a real POSIX shell.

The implementation does not have a real memory allocator. It reads from the input file directly into heap space at the program break. It collects whitespace-delimited tokens, keeping the start of each token in an array of command-line arguments. When it finds the end of a line (an unescaped newline, a comment or the end of the file), it checks the first token and executes the corresponding command. After running the command, it resets the heap pointer and starts over with the next line.

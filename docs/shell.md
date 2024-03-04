# Onramp Shell

The Onramp Shell language is used to write [the scripts that bootstrap Onramp](../core/build.sh) within an Onramp virtual machine.

It is a minimal subset of POSIX shell (with minor exceptions.)



## Overview

The Onramp shell is as simple as possible: a command is a line of whitespace-separated tokens. The first token is the command and subsequent tokens are its arguments. Supported commands are:

- `echo` -- prints the arguments to standard output
- `set -e` -- exits on error
- `mkdir -p` -- creates the given directory (non-recursively!) if it does not exist
- `cp` -- copies a file
- `onrampvm` -- runs an Onramp program
- `sh` -- runs a script in a subshell

When a line comment `#` is encountered the rest of the line is ignored. Line endings outside of comments can be escaped with a `\\`.

The above constitutes the complete functionality of the shell. There are no pipes, no stream redirections, no conditionals, no control structures, no error handling, no escape sequences, no variables, no prompt, no line editing, no history, etc.

The language is designed to be compatible with POSIX shell scripts. This allows us to run Onramp shell scripts with an ordinary POSIX shell for debugging purposes (as long as they use UNIX-style line endings.)



## Syntax

An Onramp shell script is separated by lines. A line ends in one of four ways:

- a carriage return followed by a line feed;
- a carriage return only;
- a line feed only;
- the end of the input file.

(Note that this differs from POSIX where only a line feed is considered to be the end of a line.)

A `\` character at the end of a line that does not have a comment escapes the line ending. The backslash and line ending are elided and the two lines are merged together without any additional whitespace (so it is possible to merge tokens across lines.)

A `\` cannot otherwise appear at all outside of comments (no other escape sequences are supported.)

Some special characters are forbidden anywhere outside of comments: `"`, `'`, `\``, `(`, `)`, `\\`, `&`, `$`, and probably more to be added.

A line can contain a command. A command consists of a series of whitespace-separated tokens. The first token is the name of the command and subsequent tokens are its arguments.

A comment starts with a `#` either at the start of the line or after horizontal whitespace. It completes the preceding command (if any) and runs until the end of the line (which cannot be escaped.)

Once a line is consumed from the input, the command it contains (if any) is executed. If a line contains only whitespace and/or comments, the line is ignored.

When the end of the input is reached, the shell runs the command contained in the line just read (if any), then exits with the status code of the last executed command.



## Commands


##### `echo [args...]`

The `echo` command prints all subsequent arguments to standard output. Each argument is followed by a single space, except the last argument which is followed by a line feed. If no arguments are provided, a single line feed is printed.

Note that the arguments cannot be quoted. Quotes are not allowed anywhere outside of comments.


##### `mkdir -p <dirname>`

The `mkdir` command creates a directory if it does not exist. `-p` must be the first argument. The name of the directory to create is the second argument.

WARNING: The current version of the shell does not create directories recursively! The required `-p` option exists only for compatibility with POSIX shells to not fail if the directory already exists.


##### `cp <source> <destination>`

The `cp` command copies a file. Only a single source file may be given. The destination cannot be a directory; it must be the full destination filename.


##### `onrampvm <program> [args...]`

The `onrampvm` command runs an Onramp program.

The first argument is the name of the program. All arguments (including the program name as the zeroth argument) are passed to the program.

The program inherits the shell's environment variables.


##### `sh [script]`

The `sh` command runs another Onramp shell script.


##### `set -e`

The `set` command must be given `-e` as its only argument. It causes the shell to stop on any subsequent error.

The shell will exit with the same status code as the failing command.

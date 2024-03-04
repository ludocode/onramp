# Onramp Debug Info

Onramp Debug Info is a file format that contains debug information for an Onramp executable.

It has the file extension `.od`, appended to the full filename of the executable including its own extension. (So for a typical unwrapped Onramp executable, it would have the extension `.oe.od`.)

## Syntax Overview

Onramp debug info is a plain text file. Each line is either a comment, a directive or a byte count. The directives indicate metadata such as source location, variable names, etc. The numbers indicate the number of subsequent bytes in the executable for which the previous directives apply.

For example:

```c
; Onramp debug info for someprogram.oe
#line 9 "foo.c"
12
#line 13 "bar.c"
18
```

This indicates that the first 12 bytes of the executable correspond with line 9 of `foo.c` and the subsequent 18 bytes correspond to line 13 of `bar.c`.

## Comment

A `;` indicates a line comment. The rest of the line is ignored.

## Byte Count

Numbers are provided in decimal and are always on their own lines. They indicate the number of bytes over which previous directives apply.

A directive may be followed by several byte counts or none. Do not assume each directive is always followed by one byte count. Consecutive byte counts should be added together.

Note that the total byte count may exceed the size of the executable. Debug info may also cover the zero (bss) section after the executable.

## Directives

The directive syntax is inspired by C and matches the debug directives in Onramp assembly and Onramp object files.

### Line Directive

A `#line` directive has the same syntax as in C:

```c
#line <number> ["filename"]
```

The first argument is a line number and the optional second argument is a filename in double quotes. If a filename is not provided, it means the filename has not changed from the previous directive.

There are no escape sequences for the quoted filename. Filenames with double quotes or other non-printable characters are not supported.

A line number of 0 can be used to indicate that source line number information is not available. Valid line numbers start at 1.

### Symbol Directive

A `#symbol` directive names the symbol for subsequent bytes.

```c
#symbol <symbol>
```

### Line Increment

A bare `#` directive is a line increment. The previous filename has not changed and the line is incremented by 1. (This should not be used if the previous line number is 0.)

### Other

Directives for indicating variable names and other debug metadata are not yet implemented.

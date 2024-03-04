# Onramp Hexadecimal

Onramp Hexadecimal is a simple language used to implement binary files in plain text hexadecimal with comments. This makes it possible to handwrite machine code and bytecode in clear, maintainable, auditable text.

A hex conversion tool is required to strip comments and convert the text to binary. There are many implementations in [`platform/hex/`](../platform/hex/). The language they implement is described in this document.



## Plain Text

An Onramp Hexadecimal file must be in plain text. All non-comment data is in ASCII.

In our Onramp sources we stick to the printable ASCII characters plus whitespace, although the tools should be able to handle any non-null bytes in comments. For maximum compatibility you should use UTF-8 if you want international characters in comments.



## Hexadecimal Bytes

Bytes are specified as two hexadecimal characters. Characters can be in lowercase or uppercase.

```
48 65 6c 6C 6f
```

An Onramp Hex tool converts the above to the five bytes "Hello".



## Comments

The `;` character starts a line comment. The rest of the line until the next carriage return or line feed character is ignored.

```asm
; this is a comment

48 65 6c 6c 6f  ; "Hello"
```



## Debug Info

A `#` character is intended to be used for debug info. Currently all hex tools treat this as another kind of line comment.



## Whitespace

The space, horizontal tab, line feed and carriage return characters are considered whitespace. They are ignored (except that line endings end a comment.)

```
48 65 6c 6c 6f  ; hex bytes can be spaced out
48656C6C6F      ; or compressed

48      ; or separated in lines
65
6C
6C
6F

    48656c      ; or indented
		6C6F    ; with tabs or spaces
```



## Address Assertions

An address assertion is a special kind of comment that a hex tool can use to verify the current address.

The `@` character starts an address assertion. It is followed by `0x` and a hexadecimal address. A hex tool that supports address assertions verifies that the value of the address assertion matches the number of bytes that have been emitted so far.

The address must be followed by a line ending or by horizontal whitespace. After horizontal whitespace, the rest of the line (until the next carriage return or line feed) is ignored. Onramp hexadecimal files sometimes follow the address by the name of the symbol or label being defined or the string being encoded.

```asm
@0x100 "Hello"
48 65 6c 6c 6f

@0x105 main (comment comment comment)
; ...
; ...
; ...
```

Address assertions are optional. Implementations of `hex` can instead treat the entire line starting from `@` as a comment, the same as `;` and `#`.



## Trailing Backslash

A backslash `\` preceding a line feed or carriage return (i.e. at the end of a line) is forbidden. This is the case for all lines including comments and address assertions.

(The reason for this restriction is that some line-reading tools (such as the POSIX `read` command) may merge lines with trailing backslashes while others may not. This restriction removes the need to define whether a comment continues past an escaped newline.)



## Example

Given an input file like this:

```asm
; start of the file
@0x0 hello
48 65 6c 6c 6f  ; Hello
20              ; <space>

# another comment
@0x6 world
    776F726C64      # world
    21              # !

@0x0000000C
0A   ; line feed

@0x0D end
```

The hex tool converts the above to the following 13 bytes including a trailing line feed:

```
Hello world!
```

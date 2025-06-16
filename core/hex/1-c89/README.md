# C89 Hex Tool

This is the reference implementation of the hex tool. It is implemented in ANSI C for maximum portability. It performs all required error checks, generates debug info, gives nice error messages with line numbers, and does simple buffering to improve performance.

It is bootstrapped as part of a normal Onramp build so it is here in `core/`. There are many other implementations of the hex tool under [`platform/hex/`](../../../platform/hex/).

Most implementations of hex are much simpler than this. It is not necessary for a hex implementation to do all of these checks or provide this kind of error handling.

Build scripts exist in [`platform/hex/c89/`](../../../platform/hex/c89/) to build and test this for specific platforms.



## Debug Info

Pass the `-g` option to generate [Onramp debug info](../../../docs/debug-info.md). A file suffixed with `.od` will be generated alongside the output. The debug info contains symbol names taken from address assertion comments as well as line information for every hex byte.

Debug information can be used with the [debugger VM](../../../platform/vm/c-debugger/) to get stack traces on failures. This makes it much easier to debug handwritten hexadecimal bytecode. Here's an example of a stack trace while debugging [ld/0](../../ld/0-global/) with debug info enabled:

```
Invalid instruction
    0x205A8 parse_input_file() ../../../core/ld/0-global/ld.oe.ohx:874
    0x20444 perform_pass() ../../../core/ld/0-global/ld.oe.ohx:716
    0x202C4 start() ../../../core/ld/0-global/ld.oe.ohx:532
```

This is only really useful for Onramp bytecode programs such as [ld/0](../../ld/0-global/) and [hex/onramp](../../hex/0-onramp/). It is not useful for hexadecimal native binaries.

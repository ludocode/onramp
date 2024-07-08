This directory contains platform-specific scripts for building and testing the Onramp hex tool written in C89. The actual code is part of the Onramp bootstrap process; it's in [`core/hex/1-c89/`](../../../core/hex/1-c89/).

The C89 code is the reference implementation of an [Onramp Hexadecimal](../../docs/hexadecimal.md) converter. It is implemented in C89 for maximum portability. It performs all required error checks, gives nice error messages with line numbers, and does simple buffering to improve performance.

Most implementations of hex are much simpler than this. It is not necessary for a hex implementation to do all of these checks or provide this kind of error handling.

The C89 hex tool and [C89 virtual machine](../../vm/c89/) can be used to bootstrap Onramp on an old platform that only has a C89 compiler. Onramp can then be used to build a modern native compiler for that system.

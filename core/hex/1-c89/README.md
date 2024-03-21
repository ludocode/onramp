This is the reference implementation of the hex tool. It is implemented in C89 for maximum portability. It performs all required error checks, gives nice error messages with line numbers, and does simple buffering to improve performance.

It is bootstrapped as part of a normal Onramp build so it is here in `core/`. There are many other implementations of the hex tool under [`platform/hex/`](../../../platform/hex/).

Most implementations of hex are much simpler than this. It is not necessary for a hex implementation to do all of these checks or provide this kind of error handling.

Build scripts exist in [`platform/hex/c89/`](../../../platform/hex/c89/) to build and test this for specific platforms.

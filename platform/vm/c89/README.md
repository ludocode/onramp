# Onramp C89 Virtual Machine

This is a complete implementation of the Onramp VM in C89 (also known as ANSI C.) This VM supports all Onramp features (except debug info) as much as is possible on the platform.

This VM, along with the [C89 hex tool](../../hex/c89/), can be used to bootstrap Onramp on an old platform that only has a C89 compiler. Onramp can then be used to build a modern native compiler for that system.

The VM is designed to be as portable as possible so it has quite a bit of platform detection in order to provide functionality that isn't possible in standard C. For example, it supports efficient file truncation, raw input mode, and nanosecond time precision on recognized platforms.

If the platform is not recognized, it uses only standard C functions and requires only 32-bit math.

All of this portability requires a fair bit of code so the source is quite large. A minimal ANSI C VM would be less than half the length but would not support some features. (For example, non-blocking input makes it possible to play Doom from command-line, which is not possible in standard C.)

This is an implementation of the Onramp VM in C89 (also known as ANSI C.)

This, along with the [C89 hex tool](../../hex/c89/), can be used to bootstrap Onramp on an old platform that only has a C89 compiler. Onramp can then be used to build a modern native compiler for that system.

This VM supports all Onramp features (except debug info) so it is fairly large. It also has quite a bit of platform detection in order to provide functionality that isn't possible in standard C. For example, it supports efficient file truncation, raw input mode, and nanosecond time precision on recognized platforms. If the platform is not recognized, it uses only standard C functions and requires only 32-bit math.

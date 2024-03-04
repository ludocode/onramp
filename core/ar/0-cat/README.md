# Onramp Archiver - First Stage

The first stage archiver is written directly in object code. It is the first program we link using our first stage linker.

Since we don't have the means to gather the libc into a static library yet, we manually specify all of the libc object files in the build script. Once this is bootstrapped, we use it to archive the libc into a static library.

This stage only supports "rc" mode. It just concatenates each source file into the destination, prefixing each one with `%<filename>`.

```sh
ar rc <destination> <source> [source...]
```

These metadata lines are ignored by the first stage linker but they become important later. Our second stage linker supports file scope, and our C compiler emits generated labels and symbols with `static` linkage. These features require metadata to delimit object files in static archives.

We don't need any other features to bootstrap Onramp so this stage is used right up until we bootstrap the final toolchain. We do have a second stage archiver but it provides features for building things with Onramp rather than for bootstrapping Onramp.

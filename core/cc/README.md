# Onramp Driver

This is the Onramp C compiler driver. It is a single stage written in [Onramp Minimal C (omC)](../docs/minimal-c.md).



## Phases of Translation

The `cc` tool can perform any or all of the following transformations. When performing multiple phases, temporary files are used to pass data along from one tool to the next.

| Phase         | Tool            | Operation                | Command-line option |
|---------------|-----------------|--------------------------|---------------------|
| Preprocessing | [`cpp`](../cpp) | `.c` -> `.i`             | `-E`                |
| Compilation   | [`cci`](../cci) | `.i` -> `.os`            | `-S`                |
| Assembly      | [`as`](../as)   | `.os` -> `.oo`           | `-c`                |
| Linking       | [`ld`](../ld)   | `.oo` + `.oa` -> (exec.) | none (default)      |

The output of linking is an Onramp bytecode executable, optionally wrapped for the user's platform, that can be run on the Onramp virtual machine.



## Bootstrapping

By default, the compiler driver expects to use the final bootstrapped versions of the underlying tools and libc. During bootstrapping, these are not available yet. The tools to use can be overridden using the following command-line options:

- `-with-cpp=<path/to/cpp.oe>` -- Sets the path to the preprocessor
- `-with-cci=<path/to/cci.oe>` -- Sets the path to the compiler
- `-with-as=<path/to/as.oe>` -- Sets the path to the assembler
- `-with-ld=<path/to/ld.oe>` -- Sets the path to the linker
- `-nostdlib` -- Prevents automatically linking against the libc
- `-nostdinc` -- Prevents automatically passing the libc headers to the preprocessor
- `-nostddef` -- Prevents defining any default macros for the preprocessor

The bootstrapping scripts provide these options as needed to every invocation of the driver until the final tools are complete.

You can also use these options if you'd like to use an alternate tool for any of the phases of compilation. Note that they must be Onramp executables (not native executables) and they must be compatible with the command-line options that will be provided by the driver.

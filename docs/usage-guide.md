# Onramp Usage Guide

This guide explains how to use Onramp after you have built it.

Onramp builds a bytecode program called `cc`. In a hosted environment, it also provides a platform-specific wrapper called `onrampcc`. This has a similar interface to the user-facing executable of other UNIX compilers such as `gcc`, `clang`, and so on.

This program runs the phases of translation, which are implemented as individual tools in Onramp: the preprocessor, the compiler, the assembler and the linker. You do not need to run these tools yourself to use Onramp. You can just use `onrampcc`.

```sh
onrampcc foo.c -o foo
```

If you don't want to perform all stages of translation at once, you should still use `onrampcc`, but with a `-E`, `-S` or `-c` option to limit the stages it will run. For example:

```sh
onrampcc -c file1.c -o file1.oo
onrampcc -c file2.c -o file2.oo
onrampcc -c file3.c -o file3.oo
onrampcc file1.oo file2.oo file3.oo -o foo
```

See the command-line options below.

By default `onrampcc` produces binaries wrapped for your platform. The above `foo` file can be run like a normal executable, except that it requires `onrampvm` on your PATH.



## Phases of Translation

The `onrampcc` tool can perform any or all of the following transformations. When performing multiple phases, temporary files are used to pass data along from one tool to the next.

| Phase         | Operation                     | Command-line option |
|---------------|-------------------------------|---------------------|
| Preprocessing | `.c` -> `.i`                  | `-E`                |
| Compilation   | `.i` -> `.os`                 | `-S`                |
| Assembly      | `.os` -> `.oo`                | `-c`                |
| Linking       | `.oo` + `.oa` -> (executable) | none (default)      |

The output of linking is an Onramp bytecode executable, optionally wrapped for your platform, that can be run on the Onramp virtual machine.



## Command-Line Options

The command-line syntax for the Onramp compiler driver is:

```
onrampcc [options...] <input> [input...] -o <output>
```

The `-o` option is used to specify the output file. It is currently required.


### Mode

By default `onrampcc` operates in linking mode. In this mode it can take multiple files as input, each of which can be any of the supported input types (C, preprocessed C source, assembly, object files and static library archives.) It will perform all phases of translation as needed to translate each input file to an object file, then link the objects together to produce an executable.

You can instead limit the phases it will perform with these options:

- `-E` -- Stop after preprocessing. The output is preprocessed C (`.i`).
- `-S` -- Stop after compilation. The output is Onramp assembly (`.os`).
- `-c` -- Stop after assembly. The output is an Onramp object file (`.oo`).

In each of these modes, the input must consist of a single file, which must be at an earlier phase of translation than the option given (for example you cannot preprocess an assembly file.)

### Language standard:

The Onramp compiler supports several versions of the C standard, along with some extensions implemented by popular compilers.

- `-std=<version>` -- Sets the language standard to compile. Supported versions:
    - `c89`, `c99`, `c11`, `c17` (default), `c23` -- The corresponding version of standard C
    - `gnu89`, `gnu99`, `gnu11`, `gnu17`, `gnu23` -- GNU C; implies `-fgnu-extensions`
- `-ansi` -- An alias of `-std=c89`

Onramp can emulate GNU C and other compilers but this is not on by default. Extensions must be manually enabled.

- `-fgnu-extensions` -- Enables GNU extensions and defines `__GNUC__`
- `-fms-extensions` -- Enables Microsoft Visual C++ extensions
- `-fplan9-extensions` -- Enables Plan 9 extensions

### Other Options

Output format:

- `-g` -- Emit debug info. When linking, this produces a matching file with extension `.od`.
- `-O` -- Perform optimizations when compiling and linking.

Preprocessor options:

- `-Dname` -- Define a macro with an empty expansion.
- `-Dname=expansion` -- Define a macro with the given expansion.
- `-I/path/to/includes` -- Add a search path for `#include` and `#embed`.
- `-include /path/to/header.h` -- Include the given header at the start of preprocessing.

Tool and library overrides:

- `-with-cpp=/path/to/cpp.oe` -- Use an alternate preprocessor.
- `-with-cci=/path/to/cci.oe` -- Use an alternate compiler.
- `-with-as=/path/to/as.oe` -- Use an alternate assembler.
- `-with-ld=/path/to/ld.oe` -- Use an alternate linker.
- `-nostdinc` -- Do not use any default include paths (e.g. to the Onramp libc.)
- `-nostdlib` -- Do not link any libraries by default (e.g. the Onramp libc.)
- `-nostddef` -- Do not define any macros by default (e.g. `__onramp__`.)

Miscellaneous options:

- `-v` -- Verbose mode; prints the sub-commands to be executed.
- `-###` -- Dry-run mode; does not run any sub-commands. Implies `-v`.
- `-dM` -- Print defined macros after preprocessing. Requires `-E`.

Internal options:

- `-wrap-header=/path/to/header` -- Use the given executable wrapper for a hosted platform.



## Environment Variables

- `TMPDIR` -- The path to store temporary files (default `/tmp` on POSIX systems.)



## Pre-defined macros

The Onramp compiler pre-defines several macros by default. Here's a list of user-facing Onramp-specific macros and their purposes:

- `#define __onramp__ 1` -- The compiler is targeting an Onramp VM and emits Onramp assembly or bytecode.
- `#define __onramp_cci__ 1` -- This is the Onramp compiler.
- `#define __onramp_cci_full__ 1` -- This is the final stage of the Onramp compiler.
- `#define __onramp_cpp__ 1` -- This is the Onramp preprocessor.
- `#define __onramp_cpp_full__ 1` -- This is the final stage of the Onramp preprocessor.
- `#define __onramp_libc__ 1` -- The libc headers are those of the Onramp libc, and the program will be linked with it.
- `#define __onramp_libc_full__ 1` -- This is the final stage of the Onramp libc.

It's important to use the correct macros. For example if you want to detect the Onramp compiler to work around some issue, you should detect `__onramp_cci__`, not `__onramp__`. If you want special behaviour on an Onramp VM, you should use `__onramp__`, not `__onramp_cci__`. If you are making a backend for some other compiler that emits Onramp assembly or bytecode, you should define only `__onramp__`, and potentially `__onramp_libc__` and `__onramp_libc_full__` if you are also targeting the Onramp libc.

(NOTE: Onramp is currently under development. Since the full Onramp stages are not yet complete, the currently defined stage macros are actually of earlier stages.)

You can view the predefined macros using `-dM`, for example:

```sh
touch /tmp/onramp-blank.c && onrampcc -dM -E /tmp/onramp-blank.c -o /dev/null | sort
```

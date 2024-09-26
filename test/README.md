# Testing Onramp

Onramp is designed to be tested in a POSIX development environment. The test suite depends on typical POSIX development tools:

- a native C compiler (not Onramp);
- a `make` tool (GNU or BSD);
- a POSIX shell and coreutils;
- a `/tmp` directory;
- and probably some other assumptions not listed here.

There are two ways to test components: using the *best* available toolchain, and using the *boot*strapping toolchain. Both ways are important to fully test Onramp.



## Pre-Testing Setup

Consider doing a clean before testing:

```sh
scripts/posix/clean.sh
```

You'll need to have a VM and hex tool on your PATH. The simplest way to do that is:

```sh
scripts/posix/build.sh --dev --setup
. scripts/posix/env.sh
```

A better option if you plan to do Onramp development is to install developer symlinks. This will place symlinks in `~/.local/bin` for `onrampvm`, `onrampcc` and so on that point directly into your build folder. This allows you to easily test local changes without having to reinstall Onramp.

```sh
scripts/posix/build.sh --dev
scripts/posix/install.sh --dev
```

The file `test/local.mk.sample` can be copied to `test/local.mk` to set variables for all test Makefiles. By default it adds `-Werror` and Address Sanitizer.

You could add to or override other things as well, for example `CC` to use a particular compiler.

It is highly recommended to use `local.mk` to at least enable `-Werror`. If you get warnings with any compiler, please file a bug against Onramp.



## Testing Everything

If you just want to run all tests that can be run on your system, do the above setup, then run:

```sh
test/test-all.sh
```

This approximates everything the GitHub CI does. None of it is parallelized (yet) so it will take tens of minutes to complete.



## Testing components using the best available toolchain

The test directory for each stage of each component contains within it a `Makefile`. This builds the component (and any dependencies) using the best available toolchain and runs its unit test suite.

Tools written in C, or an Onramp subset of C, will be built with your system C compiler. This gives the best warnings and error detection, and allows us to use tools like Address Sanitizer to verify our components for correctness.

Tools written in Onramp assembly, bytecode or hexadecimal will be built with the final stage of the corresponding Onramp tools. This again gives the best warnings and error detection. Final stage Onramp tools are much better at error detection and reporting than earlier bootstrapping stages.



### Testing one component

You can build an individual component simply by going into its test folder and typing `make`. For example:

```sh
cd test/as/2-full
make
```

This builds the full assembler with your C compiler and runs all tests. This is how you would normally build and test a component while doing development on it. If you only want to build, type `make build`.

When a test case fails, the script prints the commands used to build and run that test case. You can copy and paste those commands to rebuild and run only that test case.

Type `make generate` to re-generate the output of all test cases. Use this if you change the output formatting of the tool, or if you change a libc header for example, which could invalidate the output of many tests due to minor changes (such as `#line` numbers being different.) This renders the test results moot! You need to double-check the diff carefully to make sure it has made only the changes you want to the output.



### Testing all core components

Core components can be tested with the following script:

```sh
test/test-core.sh
```

This just runs `make` on every stage of every core component.



## Testing components as bootstrapped

Core components can alternatively be tested with the following script:

```sh
test/test-bootstrap.sh
```

This builds Onramp using the normal bootstrap process, testing each stage of each tool after it is built.

This is a more "real" test of how the tools will actually be built when Onramp is bootstrapped: it can detect errors in the bootstrap process that the normal `make`-based tests may not. It can also detect the use of later-stage features that are not available in earlier stages. However, it cannot detect many errors that the final stage tools, modern compilers, and tools like Address Sanitizer can detect.

If you want to test only a single stage as bootstrapped, you can copy-paste the corresponding line from this script. If you want to test only the final toolchain after the bootstrap process is complete, you can test it like this:

```sh
test/test-final.sh
```

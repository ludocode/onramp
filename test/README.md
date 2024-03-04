# Testing Onramp

Onramp is designed to be tested in a POSIX development environment. The test suite depends on typical POSIX development tools:

- a C compiler;
- a `make` tool (GNU or BSD);
- a POSIX shell and coreutils;
- a `/tmp` directory;
- and probably some other assumptions not listed here.

There are two ways to test components: using the *best* available toolchain, and using the *boot*strapping toolchain. Both ways are important to fully test Onramp.



## Testing components using the best available toolchain

This is the typical way to test components.

The test directory for each stage of each component contains within it a `Makefile`. This builds the component (and any dependencies) using the best available toolchain and runs its unit test suite.

Tools written in C, or an Onramp subset of C, will be built with your system C compiler. This gives the best warnings and error detection, and allows us to use tools like Address Sanitizer to verify our components for correctness.

Tools written in Onramp assembly, bytecode or hexadecimal will be built with the final stage of the corresponding Onramp tools. This again gives the best warnings and error detection. Later stage Onramp tools are much better at error detection and reporting than earlier stage tools.



### Testing one component

You can build an individual component simply by going into its test folder and typing `make`. Alternatively, you can do it from the top-level with the `-C` option. For example, try:

```sh
make -C test/as/2-full
```

This builds the full assembler with your C compiler and runs all tests. Note that any paths reported in the output are relative to the test folder, not the root.



### Testing all core components

Core components can be tested with the following script:

```sh
test/test-core.sh
```

This just runs `make` on every stage of every core component.



### Local Settings

The file `test/local.mk.sample` can be copied to `test/local.mk` to set variables for all test Makefiles. By default it adds `-Werror` and Address Sanitizer.

You could add to or override other things as well, for example `CC` to use a particular compiler.

It is highly recommended to enable `-Werror`. If you get warnings with any compiler, please file a bug against Onramp.



## Testing components as bootstrapped

Core components can alternatively be tested with the following script:

```sh
test/test-bootstrap.sh
```

This builds Onramp using the normal bootstrap process, testing each stage of each tool after it is built.

This is a more "real" test of how the tools will actually be built when Onramp is bootstrapped: it can detect errors in the bootstrap process that the normal `make`-based tests may not. It can also detect the use of later-stage features that are not available in earlier stages.

However, it cannot detect many errors that the final stage tools, modern compilers, and tools like Address Sanitizer can detect.

If you want to test only a single stage as bootstrapped, you can setup your environment and then copy-paste the corresponding line from this script.

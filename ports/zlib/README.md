# zlib

The zlib library builds on Onramp with no changes. On a POSIX system you can simply point it to the Onramp wrapper scripts:

```sh
CC=onrampcc AR=onrampar ./configure && make
```

This requires a POSIX shell and a make tool. The above has been tested with zlib 1.3.2. The configure script correctly determines that shared libraries are unavailable; it builds only a static library `libz.a`, the example programs (including `minigzip`), and the tests.

Run `make test` to test it. All tests should pass.

The configure script and Makefile appear to be handwritten, but use of them requires bootstrapping a POSIX shell and GNU make. It may be worth investigating whether the source files can be built manually (perhaps with an Onramp shell script.)

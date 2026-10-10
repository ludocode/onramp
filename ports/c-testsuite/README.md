# c-testsuite

The [c-testsuite](https://github.com/c-testsuite/c-testsuite) database can be run unmodified under Onramp using its posix runner.

It can be run like this:

```sh
CC=onrampcc CFLAGS=-O ./single-exec posix | tee c-testsuite.log | grep -v '^#'
```

Optimization is recommended because test 00040.c is pretty slow without it. The `tee` command stores the output to a file in case of errors and the grep command filters it to something readable.

Alternatively, if you have c-testsuite checked out alongside Onramp, you can simply run `ports/c-testsuite/run.sh`.

Issue #18 is tracking test suite failures. There are still 28 failures out of 220 as of this writing.

You may encounter terminal weirdness if you're using a VM with raw input support. It was exceedingly difficult to get this right so there are probably still bugs; see #11 for details. If you notice an issue, please file a bug report.

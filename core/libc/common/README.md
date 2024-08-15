# Onramp C Standard Library -- Headers

This directory contains the libc headers along with a script to copy them to the build output.

The libc headers are shared between all stages of Onramp. The preprocessor is used to hide portions of the libc that cannot be compiled on earlier stages.

In particular, we need to hide function pointers and unsupported types (`float`, `double`, `long long`) under [cci/1](../../cci/1-opc); we additionally need to hide `short`, `unsigned`, `struct`, variadic functions and more under [cci/0](../../cci/0-omc); and we need to hide or provide fallbacks for function-like macros (such as `assert()`) under [cpp/1](../../cpp/1-omc). We therefore test the following macros to hide functionality:

- `__onramp_cpp_omc__`
- `__onramp_cci_omc__`
- `__onramp_cci_opc__`

These macros are defined by the build scripts during bootstrapping. They also define the current libc stage which is usually one of these:

- `__onramp_libc_oo__`
- `__onramp_libc_omc__`
- `__onramp_libc_opc__`

We generally do not bother to hide unavailable libc functionality as long as the prototype can be parsed. Attempting to use an unavailable function will lead to a linker error.

When Onramp is fully bootstrapped, none of the above are defined.

The headers must support preprocessing with cpp/1 which supports `#ifdef` and `#ifndef` but not `#if`. Tests for the full C compiler therefore look like this:

```c
#ifndef __onramp_cci_omc__
#ifndef __onramp_cci_opc__
// functions that use float, double, etc.
#endif
#endif
```

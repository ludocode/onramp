# TinyCC

TinyCC can be built with Onramp. The resulting TinyCC runs on the Onramp VM and outputs native binaries. It is capable of producing a native build of itself.

These instructions are tested on commit 32b597746c of TinyCC mob dated 2025-06-24 on a Linux x86_64 system. It will probably work on other versions and other platforms, but it may require additional changes.

A patch is currently required to build Onramp. It mostly contains hacks to disable features that aren't yet supported. This patch must not be submitted upstream; it should shrink over time as we implement more missing features. Apply it like this:

```sh
patch -p0 </path/to/0001-TinyCC-hacks-to-support-Onramp.patch
```



## POSIX

On a POSIX system, TinyCC can be built with the included configure script and Makefile.

The configure script and Makefile appear to be handwritten, but use of them requires a POSIX shell and GNU make. We will eventually need to handwrite a `config.h` and an Onramp shell script to build TinyCC without these dependencies.

Onramp does not support dynamic libraries or pthreads so these options must be disabled in the configuration. With Onramp on your PATH, configure it like this:

```sh
CC=onrampcc AR=onrampar ./configure \
    --prefix="$(pwd)/install-stage0" \
    --enable-static \
    --config-ldl=no \
    --config-pthread=no
make
make install
```

(Note that we are not using `--enable-cross`. It should be used but it doesn't work well in this commit of TinyCC.)

This will take some time. Onramp is currently glacially slow. By point of comparison, TinyCC compiled by Onramp running on an Onramp VM is about 15x faster than the self-bootstrapped Onramp compiler. A native TinyCC compiled with itself is 15x-100x faster still.

The resulting compiler is `install-stage0/bin/tcc`. This runs on the OnrampVM and outputs native binaries. (It also requires a native compiled libc and its headers in order to produce normal programs. It is probably capable of bootstrapping a libc but Onramp is not there yet; see [Freestanding](#freestanding) below.)

The bootstrapped TinyCC is capable of building itself. Follow the above with:

```sh
make clean
rm -f config.*
CC="$(realpath install-stage0/bin/tcc)" \
    AR="$(realpath install-stage0/bin/tcc) -ar" \
    ./configure \
    --prefix="$(pwd)/install-stage1"
make
make install
```

The result is `install-stage1/bin/tcc`. This is a native TinyCC; it no longer depends on an Onramp VM. Add it to your PATH like so:

```
export PATH=$(pwd)/install-stage1/bin:$PATH
```

A third verification stage can be built in exactly the same way as above:

```sh
make clean
rm -f config.*
CC="$(realpath install-stage1/bin/tcc)" \
    AR="$(realpath install-stage1/bin/tcc) -ar" \
    ./configure \
    --prefix="$(pwd)/install-stage2"
make
make install
```

If you compare `install-stage2/bin/tcc` to `install-stage1/bin/tcc`, you will notice that they are different. The build is not currently reproducible. A future goal of Onramp is to make the verification stages match.



## Freestanding

A freestanding bootstrap of TinyCC is not ready yet. Once Onramp bootstraps in freestanding, TinyCC should be straightforward as well; it just requires handwriting a `config.h` and an Onramp shell build script. The resulting TinyCC should be able to build a native libc and then build binaries against it.

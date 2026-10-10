# Doom

Onramp does not support graphics so a console port of Doom has been written called [doom-cli](https://github.com/ludocode/doom-cli).

It requires an Onramp VM that can do raw input. The C89 VM no longer has raw input support so you have to use either the x86\_64-linux VM or the debugger. If you just want to test it out, I recommend building Onramp like this:

```sh
./configure --native --dev
./build.sh
. ./env.sh
```

You can then build doom-cli with Onramp like this:

```sh
cd doomgeneric
CC=onrampcc ./build-cli.sh
```

Assuming it built correctly, run it like this:

```sh
./doomgeneric -iwad /path/to/DOOM.WAD
```

Read the doom-cli [README](https://github.com/ludocode/doom-cli/blob/master/README.md) for controls and options. There is also a [blog post](https://ludocode.com/blog/onramp-can-compile-doom) with more details.

Raw input is notoriously difficult to get right especially in a machine code VM. If you encounter any terminal weirdness, please file a bug against Onramp.

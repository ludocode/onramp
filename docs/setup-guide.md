# Onramp Setup Guide

This guide explains how to build and configure Onramp.

The Onramp bootstrap process is platform-independent, but it requires a platform-specific Onramp VM and hex tool to be configured first.

Scripts exists for automatically building Onramp on certain hosted platforms. If you're not using one of the supported platforms, skip ahead to the "Manual Hosted Setup" section.



## POSIX setup

### Building for POSIX

Onramp includes build scripts for POSIX systems in `scripts/posix/`. Build Onramp like this:

```sh
scripts/posix/build.sh
```

If there is a native machine code VM for your platform (e.g. `linux-x86_64/`), the scripts will use it. This is a true bootstrap process in which the only non-firmware trust seeds are your kernel and coreutils. (On Linux it doesn't even need a libc, so you could in theory bootstrap Onramp with nothing but a statically linked BusyBox.)

(If you don't trust your kernel, you'll need to bootstrap Onramp in freestanding. See the Freestanding section below.)

If there isn't a machine code VM for your platform, the script will attempt to compile a VM or use an interpreted one. If it cannot find a VM that works on your system, the script will fail.

Obviously if it uses a C compiler to build a VM, it didn't really bootstrap a C compiler from scratch. This may be fine if you're just trying to get Onramp working on an older system that just has an ANSI C compiler. If you want to eliminate compilers from your trusted seeds, you'll need a real machine code VM.

The build script supports the following command-line options to choose a specific hex tool and VM and to otherwise change its behaviour. (Pass no options to use the default auto-detection.)

- `--hex [name]` -- Use the hex tool with the given name
- `--vm [name]` -- Use the VM with the given name
- `--dev` -- Use preferred tools for developing Onramp
- `--min` -- Use only tools with no additional dependencies (i.e. a machine code VM), fail otherwise
- `--skip-core` -- Skip the core bootstrap; just do the POSIX setup

For example, to use the fastest VM and hex tool (requiring a native C compiler):

```sh
scripts/posix/setup.sh --hex c89 --vm c89
```

For developing Onramp (requiring a native C compiler and make tool):

```sh
scripts/posix/setup.sh --dev
```

### Installing for POSIX

Once Onramp is bootstrapped, you can install it into `~/.local` like this:

```sh
scripts/posix/install.sh
```

Assuming `~/.local/bin` is on your PATH, you can then use `onrampcc` and run the programs it compiles normally. (I suppose you could also install it into `/usr/local` but I have no idea why anyone prefers that to `~/.local` so the script doesn't have an option for it.)

You can pass `--dev` to the install script to install symlinks to your build folder instead of copying files.

If you don't want to install Onramp, you can instead use it in-place by sourcing `env.sh` like this:

```sh
. scripts/posix/env.sh
```

This adds the build location (`build/posix/bin/`) to your PATH. You'll need this location on your PATH to run any of the compiled programs because they depend on `onrampvm`.



## Windows

Windows support is not implemented yet.

Eventually I will write a VM for Windows in machine code in a PE container to make this truly bootstrappable. Since the `x86_64-linux` VM is already written this should be pretty straightforward.

In the meantime it is probably relatively easy to get the Python or C89 VMs running on Windows. The main thing they need is to translate paths from Windows style (e.g. `C:\some\path`) to Onramp style (e.g. `/c/some/path`). This is not done yet.

From there it would be possible to use it to run the Onramp build script, except the Onramp shell is not written yet either. If you also use a POSIX shell to run it (e.g. Cygwin or MSYS2), you might be able to get this working on Windows. I haven't bothered to try this yet.



## Manual Hosted Setup

Suppose you are a future archaeologist that discovers an ancient data store in a fallout shelter centuries after a devastating nuclear war. Or, suppose you are an extra-terrestrial xenolinguist that receives a mysterious data burst in a radio signal originating in deep space.

The data contains highly compressed multimedia about then-contemporary human civilization, along with the source code for decoder programs to view it. Unfortunately, these programs are written in this cryptic programming language called C.

Luckily, if the data also includes Onramp, you can use it to compile the decoders! But first you'll need to get Onramp bootstrapped on your alien hardware.

To setup Onramp manually in an arbitrary (hosted) environment, you need to create two tools:

- a [hex tool](../platform/hex/) that can convert [Onramp hexadecimal](hexadecimal.md) to raw bytes; and
- a [virtual machine](../platform/vm/) that implements the [Onramp VM](virtual-machine.md) spec.

These tools can be written in whatever programming language you have available on your alien computer. Use the example programs in `platform/` (in particular the Python implementations) as a reference, and use the tests in `test/hex/` and `test/vm/` to verify that your implementation is correct. (You'll need to write your own scripts for your alien computer if you want to run them in an automated fashion.)

The hosted setup process assumes you can provide Onramp with a filesystem. You'll need to implement the filesystem syscalls, either bridging whatever filesystem you have or implementing an Onramp-style filesystem on top of your alien storage device.

Once that is done, you need to use your hex tool to convert two Onramp programs: the Onramp hex tool and the Onramp shell. The output files must be stored in the following paths (relative to the root of the Onramp source):

- [`core/hex/0-onramp/hex.oe.ohx`](../core/hex/0-onramp/hex.oe.ohx) --> `build/intermediate/hex-0-onramp/hex.oe`
- [`core/sh/sh.oe.ohx`](../core/sh/sh.oe.ohx) --> `build/intermediate/sh/sh.oe`

After that, run the Onramp shell in your VM, giving it the core build script ([`core/build.sh`](../core/build.sh)) as its only argument. Like this:

```sh
onrampvm build/intermediate/sh/sh.oe core/build.sh
```

Again, all paths are relative to the root of the Onramp source, and the working directory must be this path as well.

If your VM and hex implementations are correct, this should bootstrap Onramp.

WARNING: The Onramp shell tool is incomplete. In the meantime you need to run `core/build.sh` in a POSIX shell with `onrampvm` on your PATH. This will be fixed soon.

The results of bootstrapping are in `build/output/`. The compiler driver is `bin/cc.oe`, which you can run in your Onramp VM like this:

```sh
onrampvm build/output/bin/cc.oe
```

For example, to compile and run a program `foo.c`:

```sh
onrampvm build/output/bin/cc.oe foo.c -o foo.oe
onrampvm foo.oe
```

If you want to improve integration with your operating system, you can provide wrapper programs for the VM, compiler driver and other tools. You can also make the compiler produce wrapped binaries by providing it with a `-wrap-header`. Take a look at [`platform/cc/`](../platform/cc/) to see how other platforms do it.



## Freestanding

Onramp is designed to be able to eventually bootstrap itself without depending on an existing operating system. This is not implemented yet. See the [`x86-bios` VM](../platform/vm/x86-bios/) and the [Onramp `os`](../core/os/) for descriptions of how this will hopefully work.

Note that even if the OS is eliminated from the bootstrap path, modern computers still have huge amounts of code in their firmware. The implementation of [UEFI](https://en.wikipedia.org/wiki/UEFI) in your PC is probably hundreds of thousands of lines of compiled code. Modern CPUs have embedded autonomous processors such as the [Intel Management Engine](https://en.wikipedia.org/wiki/Intel_Management_Engine) which contain secret firmware and potential backdoors. Virtually all modern peripherals contain microcontrollers with onboard firmware. Some phone chargers have more program storage than the Apollo Guidance Computer that landed on the moon.

Unless you are building your entire computer yourself out of transistors, you are probably stuck trusting large amounts of proprietary code. Solving the firmware trust issue is outside the scope of Onramp.

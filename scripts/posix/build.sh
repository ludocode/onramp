#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2023-2024 Fraser Heavy Software
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.



# This is the Onramp automatic build script for POSIX systems. It will choose
# an appropriate hex tool and virtual machine for your system and use them to
# perform the bootstrap process.
#
# This script supports several command-line options. Pass --help to see them.
# They are also documented in the setup guide.
#
# See docs/setup-guide.md for more on how to use this script.
# See docs/bootstrap-path.md to follow how Onramp is actually built.



set -e
cd "$(dirname "$0")/../.."



#######################################################
# Hex tool setup
#######################################################

# Each function builds a hex tool (if necessary) and runs a simple test to make
# sure it works.
#
# We prefer to use the POSIX shell tool in non-dev mode to provide a genuine
# hosted bootstrap. We don't bother to build any native hex tools here since
# they require an existing hex tool to bootstrap themselves. This script is
# written in POSIX shell so we can just use the POSIX shell hex tool.
#
# We do allow selecting other tools though and we default to the C89 tool under
# --dev because most of them are faster than the POSIX shell tool and some
# provide better error messages.
#
# The hex tool is not actually installed into a standard place in build/posix/
# because it's not necessary; the bootstrap process builds a hex tool that runs
# on the Onramp VM. Compiled hex tools are built into their own directories
# under build/test/ and intepreted hex tools are run directly from the source.

HEX=
HEX_TEST_ARGS="test/hex/tests/hello-world-readme.ohx -o /dev/stdout"
HEX_RESULT="Hello world!"

setup_hex_c89() {
    echo "Checking c89/ hex tool"
    # Try a build with the default CFLAGS first. If that fails, try a build
    # with no CFLAGS in case the compiler doesn't support the defaults.
    echo "Checking c89/ VM"
    if ( platform/hex/c89/build.sh 2>&1 >/dev/null ) ||
            ( CFLAGS= platform/hex/c89/build.sh 2>&1 >/dev/null ); then
        if [ "$(build/test/hex-c89/hex $HEX_TEST_ARGS 2>/dev/null)" = "$HEX_RESULT" ]; then
            echo "Using c89/ hex tool"
            HEX=build/test/hex-c89/hex
        fi
    fi
}

setup_hex_python_golf() {
    echo "Checking python-golf/ hex tool"
    if [ "$(platform/hex/python-golf/hex.py $HEX_TEST_ARGS 2>/dev/null)" = "$HEX_RESULT" ]; then
        echo "Using python-golf/ hex tool"
        HEX=platform/hex/python-golf/hex.py
    fi
}

setup_hex_python() {
    echo "Checking python/ hex tool"
    if [ "$(platform/hex/python/hex.py $HEX_TEST_ARGS 2>/dev/null)" = "$HEX_RESULT" ]; then
        echo "Using python/ hex tool"
        HEX=platform/hex/python/hex.py
    fi
}

setup_hex_sh_alt() {
    # These are fallbacks in case the default sh/ tool doesn't work (e.g. if
    # the shell doesn't support arithmetic expressions or parameter expansion.)
    # One of these might work instead.
    echo "Checking sh-alt/ hex tool"
    if [ "$(platform/hex/sh-alt/hexcomb.sh $HEX_TEST_ARGS 2>/dev/null)" = "$HEX_RESULT" ]; then
        echo "Using sh-alt/hexcomb.sh hex tool"
        HEX=platform/hex/sh-alt/hexcomb.sh
    elif [ "$(platform/hex/sh-alt/hexsemi.sh $HEX_TEST_ARGS 2>/dev/null)" = "$HEX_RESULT" ]; then
        echo "Using sh-alt/hexsemi.sh hex tool"
        HEX=platform/hex/sh-alt/hexsemi.sh
    elif [ "$(platform/hex/sh-alt/hexsplit.sh $HEX_TEST_ARGS 2>/dev/null)" = "$HEX_RESULT" ]; then
        echo "Using sh-alt/hexsplit.sh hex tool"
        HEX=platform/hex/sh-alt/hexsplit.sh
    fi
}

setup_hex_sh() {
    echo "Checking sh/ hex tool"
    if [ "$(platform/hex/sh/hex.sh $HEX_TEST_ARGS 2>/dev/null)" = "$HEX_RESULT" ]; then
        echo "Using sh/ hex tool"
        HEX=platform/hex/sh/hex.sh
    fi
}

setup_hex_xxd() {
    echo "Checking xxd/ hex tool"
    if [ "$(platform/hex/xxd/hex.sh $HEX_TEST_ARGS 2>/dev/null)" = "$HEX_RESULT" ]; then
        echo "Using xxd/ hex tool"
        HEX=platform/hex/xxd/hex.sh
    fi
}

autodetect_hex() {

    # When auto-detecting in non-dev mode we prefer to use the POSIX shell
    # tool. It's very slow but it has (nearly) complete error detection, it
    # supports address assertions, and most importantly it doesn't add any
    # additional dependencies to the bootstrap process.
    if [ "x$HEX" = "x" ]; then
        setup_hex_sh
    fi

    # If we have a C compiler we'll try the C89 hex tool since it's much faster
    # than interpreted tools and still provides lots of error checking.
    #
    # Obviously it sort of defeats the purpose to use a C compiler to bootstrap
    # Onramp, but this can still be useful for example on an old system that
    # only has a C89 compiler which couldn't directly compile most modern
    # compilers.
    if [ "x$HEX" = "x" ]; then
        setup_hex_c89
    fi

    # If none of the above worked, maybe we have a very limited or buggy shell
    # and not much else in the environment. We try a few other interpreted
    # tools instead. We prefer tools with better error checking, minimal
    # dependencies, and better performance.
    if [ "x$HEX" = "x" ]; then
        setup_hex_python
    fi
    if [ "x$HEX" = "x" ]; then
        setup_hex_xxd
    fi
    if [ "x$HEX" = "x" ]; then
        setup_hex_python_golf
    fi
    if [ "x$HEX" = "x" ]; then
        setup_hex_sh_alt
    fi

    if [ "x$HEX" = "x" ]; then
        echo "ERROR: No hex tool was found for this platform."
        exit 1
    fi
}

autodetect_min_hex() {
    # We support only the POSIX shell hex tools under --min. Any other tool
    # would require an additional dependency.
    if [ "x$HEX" = "x" ]; then
        setup_hex_sh
    fi
    if [ "x$HEX" = "x" ]; then
        setup_hex_sh_alt
    fi
    if [ "x$HEX" = "x" ]; then
        echo "ERROR: No dependency-free hex tool was found for this platform."
        exit 1
    fi
}

choose_hex() {
    if [ "$1" = "c89" ]; then
        setup_hex_c89
    elif [ "$1" = "python-golf" ]; then
        setup_hex_python_golf
    elif [ "$1" = "python" ]; then
        setup_hex_python
    elif [ "$1" = "sh-alt" ]; then
        setup_hex_sh_alt
    elif [ "$1" = "sh" ]; then
        setup_hex_sh
    elif [ "$1" = "xxd" ]; then
        setup_hex_xxd
    else
        echo "ERROR: No POSIX setup process exists for requested hex tool: $1"
        exit 1
    fi

    if [ "x$HEX" = "x" ]; then
        echo "ERROR: The requested hex tool could not be configured for this platform."
        exit 1
    fi
}



#######################################################
# VM setup
#######################################################

# Each function builds a VM (if necessary), runs a simple test to make sure it
# works, and copies it into `build/posix/`.
#
# The VM is copied into `build/posix/share/onramp/platform/` and a symlink or
# wrapper is provided at `build/posix/bin/onrampvm`. This is the VM that is
# used for the bootstrap process and for running compiled user programs, and
# it's the VM that is ultimately installed by the install script.
#
# We try to auto-detect a native machine code VM to minimize dependencies and
# provide a genuine hosted bootstrap. If none is available we fallback to a
# compiled or interpreted VM.
#
# In dev mode we use the debugger.

VM_PATH=build/posix/bin/onrampvm
VM_TEST=build/test/hello.oe
VM_RESULT="Hello world!"

init_vm() {
    rm -f $VM_TEST
    mkdir -p build/test/
    $HEX test/vm/io/hello.oe.ohx -o $VM_TEST
}

setup_vm_c89() {
    # Try a build with the default CFLAGS first. If that fails, try a build
    # with no CFLAGS in case the compiler doesn't support the defaults.
    echo "Checking c89/ VM"
    if ( platform/vm/c89/build.sh 2>&1 >/dev/null ) ||
            ( CFLAGS= platform/vm/c89/build.sh 2>&1 >/dev/null ); then
        if [ "$(build/test/vm-c89/vm $VM_TEST 2>/dev/null)" = "$VM_RESULT" ]; then
            echo "Using c89/ VM"
            cp build/test/vm-c89/vm build/posix/share/onramp/platform/vm-x89
            (cd build/posix/bin; ln -s ../share/onramp/platform/vm-x89 onrampvm)
        fi
    fi
}

setup_vm_c_debugger() {
    # The debugger build relies on make.
    echo "Checking c-debugger/ VM"
    if make -C platform/vm/c-debugger build 2>&1 >/dev/null; then
        if [ "$(build/test/vm-c-debugger/vm $VM_TEST 2>/dev/null)" = "$VM_RESULT" ]; then
            echo "Using c-debugger/ VM"
            cp build/test/vm-c-debugger/vm build/posix/share/onramp/platform/vm-c-debugger
            (cd build/posix/bin; ln -s ../share/onramp/platform/vm-c-debugger onrampvm)
        fi
    fi
}

setup_vm_binary() {
    echo "Checking $1/ VM"
    mkdir -p build/intermediate/vm-$1
    rm -f build/intermediate/vm-$1/vm
    if $HEX platform/vm/$1/vm.ohx -o build/intermediate/vm-$1/vm 2>&1 >/dev/null; then
        chmod +x build/intermediate/vm-$1/vm
        if [ "$(build/intermediate/vm-$1/vm $VM_TEST 2>/dev/null)" = "$VM_RESULT" ]; then
            echo "Using $1/ VM"
            cp build/intermediate/vm-$1/vm build/posix/share/onramp/platform/vm-$1
            (cd build/posix/bin; ln -s ../share/onramp/platform/vm-$1 onrampvm)
        fi
    fi
}

setup_vm_detect_machine_code() {

    # Note: We try to use the Linux VMs on FreeBSD since they provide optional
    # Linux syscall compatibility. The tests will determine whether it's
    # enabled.
    if [ "$(uname -s)" = "Linux" ] || [ "$(uname -s)" = "FreeBSD" ]; then
        if [ "$(uname -m)" = "x86_64" ]; then
            setup_vm_binary x86_64-linux
        fi
    fi

    # No other POSIX operating systems are currently implemented.
}

setup_vm_python() {
    echo "Checking python/ VM"
    if [ "$(platform/vm/python/vm.py $VM_TEST 2>/dev/null)" = "$VM_RESULT" ]; then
        echo "WARNING: The Python VM is very slow. It may take tens of minutes to"
        echo "         complete the bootstrap process."
        echo "Using python/ VM"
        cp platform/vm/python/vm.py build/posix/share/onramp/platform/vm.py
        (cd build/posix/bin; ln -s ../share/onramp/platform/vm.py onrampvm)
    fi
}

setup_vm_sh() {
    echo "Checking sh/ VM"
    if [ "$(platform/vm/sh/vm.sh $VM_TEST 2>/dev/null)" = "$VM_RESULT" ]; then
        echo "WARNING: The POSIX shell VM is incomplete and glacially slow. It will"
        echo "         take months to complete the bootstrap process and will probably"
        echo "         fail due to missing features. Do not use this."
        echo "Using sh/ VM"
        cp platform/vm/sh/vm.sh build/posix/share/onramp/platform/vm.sh
        (cd build/posix/bin; ln -s ../share/onramp/platform/vm.sh onrampvm)
    fi
}

autodetect_vm() {

    # We prefer to use a native machine code VM to provide a genuine bootstrap
    # with minimal dependencies. When bootstrapping with a machine code VM and
    # a POSIX shell hex tool, the only non-firmware trust seeds are the kernel
    # and coreutils.
    if ! [ -e $VM_PATH ]; then
        setup_vm_detect_machine_code
    fi

    # Otherwise we try to use a VM written in C. They are the fastest VMs and
    # the most likely to be compilable.
    #
    # Obviously it sort of defeats the purpose to use a C compiler to bootstrap
    # Onramp, but this can still be useful for example on an old system that
    # only has a C89 compiler which couldn't directly compile most modern
    # compilers.
    #
    # We prefer the debugger even though it's a bit slower because it provides
    # annotated stack traces on crashes.
    if ! [ -e $VM_PATH ]; then
        setup_vm_c_debugger
    fi
    if ! [ -e $VM_PATH ]; then
        setup_vm_c89
    fi

    # As a last resort we use an interpreted VM. These are sorted roughly in
    # decreasing order of performance.
    # TODO implement a VM in something faster than Python
    if ! [ -e $VM_PATH ]; then
        setup_vm_python
    fi

    # We don't use the sh/ VM ever. It's so slow that it would take months to
    # complete the bootstrap.

    if ! [ -e $VM_PATH ]; then
        echo "ERROR: No VM was found for this platform."
        exit 1
    fi
}

autodetect_min_vm() {
    # We support only machine code VMs under --min. Any other VMs would require
    # an additional dependency.
    if ! [ -e $VM_PATH ]; then
        setup_vm_detect_machine_code
    fi
    if ! [ -e $VM_PATH ]; then
        echo "ERROR: No dependency-free VM was found for this platform."
        exit 1
    fi
}

choose_vm() {
    if [ "$1" = "c-debugger" ]; then
        setup_vm_c_debugger
    elif [ "$1" = "c89" ]; then
        setup_vm_c89
    elif [ "$1" = "python" ]; then
        setup_vm_python
    elif [ "$1" = "sh" ]; then
        setup_vm_sh
    elif [ "$1" = "x86_64-linux" ]; then
        setup_vm_binary x86_64-linux
    else
        echo "ERROR: No POSIX setup process exists for requested VM: $1"
        exit 1
    fi

    if ! [ -e $VM_PATH ]; then
        echo "ERROR: The requested VM could not be configured for this platform."
        exit 1
    fi
}



#######################################################
# Argument Parsing
#######################################################

usage() {
    echo "Usage:"
    echo
    echo "    $0 [options]"
    echo
    echo "Options:"
    echo
    echo "    --hex <name>     Use the hex tool with the given name"
    echo "    --vm <name>      Use the VM with the given name"
    echo "    --dev            Use preferred tools for developing Onramp"
    echo "    --min            Use only tools with no additional dependencies (i.e. a shell"
    echo "                         hex tool and machine code VM), fail otherwise"
    echo "    --skip-core      Skip the core bootstrap; just do the POSIX setup"
    echo
    echo "Look in platform/hex/ and platform/vm/ for the names of tools. Only those tools"
    echo "that support POSIX platforms can be built by this script."
}

# parse arguments
DEV=0
MIN=0
SKIP_CORE=0
HEX_CHOICE=
VM_CHOICE=
while [ "x$1" != "x" ]; do

    if [ "$1" = "--dev" ]; then
        DEV=1
        shift

    elif [ "$1" = "--min" ]; then
        MIN=1
        shift

    elif [ "$1" = "--vm" ]; then
        shift
        if [ "x$1" = "x" ]; then
            echo "ERROR: --vm must be followed by a VM name."
            exit 1
        fi
        VM_CHOICE=$1
        shift

    elif [ "$1" = "--hex" ]; then
        shift
        if [ "x$1" = "x" ]; then
            echo "ERROR: --hex must be followed by a hex tool name."
            exit 1
        fi
        HEX_CHOICE=$1
        shift

    elif [ "$1" = "--skip-core" ]; then
        SKIP_CORE=1
        shift

    elif [ "$1" = "--help" ] || [ "$1" = "-h" ] || [ "$1" = "-?" ]; then
        usage
        exit 0

    else
        echo "ERROR: Unrecognized option: $1"
        exit 1
    fi
done

if [ $DEV -eq 1 ] && [ $MIN -eq 1 ]; then
    echo "ERROR: --dev cannot be combined with --min."
    exit 1
fi

if [ $MIN -eq 1 ]; then
    if [ "x$HEX_CHOICE" != "x" ]; then
        echo "ERROR: --min cannot be combined with --hex."
        exit 1
    fi
    if [ "x$VM_CHOICE" != "x" ]; then
        echo "ERROR: --min cannot be combined with --vm."
        exit 1
    fi
fi

if [ $DEV -eq 1 ]; then
    if [ "x$HEX_CHOICE" != "x" ]; then
        echo "ERROR: --dev cannot be combined with --hex."
        exit 1
    fi
    if [ "x$VM_CHOICE" != "x" ]; then
        echo "ERROR: --dev cannot be combined with --vm."
        exit 1
    fi
    # For development we use our best error-handling tools. These require a C
    # compiler and make tool but so do all the tests.
    HEX_CHOICE=c89
    VM_CHOICE=c-debugger
fi



#######################################################
# Main build
#######################################################

echo "Cleaning build/"
rm -rf build

# Setup basic POSIX paths
mkdir -p \
    build/posix/bin \
    build/posix/share/onramp/platform
(cd build/posix/share/onramp; ln -sf ../../../output/bin)
(cd build/posix/share/onramp; ln -sf ../../../output/lib)
(cd build/posix/share/onramp; ln -sf ../../../output/include)

# Setup the hex tool
if [ "x$HEX_CHOICE" != "x" ]; then
    choose_hex $HEX_CHOICE
elif [ $MIN -eq 1 ]; then
    autodetect_min_hex
else
    autodetect_hex
fi

# Setup the VM
init_vm
if [ "x$VM_CHOICE" != "x" ]; then
    choose_vm $VM_CHOICE
elif [ $MIN -eq 1 ]; then
    autodetect_min_vm
else
    autodetect_vm
fi

# Add the VM to our path
export PATH="$(pwd)/build/posix/bin:$PATH"

# We need the hex tool inside the VM to get started. This needs to be converted
# from the outside.
echo "Hexing hex/onramp"
mkdir -p build/intermediate/hex-0-onramp
$HEX core/hex/0-onramp/hex.oe.ohx -o build/intermediate/hex-0-onramp/hex.oe

# We also need the Onramp shell in order to run our build script inside the VM.
# We can now use our Onramp bytecode hex tool for it.
echo "Hexing sh"
mkdir -p build/intermediate/sh
onrampvm build/intermediate/hex-0-onramp/hex.oe core/sh/sh.oe.ohx -o build/intermediate/sh/sh.oe

# Now that we have everything we need we can jump inside the VM for the rest
# of the bootstrap process.
if [ $SKIP_CORE -eq 0 ]; then
    echo
    echo "Entering Onramp virtual machine..."
    # TODO since our sh tool is incomplete we run this script directly.
    #onrampvm build/intermediate/sh/sh.oe core/build.sh
    sh core/build.sh
fi

# Lastly we copy the POSIX wrappers into place.
cp platform/cc/posix/onrampcc build/posix/bin
cp platform/cc/posix/onrampar build/posix/bin
cp platform/cc/posix/onramphex build/posix/bin
cp platform/cc/posix/wrap-header build/posix/share/onramp/platform

echo
if [ $SKIP_CORE -eq 0 ]; then
    echo "POSIX build complete."
else
    echo "POSIX setup complete."
fi
echo

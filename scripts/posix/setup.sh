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



# This script sets up the platform-specific parts of Onramp for POSIX. It
# chooses a VM and hex tool, builds them (if necessary), and hexes the initial
# stages.
#
# See docs/bootstrap-path.md to follow what this script does.



set -e
cd "$(dirname "$0")/../.."

# We currently do a full clean. We should add a command-line option to keep the
# VM in place in case you want to choose a custom VM (e.g. the debugger.)
rm -rf build

mkdir -p \
    build/posix/bin \
    build/posix/share/onramp/platform
(cd build/posix/share/onramp; ln -sf ../../../output/bin)
(cd build/posix/share/onramp; ln -sf ../../../output/lib)
(cd build/posix/share/onramp; ln -sf ../../../output/include)



##################################
# Choose a hex tool
##################################

# For now, since this is a POSIX shell script, we'll just use the hex tool
# implemented in POSIX shell. It's plenty fast enough.
# TODO we should test whether it works and if not fallback to sh-alt or
# something else.
HEX=platform/hex/sh/hex.sh
#TODO
#HEX=platform/hex/python/hex.py
echo "Using hex tool: $HEX"



##################################
# Choose a VM
##################################

HAVE_VM=0

# Check if we already have a VM on the command-line. If so we won't need one in
# our build folder.
if [ $HAVE_VM -eq 0 ]; then
    if command -v onrampvm > /dev/null; then
        echo "Using existing onrampvm on PATH: $(command -v onrampvm)"
        HAVE_VM=1
    fi
fi

# Otherwise we're going to need a VM in our build folder (or we already have
# one there.) We'll set up our PATH to use it now.
if [ $HAVE_VM -eq 0 ]; then
    export PATH="$(pwd)/build/posix/bin:$PATH"
    if command -v onrampvm > /dev/null; then
        echo "Using existing onrampvm in build/: $(command -v onrampvm)"
        HAVE_VM=1
    fi
fi

# We always prefer a native VM if possible.
# TODO disabled for now, native VM is missing some syscalls
if false; then
#if [ $HAVE_VM -eq 0 ]; then
    NATIVE_VM=platform/vm/$(uname -m)-$(uname -s|tr [A-Z] [a-z])
    if [ -e $NATIVE_VM ]; then
        echo "Hexing $NATIVE_VM"
        $HEX $NATIVE_VM/vm.ohx -o build/posix/share/onramp/platform/vm
        chmod +x build/posix/share/onramp/platform/vm
        (cd build/posix/bin; ln -s ../share/onramp/platform/vm onrampvm)
        echo "Using native vm: $NATIVE_VM"
        HAVE_VM=1
    fi
fi

# Otherwise check for supported compilable languages
if [ $HAVE_VM -eq 0 ]; then
    for CC in cc gcc tcc; do
        if command -v $CC > /dev/null; then
            echo "Building vm/c-debugger"
            if $CC -O2 -Icore/libo/1-opc/include core/libo/1-opc/src/*.c platform/vm/c-debugger/src/*.c -o build/posix/share/onramp/platform/vm; then
                echo "Using compiled VM: vm/c-debugger"
                (cd build/posix/bin; ln -s ../share/onramp/platform/vm onrampvm)
                HAVE_VM=1
                break
            fi
        fi
    done
fi

# Otherwise check if we have a supported scripting language.
if [ $HAVE_VM -eq 0 ]; then
    # Note that we don't include the POSIX shell VM (sh/vm.sh), it's too slow
    SCRIPTING_VMS="python/vm.py"
    for SCRIPTING_VM in $SCRIPTING_VMS; do
        LANGNAME=$(dirname $SCRIPTING_VM)
        FILENAME=$(basename $SCRIPTING_VM)
        if command -v $LANGNAME > /dev/null; then
            SCRIPTING_VM=platform/vm/$SCRIPTING_VM
            echo "Using $LANGNAME vm: $SCRIPTING_VM"
            cp $SCRIPTING_VM build/posix/share/onramp/platform/$FILENAME
            (cd build/posix/bin; ln -s ../share/onramp/platform/$FILENAME onrampvm)
            HAVE_VM=1
            if [ "$LANGNAME" = "sh" ]; then
                echo "WARNING: POSIX shell VM is extremely slow!"
            fi
            break
        fi
    done
fi

# If we still don't have a VM, we can't bootstrap.
if [ $HAVE_VM -eq 0 ]; then
    echo "ERROR: No Onramp virtual machine could be found for your system."
    exit 1
fi



##################################
# Bootstrap core Onramp tools
##################################

# We need the hex tool inside the VM to get started. This needs to be converted
# from the outside.
echo "Hexing hex/onramp"
mkdir -p build/intermediate/hex-0-onramp
$HEX core/hex/0-onramp/hex.oe.ohx -o build/intermediate/hex-0-onramp/hex.oe

# We also need the Onramp shell in order to run our build script inside the VM.
echo "Hexing sh"
mkdir -p build/intermediate/sh
$HEX core/sh/sh.oe.ohx -o build/intermediate/sh/sh.oe



echo "POSIX setup complete."

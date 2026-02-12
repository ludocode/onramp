#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2026 Fraser Heavy Software
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


# This is the configure script for POSIX platforms. It works like a traditional
# configure script except that instead of generating a Makefile, it generates
# a build.sh script so that Onramp does not depend on Make.
#
# (Onramp can also be bootstrapped before a shell, but if you're running this
# you already trust your shell so that's what we generate.)
#
# Out-of-tree builds are supported. You can create a separate directory (as a
# subdirectory, a sibling directory, or anywhere else) in which to run this
# script to keep the source directory clean. The build directory will have
# symlinks to the source.
# (TODO: --test doesn't work out-of-tree yet.)


# This script configures a hex tool and VM, then generates build scripts.
# `configure.sh` scripts in the `platform/hex/` and `platform/vm/` directories
# are used by this script to configure the various tools. Only those tools that
# have POSIX support have a `configure.sh`.
#
# The script first configures a hex tool (or expects one configured manually)
# at `onramp/configure/onramphex`. This is only used to hex the VM (if
# necessary) and the Onramp bytecode hex/0-onramp hex tool.
#
# The script then configures a VM (or expects one configured manually) at
# `onramp/posix/bin/onrampvm`. This is typically a wrapper script or symlink to
# the VM in `onramp/posix/share/onramp/platform/`. The VM is included in the
# final POSIX installation so it must be in `onramp/posix/` which gets
# installed to `~/.local/` (or `/usr/local/` or the user's preferred
# installation path.)
#
# Lastly, the script generates `build.sh`, `install.sh` and possibly
# `build.ninja` files. These are mostly wrappers to scripts in
# `scripts/posix/`.


set -e
BUILD="$(pwd)"
ROOT="$(dirname "$0")"

usage() {
    cat <<EOF

Usage:

    $0 [options]

Options:

    --strict        Allow only minimal dependencies (kernel, coreutils, libc)
    --hex <name>    Use the hex tool with the given name, or "manual"
    --vm <name>     Use the VM with the given name, or "manual"
    --dev           Use preferred tools for developing Onramp
    --test          Run all tests during bootstrap process
    --native        Don't bootstrap; build only final stage tools with native cc
    --clean         Don't configure; delete all configure and build files
    --verbose       Print more information
    --help          Print this help

The directories in platform/hex/ and platform/vm/ are the names of tools. Only
those tools that support POSIX platforms can be built by this script.

The --test and --native options have additional dependencies (Make, Ninja, and
a native C compiler.) They are not used in a proper bootstrap.

Pass "manual" as the hex tool or VM if you have configured it manually before
running this script. The hex tool and VM must be installed at:
    output/configure/onramphex
    output/posix/bin/onrampvm

After configuration, run \`./build.sh\` to build and \`./install.sh\` to
install.

See the Setup Guide for details:
    docs/setup-guide.md
EOF
}

# Print an info message (whether or not --verbose is given)
logi() {
    echo "$@"
    echo "$@" >> config.log
}

# Print a debug message (only if --verbose is given)
logd() {
    if [ $VERBOSE -eq 1 ]; then
        echo "$@"
    fi
    echo "$@" >> config.log
}

setup_log() {
    if [ -e config.log ]; then
        echo '---------------' >> config.log.history
        cat config.log >> config.log.history
        rm -f config.log
    fi
    touch config.log
}

# Run an `rm` command and also log it.
rm_log() {
    logi rm "$@"
    rm "$@"
}

clean() {
    if [ -L core ]; then rm_log core; fi
    if [ -L extra ]; then rm_log extra; fi
    if [ -L platform ]; then rm_log platform; fi
    if [ -L "test" ]; then rm_log "test"; fi
    rm_log -rf output
    rm_log -f build.sh install.sh build.ninja

    # log must be cleaned last (otherwise rm_log will recreate it)
    rm_log -f .ninja_log config.log config.log.history
    exit 0
}

parse_options() {
    # These options must appear alone.
    if [ $# -eq 1 ]; then
        case "$1" in
            --clean) clean ;;
            --distclean) clean ;; # undocumented alias of --clean
        esac
    fi

    DEV=0
    STRICT=0
    NATIVE=0
    VERBOSE=0
    TEST=0
    HEX_CHOICE=
    VM_CHOICE=
    NO_HEX=" "
    NO_VM=" "

    while [ $# -ne 0 ]; do
        case "$1" in
            --dev) DEV=1; shift ;;
            --strict) STRICT=1; shift ;;
            --native) NATIVE=1; shift ;;
            --verbose) VERBOSE=1; shift ;;
            --test) TEST=1; shift ;;
            --hex) HEX_CHOICE="$2"; shift; shift ;;
            --vm) VM_CHOICE="$2"; shift; shift ;;

            # We don't want to restrict this option to be alone like --clean
            # even though it stops argument parsing right away. Users should be
            # able to append it to a partially written command-line to get
            # help.
            --help) usage ; exit 0 ;;

            --clean)
                logi "ERROR: --clean cannot be combined with any other options."
                exit 1
                ;;

            # --min is an undocumented and deprecated alias of --strict. we
            # keep it for backwards compatibility with the old posix build
            # script.
            --min)
                logi "WARNING: --min is deprecated. Assuming --strict instead."
                STRICT=1;
                shift
                ;;

            # These undocumented options are mostly for testing purposes. They
            # forbid using a particular tool. They can be specified multiple
            # times.
            --no-hex) NO_HEX="$NO_HEX$2 "; shift; shift ;;
            --no-vm) NO_VM="$NO_VM$2 "; shift; shift ;;

            *)
                logi "ERROR: Invalid option: $1"
                usage
                exit 1
                ;;
        esac
    done

    # strict compatibility
    if [ $STRICT -eq 1 ] && [ "$HEX_CHOICE" != "" ]; then
        logi "WARNING: --hex overrides --strict."
    fi
    if [ $STRICT -eq 1 ] && [ "$VM_CHOICE" != "" ]; then
        logi "WARNING: --vm overrides --strict."
    fi
    if [ $STRICT -eq 1 ] && [ $DEV -eq 1 ]; then
        logi "ERROR: --strict and --dev are incompatible."
        exit 1
    fi
    if [ $NATIVE -eq 1 ] && [ $TEST -eq 1 ]; then
        # would really like to lift this restriction but there's a lot of work involved
        logi "ERROR: --native and --test are currently incompatible."
        exit 1
    fi
    if [ $STRICT -eq 1 ] && [ $NATIVE -eq 1 ]; then
        logi "WARNING: --strict applies only to VM and hex tool, not use of --native toolchain."
    fi
    if [ $STRICT -eq 1 ] && [ $NATIVE -eq 1 ]; then
        logi "WARNING: --strict applies only to VM and hex tool, not --test tooling."
    fi

    # TODO: --test doesn't work out-of-tree yet
    if [ $TEST -eq 1 ] && [ "$ROOT" != "." ]; then
        logi "ERROR: --test doesn't work out-of-tree yet. It must be run as \`./configure.sh\`."
        exit 1
    fi

    # set preferred tools for --dev
    if [ $DEV -eq 1 ] && [ "$HEX_CHOICE" = "" ]; then
        HEX_CHOICE=c89
        logi "Choosing hex tool $HEX_CHOICE for development due to --dev"
    fi
    if [ $DEV -eq 1 ] && [ "$VM_CHOICE" = "" ]; then
        VM_CHOICE=c-debugger
        logi "Choosing vm $VM_CHOICE for development due to --dev"
    fi
}

create_paths() {
    if ! [ -e core ]; then ln -s "$ROOT/core" .; fi
    if ! [ -e extra ]; then ln -s "$ROOT/extra" .; fi
    if ! [ -e platform ]; then ln -s "$ROOT/platform" .; fi
    if ! [ -e "test" ]; then ln -s "$ROOT/test" .; fi  # vim syntax highlighting gets confused by test keyword

    # We always clean all previous build output on a reconfigure.
    if [ -e output ]; then
        logi "Cleaning previous output"

        # If the user has manually configured a VM or hex tool, we need to
        # avoid cleaning it.
        if [ "$HEX_CHOICE" != "manual" ] && [ "$VM_CHOICE" != "manual" ]; then
            rm_log -rf output
        else
            if [ "$HEX_CHOICE" != "manual" ]; then
                rm_log -rf output/configure
            fi
            if [ "$VM_CHOICE" != "manual" ]; then
                rm_log -rf output/posix
            fi
            rm_log -rf output/intermediate output/final output/test
        fi
    fi

    mkdir -p \
        output/configure \
        output/posix/bin \
        output/posix/share/onramp/platform
    (cd output/posix/share/onramp; ln -sf ../../../final/bin .)
    (cd output/posix/share/onramp; ln -sf ../../../final/lib .)
    (cd output/posix/share/onramp; ln -sf ../../../final/include .)
}

test_hex() {
    logd "Testing hex tool $1..."

    if ! output/configure/onramphex test/hex/tests/hello-world-readme.ohx -o output/configure/hex-$1.out >>config.log 2>&1; then
        logd "Couldn't run hex tool $1."
        return 1;
    fi

    if [ "$(cat output/configure/hex-$1.out)" != "Hello world!" ]; then
        logd "Hex tool $1 produced incorrect results. Output:"
        logd ---
        cat output/configure/hex-$1.out >>config.log
        logd ---
        return 1;
    fi
}

try_hex() {
    case "$NO_HEX" in
        *" $1 "*)
            logi "Skipping hex tool $1 due to --no-hex."
            return 1
    esac

    logd "Checking hex tool $1..."

    # Make sure this is a valid tool
    if ! [ -e "$ROOT/platform/hex/$1" ]; then
        logd "No such hex tool: $1"
        return 1
    fi
    if ! [ -e "$ROOT/platform/hex/$1/configure.sh" ]; then
        # This should only happen if the user has requested a tool that doesn't
        # have POSIX support.
        logd "$1 is not supported on POSIX."
        return 1
    fi

    # Build (if necessary) and install the tool
    if ! "$ROOT/platform/hex/$1/configure.sh" >>config.log 2>&1; then
        logd "Couldn't configure hex tool $1."
        rm -f output/configure/onramphex
        return 1
    fi

    # Make sure it works
    if ! test_hex $1; then
        logd "Hex tool $1 failed test."
        rm -f output/configure/onramphex
        return 1
    fi

    logi "Configured hex tool: $1"

    if [ "$1" = "sh-alt" ]; then
        echo "WARNING: The sh-alt hex tool is very slow. It will take a long time to finish the configuration."
    fi
}

setup_hex() {
    logi "Configuring hex tool..."

    if [ "$HEX_CHOICE" != "" ]; then

        # Manual configuration
        if [ "$HEX_CHOICE" = "manual" ]; then
            if ! [ -e output/configure/onramphex ]; then
                logi 'ERROR: `--hex manual` was requested but `output/configure/onramphex` does not exist.'
                exit 1
            fi
            if ! test_hex manual; then
                logi "ERROR: Manually configured hex tool failed test."
                exit 1
            fi
            return
        fi

        # Explicit hex tool selection
        if ! try_hex "$HEX_CHOICE"; then
            logi "ERROR: Failed to configure the requested hex tool."
            exit 1
        fi
        return
    fi

    if try_hex x86_64-linux; then return; fi
    if try_hex sh; then return; fi
    if try_hex sh-alt; then return; fi

    if [ $STRICT -eq 1 ]; then
        logd "There are no other --strict hex tools available."
        logi "ERROR: --strict was requested but a minimal dependency hex tool was not found."
        exit 1
    fi

    if try_hex python; then return; fi
    if try_hex python-golf; then return; fi
    if try_hex c89; then return; fi
    if try_hex xxd; then return; fi

    logi "ERROR: A working hex tool was not found."
    exit 1
}

test_vm() {
    # Make sure it can run the test without crashing
    if ! output/posix/bin/onrampvm output/configure/hello.oe >output/configure/vm-$1.out 2>>config.log; then
        logd "Couldn't run VM $1."
        return 1;
    fi

    # Make sure it matches the expected output
    if [ "$(cat output/configure/vm-$1.out)" != "Hello world!" ]; then
        logd "VM $1 produced incorrect results. Output:"
        logd ---
        cat output/configure/vm-$1.out >>config.log
        logd ---
        return 1;
    fi
}

try_vm() {
    case "$NO_VM" in
        *" $1 "*)
            logi "Skipping VM $1 due to --no-vm."
            return 1
            ;;
    esac

    logd "Checking VM $1..."

    # Make sure this is a valid tool
    if ! [ -e "$ROOT/platform/vm/$1" ]; then
        logd "No such VM: $1"
        return 1
    fi
    if ! [ -e "$ROOT/platform/vm/$1/configure.sh" ]; then
        # This should only happen if the user has requested a tool that doesn't
        # have POSIX support.
        logd "$1 is not supported on POSIX."
        return 1
    fi

    # Build (if necessary) and install the tool
    if ! "$ROOT/platform/vm/$1/configure.sh" >>config.log 2>&1; then
        logd "Couldn't configure VM $1."
        rm -f output/posix/bin/onrampvm
        return 1
    fi

    # Make sure it works
    if ! test_vm $1; then
        logd "VM $1 failed test."
        rm -f output/posix/bin/onrampvm
        return 1
    fi

    logi "Configured VM: $1"

    if [ "$1" = "python" ]; then
        echo "WARNING: The Python VM is very slow. It will take several hours to bootstrap Onramp."
    fi
}

setup_vm() {
    # To test the VM we'll run a simple Hello World program. (We avoid using a
    # large program like vminfo because the hex tool may be slow.)
    logd "Hexing VM test program..."
    output/configure/onramphex test/vm/io/hello.oe.ohx -o output/configure/hello.oe >>config.log 2>&1

    logi "Configuring VM..."

    if [ "$VM_CHOICE" != "" ]; then

        # Manual configuration
        if [ "$VM_CHOICE" = "manual" ]; then
            if ! [ -e output/posix/bin/onrampvm ]; then
                logi 'ERROR: `--vm manual` was requested but `output/posix/bin/onrampvm` does not exist.'
                exit 1
            fi
            if ! test_vm manual; then
                logi "ERROR: Manually configured VM failed test."
                exit 1
            fi
            return
        fi

        # Explicit vm tool selection
        if ! try_vm "$VM_CHOICE"; then
            logi "ERROR: Failed to configure the requested VM."
            exit 1
        fi
        return
    fi

    if try_vm x86_64-linux; then return; fi

    if [ $STRICT -eq 1 ]; then
        logd "There are no other --strict VMs available."
        logi "ERROR: --strict was requested but a minimal dependency VM was not found."
        exit 1
    fi

    if try_vm c89; then return; fi
    if try_vm c-debugger; then return; fi
    if try_vm python; then return; fi

    logi "ERROR: A working VM was not found."
    exit 1
}

setup_misc() {

    # Hex our Onramp bytecode hex tool
    echo "Hexing hex/onramp..."
    mkdir -p output/configure/hex-0-onramp
    output/configure/onramphex core/hex/0-onramp/hex.oe.ohx -o output/configure/hex-0-onramp/hex.oe

    # Hex our Onramp shell (with our bytecode tool because it is much faster
    # than the shell hex tool)
    echo "Hexing sh..."
    mkdir -p output/configure/sh
    output/posix/bin/onrampvm output/configure/hex-0-onramp/hex.oe core/sh/sh.oe.ohx -o output/configure/sh/sh.oe

    # copy POSIX wrappers into place
    # (The tools won't work until the build is complete but it's simpler to
    # copy the wrappers in place as part of configure.)
    if [ $NATIVE -eq 1 ]; then
        cp platform/cc/posix/onrampcc-native output/posix/bin/onrampcc
            # TODO ar/1 not written yet
            cp platform/cc/posix/onrampar output/posix/bin
            #(cd output/posix/bin; ln -sf ../../../final/bin/ar onrampar)
        (cd output/posix/bin; ln -sf ../../final/bin/hex onramphex)
    else
        cp platform/cc/posix/onrampcc output/posix/bin
        cp platform/cc/posix/onrampar output/posix/bin
        cp platform/cc/posix/onramphex output/posix/bin
    fi
    cp platform/cc/posix/wrap-header output/posix/share/onramp/platform

    # generate ninja file if necessary
    if [ $NATIVE -eq 1 ]; then
        scripts/posix/native.sh --configure
    fi

    # generate build script
    echo '#!/bin/sh' >build.sh
    echo "# This Onramp build script is generated by $0" >>build.sh
    echo 'set -e' >>build.sh
    echo 'cd "$(dirname "$0")"' >>build.sh
    echo "TEST=$TEST" >> build.sh
    echo "NATIVE=$NATIVE" >> build.sh
    echo "ROOT='$ROOT'" >> build.sh
    echo ". \"$ROOT\"/scripts/posix/build-impl.sh" >>build.sh
    chmod +x build.sh
    logi "Generated build.sh"

    # generate install script
    echo '#!/bin/sh' >install.sh
    echo "# This Onramp install script is generated by $0" >>install.sh
    echo 'set -e' >>install.sh
    echo 'cd "$(dirname "$0")"' >>install.sh
    echo ". \"$ROOT\"/scripts/posix/install-impl.sh" >>install.sh
    chmod +x install.sh
    logi "Generated install.sh"
}

finish() {
    logi 'Configuration complete. Run `./build.sh` to build Onramp.'
}

go() {
    setup_log
    parse_options "$@"
    logd "Configuration started by $0 $(date)"
    create_paths
    setup_hex
    setup_vm
    setup_misc
    finish
}

go "$@"

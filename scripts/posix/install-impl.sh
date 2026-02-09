#!/bin/false

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


# This script is the implementation of the install.sh generated from the
# configure script.
#
# It is meant to be sourced so it is not executable.


set -e
BUILD="$(pwd)"

usage() {
    cat <<EOF

Usage:

    $0 [options]

Options:

    --path <path>   Set the destination path for the installation
    --link          Install symlinks to the build instead of copying files
    --uninstall     Don't install; remove installation at the destination path
    --help          Print this help

This script installs Onramp. It must be built first (unless --uninstall or
--link is given.)

The default install path is \`~/.local\` for non-root users and \`/usr/local\`
for root. Use --path to customize it (similar to the --prefix or DESTDIR
options of a traditional configure script.)

(The Onramp installation is relocatable: you can choose the path after it is
built; you can install to a staging area and move it later; etc.)

To run Onramp programs, the \`/bin\` subdirectory of the install path must be
on your PATH. By default this is \`~/.local/bin/\`. Some Linux distributions do
this by default but not all; check your PATH to make sure.

A --link installation is useful for developing Onramp. It allows you to make
changes and rebuild without having to reinstall.

See the Setup Guide for details:
    docs/setup-guide.md
EOF
}

parse_options() {
    if [ "$(id -u)" -eq 0 ]; then
        DEFAULT_DEST_PATH=/usr/local
    else
        DEFAULT_DEST_PATH="$HOME/.local"
    fi

    DEST_PATH="$DEFAULT_DEST_PATH"
    LINK=0
    UNINSTALL=0

    while [ $# -ne 0 ]; do
        case "$1" in
            --help) usage ; exit 0 ;;
            --path) DEST_PATH="$2"; shift; shift ;;
            --link) LINK=1; shift ;;
            --uninstall) UNINSTALL=1; shift ;;

            # This is a deprecated alias of --link.
            --dev) LINK=1; shift ;;
        esac
    done

    # if the destination path starts with $HOME, print ~ instead
    DEST_PATH_PRINT="$DEST_PATH"
    case "$DEST_PATH" in
        $HOME*)
            DEST_PATH_PRINT='~'"${DEST_PATH#$HOME}"
            ;;
    esac
}

check_path() {
    if ! [ -e $1 ]; then
        echo "ERROR: $1 is missing. Did you build Onramp?"
        exit 1
    fi
}

uninstall() {
    echo "Deleting $DEST_PATH_PRINT/bin/onrampvm"   ; rm -f "$DEST_PATH/bin/onrampvm"
    echo "Deleting $DEST_PATH_PRINT/bin/onrampcc"   ; rm -f "$DEST_PATH/bin/onrampcc"
    echo "Deleting $DEST_PATH_PRINT/bin/onrampar"   ; rm -f "$DEST_PATH/bin/onrampar"
    echo "Deleting $DEST_PATH_PRINT/bin/onramphex"  ; rm -f "$DEST_PATH/bin/onramphex"
    echo "Deleting $DEST_PATH_PRINT/share/onramp/"  ; rm -rf "$DEST_PATH/share/onramp"
}

prepare_install() {

    # This is the same as uninstall() except we don't print it. Might be nice to
    # unify these.
    rm -f "$DEST_PATH/bin/onrampvm"
    rm -f "$DEST_PATH/bin/onrampcc"
    rm -f "$DEST_PATH/bin/onrampar"
    rm -f "$DEST_PATH/bin/onramphex"
    rm -rf "$DEST_PATH/share/onramp"

    # Make directories
    mkdir -p $DEST_PATH/share
    mkdir -p $DEST_PATH/bin
}

install() {
    # Make sure Onramp is actually built
    check_path output/posix/bin/onrampvm
    check_path output/posix/bin/onrampcc
    check_path output/posix/bin/onrampar
    check_path output/posix/bin/onramphex
    check_path output/posix/share/onramp
    check_path output/final/bin/cc*
    check_path output/final/bin/cci*
    check_path output/final/bin/ar*
    check_path output/final/bin/hex*
    check_path output/final/include/stdlib.h
    check_path output/final/lib/libc.oa

    prepare_install

    # Copy share/
    cp -Lr output/posix/share/onramp $DEST_PATH/share/onramp
    echo "Installed $DEST_PATH_PRINT/share/onramp/"

    # Copy bin/
    cp output/posix/bin/onramphex $DEST_PATH/bin/onramphex  ; echo "Installed $DEST_PATH_PRINT/bin/onramphex"
    cp output/posix/bin/onrampvm  $DEST_PATH/bin/onrampvm   ; echo "Installed $DEST_PATH_PRINT/bin/onrampvm"
    cp output/posix/bin/onrampcc  $DEST_PATH/bin/onrampcc   ; echo "Installed $DEST_PATH_PRINT/bin/onrampcc"
    cp output/posix/bin/onrampar  $DEST_PATH/bin/onrampar   ; echo "Installed $DEST_PATH_PRINT/bin/onrampar"
}

install_links() {
    # Note: We don't check here that Onramp is built. We want users to be able
    # to install symlinks even before they build.

    prepare_install

    # Link share/
    ln -sf "$(pwd)/output/posix/share/onramp" $DEST_PATH/share/onramp
    echo "Linked $DEST_PATH_PRINT/share/onramp/ -> output/posix/share/onramp/"

    # Link bin/
    ln -sf "$(pwd)/output/posix/bin/onramphex" $DEST_PATH/bin/onramphex  ; echo "Linked $DEST_PATH_PRINT/bin/onramphex -> output/posix/bin/onramphex"
    ln -sf "$(pwd)/output/posix/bin/onrampvm"  $DEST_PATH/bin/onrampvm   ; echo "Linked $DEST_PATH_PRINT/bin/onrampvm -> output/posix/bin/onrampvm"
    ln -sf "$(pwd)/output/posix/bin/onrampcc"  $DEST_PATH/bin/onrampcc   ; echo "Linked $DEST_PATH_PRINT/bin/onrampcc -> output/posix/bin/onrampcc"
    ln -sf "$(pwd)/output/posix/bin/onrampar"  $DEST_PATH/bin/onrampar   ; echo "Linked $DEST_PATH_PRINT/bin/onrampar -> output/posix/bin/onrampar"
}

go() {
    parse_options "$@"

    if [ $UNINSTALL -eq 1 ]; then
        uninstall
    elif [ $LINK -eq 1 ]; then
        install_links
    else
        install
    fi

    echo "Done."

    if [ $UNINSTALL -ne 1 ]; then
        echo
        echo "Note: You need \`$DEST_PATH_PRINT/bin\` on your PATH to use the installed Onramp."
        PATH_OPT=
        if [ "$DEST_PATH" != "$DEFAULT_DEST_PATH" ]; then
            PATH_OPT=" --path \"$DEST_PATH_PRINT\""
        fi
        echo "Note: Run \`$0 --uninstall$PATH_OPT\` to uninstall."
    fi
}

go "$@"

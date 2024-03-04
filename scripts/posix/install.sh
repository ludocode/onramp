#!/bin/sh

set -e
cd "$(dirname "$0")/../.."

echo

check_path() {
    if ! [ -e $1 ]; then
        echo "ERROR: $1 is missing. Did you build Onramp?"
        exit 1
    fi
}

check_path build/posix/bin/onrampvm
check_path build/posix/bin/onrampcc
check_path build/posix/bin/onramphex
check_path build/posix/share/onramp

# TODO we should check that some of the final tools are actually built, that
# the build script didn't fail or get interrupted

rm -rf $HOME/.local/share/onramp
mkdir -p $HOME/.local/share/onramp
cp -Lr build/posix/share/onramp/* $HOME/.local/share/onramp
echo "Installed ~/.local/share/onramp/"

mkdir -p $HOME/.local/bin
cp build/posix/bin/onramphex $HOME/.local/bin/onramphex  ; echo "Installed ~/.local/bin/onramphex"
cp build/posix/bin/onrampvm $HOME/.local/bin/onrampvm    ; echo "Installed ~/.local/bin/onrampvm"
cp build/posix/bin/onrampcc $HOME/.local/bin/onrampcc    ; echo "Installed ~/.local/bin/onrampcc"
cp build/posix/bin/onrampar $HOME/.local/bin/onrampar    ; echo "Installed ~/.local/bin/onrampar"

echo
echo "Done!"
echo
echo "Note: You need ~/.local/bin on your PATH to use the installed Onramp."
echo "Note: Run scripts/posix/uninstall.sh to uninstall."
echo

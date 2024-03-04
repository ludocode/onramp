#!/bin/sh

# This script uninstalls Onramp.

set -e
cd "$(dirname "$0")/../.."

echo
echo "Deleting ~/.local/bin/onrampvm"   ; rm -f "$HOME/.local/bin/onrampvm"
echo "Deleting ~/.local/bin/onrampcc"   ; rm -f "$HOME/.local/bin/onrampcc"
echo "Deleting ~/.local/bin/onrampar"   ; rm -f "$HOME/.local/bin/onrampar"
echo "Deleting ~/.local/bin/onramphex"  ; rm -f "$HOME/.local/bin/onramphex"
echo "Deleting ~/.local/share/onramp/"  ; rm -rf "$HOME/.local/share/onramp"
echo
echo "Done."

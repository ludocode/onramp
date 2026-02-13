This directory contains helper scripts for building Onramp on POSIX systems.

The only script here that is still meant to be callable is `uninstall.sh`. It allows uninstalling an Onramp installation without having to reconfigure.

The rest of these scripts are either deprecated or they are internal implementation details. The top-level `configure.sh` is now the main script for building on POSIX. It generates wrapper scripts for the internal implementation scripts here.

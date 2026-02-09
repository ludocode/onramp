This folder contains helper scripts for building Onramp on POSIX systems.

These scripts are either deprecated or they are internal implementation details. Users don't need to run anything in here anymore.

The top-level `configure.sh` is now the main script for building on POSIX. It generates `build.sh` and `install.sh` which are wrappers for `build-impl.sh` and `install-impl.sh` here.

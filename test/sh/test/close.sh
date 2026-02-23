#!/bin/sh

# The MIT License (MIT)
# Copyright (c) 2026 Fraser Heavy Software
# This test case is part of the Onramp compiler project.

# This script must be run in the Onramp shell to test that the shell will
# prevent a child program from closing the shell's standard streams.

echo Compiling test program to close stdout...
mkdir -p output/test/sh
onrampvm output/final/bin/cc.oe test/sh/test/close.c -o output/test/sh/close.oe

echo Running test program to close stdout...
onrampvm output/test/sh/close.oe

# At this point the subprogram closed its stdout; the shell should have proxied
# it and ignored it. We print a message to ensure we still have our stdout.
# TODO: Currently the libc doesn't actually close the standard streams. This
# needs to be fixed. This is currently tested just by hacking up the libc to
# allow the close.
echo Done.

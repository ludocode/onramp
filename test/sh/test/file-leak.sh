#!/bin/sh

# The MIT License (MIT)
# Copyright (c) 2026 Fraser Heavy Software
# This test case is part of the Onramp compiler project.

# This script must be run in the Onramp shell to test file handle cleanup.

set -e

echo Compiling test program to leak files...
mkdir -p output/test/sh
onrampvm output/final/bin/cc.oe test/sh/test/file-leak.c -o output/test/sh/file-leak.oe

# Many VMs have a 16 or 32 handle limit. We just run this 20 times; each run
# leaks two handles.
# The strace output should be reviewed by hand to ensure there are no leaks.
echo Running test program to leak files...
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe
onrampvm output/test/sh/file-leak.oe

echo Done.

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



# Tests running the vminfo tool with old Python VMs.
#
# The results are ignored; we just ensure that it prints at least some number
# of lines and exits with non-zero status.


set -e
cd "$(dirname "$0")/../.."

extra/vminfo/build.sh
test/vminfo/fetch-old-vms.sh

MINIMUM_LINES=20

for VERSION in 0 1 2 3; do
    set +e
    VM=build/test/old-vms/vm-$VERSION.py
    VMINFO=build/output/bin/vminfo.oe
    LOG=build/test/old-vms/vm-$VERSION.log
    echo "Running $VM $VMINFO"
    $VM $VMINFO >$LOG 2>/dev/null
    RET=$?
    if [ $RET -ne 0 ]; then
        cat $LOG
        echo "ERROR: VM version $VERSION failed vminfo with exit status $RET."
        exit 1
    fi
    if [ $(cat $LOG|wc -l) -lt $MINIMUM_LINES ]; then
        cat $LOG
        echo "ERROR: VM version $VERSION did not print enough vminfo lines."
        exit 1
    fi
done

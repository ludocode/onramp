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



# Gets old Python VMs from git history.
#
# These are used to test backwards compatibility of the vminfo tool.



set -e
cd "$(dirname "$0")/../.."

mkdir -p output/test/old-vms/
set -v
git show 27f45a7c90d7260249d0a35d632dd4470f602c18:platform/vm/python/vm.py > output/test/old-vms/vm-0.py
git show 28b484d562541db82c7fc72c4c6d8ee7ec8fe4d3:platform/vm/python/vm.py > output/test/old-vms/vm-1.py
git show 4e6d998eb6b2e07acce3e241271506a74184cf4d:platform/vm/python/vm.py > output/test/old-vms/vm-2.py
git show f314dbc3d1a9d6fe4e76b768490b250942895636:platform/vm/python/vm.py > output/test/old-vms/vm-3.py
chmod +x output/test/old-vms/*.py

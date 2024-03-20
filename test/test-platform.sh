#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c) 2024 Fraser Heavy Software
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



# Runs all platform-specific tests that can run on the current platform.
#
# (For now we assume the current platform is x86_64 Linux.)



set -e
cd "$(dirname "$0")/.."



################
# hex
################

# c89
platform/hex/c89/test.sh

# NOTE: We test the onramp/ hex tool at the end because it depends on several
# other tools here.

# python
test/hex/run.sh platform/hex/python/hex.py

# python-golf
test/hex/run.sh --lax platform/hex/python-golf/hex.py

# sh
test/hex/run.sh --lax platform/hex/sh/hex.sh

# sh-alt
test/hex/run.sh --lax platform/hex/sh-alt/hexcomb.sh
test/hex/run.sh --lax platform/hex/sh-alt/hexsemi.sh
test/hex/run.sh --lax platform/hex/sh-alt/hexsplit.sh

# x86_64-linux
if [ "$(uname -s)" = "Linux" ] && [ "$(uname -m)" = "x86_64" ]; then
    platform/hex/x86_64-linux/test.sh
fi

# xxd
test/hex/run.sh --lax platform/hex/xxd/hex.sh



################
# vm
################

# c-debugger
make -C platform/vm/c-debugger

# x86_64-linux
if [ "$(uname -s)" = "Linux" ] && [ "$(uname -m)" = "x86_64" ]; then
    platform/vm/x86_64-linux/test.sh
fi

# python
platform/vm/python/test.sh

# c89
platform/vm/c89/test.sh

# sh
# TODO disabled for now, it's too slow
#test/vm/run.sh platform/vm/sh/vm.sh



################
# misc
################

# onramp bytecode hex tool
platform/hex/onramp/test.sh

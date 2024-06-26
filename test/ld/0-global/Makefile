# The MIT License (MIT)
#
# Copyright (c) 2023-2024 Fraser Heavy Software
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

ROOT=../../..
BUILD=$(ROOT)/build/test

SRC=$(ROOT)/core/ld/0-global
OUT=$(BUILD)/ld-0-global

VM=$(BUILD)/vm-c-debugger/vm
HEX=$(BUILD)/hex-c89/hex

all: build test FORCE
FORCE:
build: $(OUT)/ld.oe FORCE

clean: FORCE
	rm -rf $(OUT)

$(VM): FORCE
	@(make -C $(ROOT)/platform/vm/c-debugger build)

$(HEX): FORCE
	$(ROOT)/platform/hex/c89/build.sh

$(OUT)/ld.oe: $(SRC)/ld.oe.ohx Makefile $(HEX)
	@rm -f $@
	@mkdir -p $(OUT)
	$(HEX) $(SRC)/ld.oe.ohx -o $@

test: build FORCE $(VM)
	../run.sh . $(VM) $(OUT)/ld.oe

generate: build FORCE $(VM)
	../generate.sh . $(VM) $(OUT)/ld.oe

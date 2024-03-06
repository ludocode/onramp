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

CFLAGS := -g
CPPFLAGS := -Wall -Wextra -Wpedantic -Wno-unused-parameter

-include ../../local.mk

ROOT=../../..
BUILD=$(ROOT)/build/test

SRC=$(ROOT)/core/cci/1-opc/src
OUT=$(BUILD)/cci-1-opc

SRCS=\
	 $(SRC)/common.c \
	 $(SRC)/compile.c \
	 $(SRC)/emit.c \
	 $(SRC)/lexer.c \
	 $(SRC)/main.c \
	 $(SRC)/parse.c \
	 $(SRC)/type.c \
	 $(SRC)/variable.c \

all: build test FORCE
FORCE:
build: $(OUT)/cci FORCE

clean: FORCE
	rm -rf $(OUT)

$(OUT)/cci: $(SRCS) Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRCS) -o $@

test-0: build FORCE
	../run.sh --ignore-output ../0-omc $(OUT)/cci

test-1: build FORCE
	../run.sh . $(OUT)/cci

test: test-1 FORCE # test-0

generate: build FORCE $(VM)
	../generate.sh . $(VM) $(OUT)/cci

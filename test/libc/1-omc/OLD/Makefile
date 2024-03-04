# TODO delete this

.PHONY: all
all: run

.PHONY: run
run: /tmp/onramp-test
	/tmp/onramp-test

.PHONY: clean
clean:
	rm -rf build

ROOT=../../../..
BUILD=$(ROOT)/build/test
LIBC0=$(ROOT)/core/libc/0-oo
LIBC1=$(ROOT)/core/libc/1-omc
SRCS :=
SRCS += $(shell find . -type f -name '*.c')
OBJS := $(patsubst %, $(BUILD)/%.o, $(SRCS))

# The test suite is an overlay over a real 32-bit libc. We pass -nostdinc but not -nostdlib.
CPPFLAGS := -m32
CPPFLAGS += -nostdinc -fno-builtin -fno-stack-protector
CPPFLAGS += -g -DDEBUG
CPPFLAGS += -I$(LIBC1)/include -I$(LIBC1)/src -I$(LIBC0)/include
CPPFLAGS += -Wall -Wextra -Werror
CPPFLAGS += -Wno-unused-parameter  # TODO temporary since so much is stubbed out
CPPFLAGS += -MD -MP
CPPFLAGS += -include __onramp/__predef.h

/tmp/onramp-test: $(OBJS)
	cc $^ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@

$(OBJS): $(BUILD)/%.o: % Makefile
	mkdir -p $(dir $@)
	$(CC) -o $@ -c $(CFLAGS) $(CPPFLAGS) $<

# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/#depdelete
DEPS := $(OBJS:%.o=%.d)
$(DEPS):
-include $(DEPS)

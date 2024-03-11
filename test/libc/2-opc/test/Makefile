# TODO convert this to new style

.PHONY: all
all: run

.PHONY: run
run: test
	./test

.PHONY: clean
clean:
	rm -rf build

BUILD=build/test
SRCS :=
SRCS += $(shell find . -type f -name '*.c')
#SRCS += $(shell find ../src -type f -name '*.c')
OBJS := $(patsubst %, $(BUILD)/%.o, $(SRCS))

LIBC=../../../../core/libc

CPPFLAGS := -m32
CPPFLAGS += -nostdinc -fno-builtin -fno-stack-protector
CPPFLAGS += -g -DDEBUG
CPPFLAGS += -I$(LIBC)/2-opc/include -I$(LIBC)/1-omc/include -I$(LIBC)/0-oo/include -I$(LIBC)/2-opc/src
CPPFLAGS += -Wall -Wextra
CPPFLAGS += -Wno-unused-parameter  # TODO temporary since so much is stubbed out
CPPFLAGS += -MD -MP
CPPFLAGS += -include __onramp/__predef.h

# One of these should be defined. We can create an overlay build over glibc or
# we can be completely standalone.
CPPFLAGS += -nostdlib -static-libgcc -lgcc
#CPPFLAGS += -DONRAMP_LIBC_OVERLAY -lm

test: $(OBJS)
	cc $^ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@

$(OBJS): $(BUILD)/%.o: % Makefile
	mkdir -p $(dir $@)
	$(CC) -o $@ -c $(CFLAGS) $(CPPFLAGS) $<

# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/#depdelete
DEPS := $(OBJS:%.o=%.d)
$(DEPS):
-include $(DEPS)

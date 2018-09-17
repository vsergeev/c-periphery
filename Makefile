LIB = periphery.a
SRCS = src/gpio.c src/spi.c src/i2c.c src/mmio.c src/serial.c

#########################################################################
#	Toolchain.
#########################################################################
#CROSSNAME       := arm-cortex_a9-linux-gnueabi-
CROSSNAME       := 
CROSS 	 	:= $(CROSSNAME)
CC			:= $(CROSS)gcc
CPP			:= $(CROSS)g++
AR			:= $(CROSS)ar
AS			:= $(CROSS)as
LD			:= $(CROSS)ld
NM			:= $(CROSS)nm
RANLIB		:= $(CROSS)ranlib
OBJCOPY		:= $(CROSS)objcopy
STRIP		:= $(CROSS)strip

SRCDIR = src
OBJDIR = obj

TEST_PROGRAMS = $(basename $(wildcard tests/*.c))

###########################################################################

OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

CFLAGS += -std=gnu99 -pedantic
CFLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast $(DEBUG) -fPIC
LDFLAGS +=

###########################################################################

.PHONY: all
all: $(LIB)

.PHONY: tests
tests: $(TEST_PROGRAMS)

.PHONY: clean
clean:
	rm -rf $(LIB) $(OBJDIR) $(TEST_PROGRAMS)

###########################################################################

tests/%: tests/%.c $(LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LIB) -o $@

###########################################################################

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(LIB): $(OBJECTS)
	$(AR) rcs $(LIB) $(OBJECTS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@


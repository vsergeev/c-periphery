LIB = periphery.a
SRCS = src/gpio.c src/spi.c src/i2c.c src/mmio.c src/serial.c src/version.c

SRCDIR = src
OBJDIR = obj

TEST_PROGRAMS = $(basename $(wildcard tests/*.c))

###########################################################################

OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

COMMIT_ID := $(shell git describe --abbrev --always --tags --dirty 2>/dev/null || echo "")

CFLAGS += -std=gnu99 -pedantic
CFLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast $(DEBUG) -fPIC
CFLAGS += -DPERIPHERY_VERSION_COMMIT=\"$(COMMIT_ID)\"
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
	ar rcs $(LIB) $(OBJECTS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@


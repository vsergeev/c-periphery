LIB = periphery.a
SOURCES = src/gpio.c src/spi.c src/i2c.c src/mmio.c src/serial.c

OBJDIR = obj

TEST_SOURCES = $(wildcard tests/*.c)
TEST_PROGRAMS = $(basename $(TEST_SOURCES))

###########################################################################

CFLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast $(DEBUG) -fPIC
LDFLAGS +=

OBJECTS = $(patsubst src/%,$(OBJDIR)/%,$(patsubst %.c,%.o,$(SOURCES)))

###########################################################################

all: $(LIB)

.PHONY : tests

tests: $(LIB) $(TEST_PROGRAMS)

clean:
	rm -rf $(LIB) $(OBJDIR) $(TEST_PROGRAMS)

###########################################################################

tests/%: tests/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LIB) -o $@

###########################################################################

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(LIB): $(OBJECTS)
	ar rcs $(LIB) $(OBJECTS)

$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@


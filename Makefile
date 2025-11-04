VERSION = 2.5.0
SO_VERSION = 2.5

STATIC_LIB = periphery.a
SHARED_LIB = libperiphery.so

SRCS = src/gpio.c src/gpio_cdev_v2.c src/gpio_cdev_v1.c src/gpio_sysfs.c src/led.c src/pwm.c src/spi.c src/i2c.c src/mmio.c src/serial.c src/version.c

SRCDIR = src
OBJDIR = obj

TEST_PROGRAMS = $(basename $(wildcard tests/*.c))

###########################################################################

OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

NULL := $(if $(filter Windows_NT,$(OS)),NUL,/dev/null)
GPIO_CDEV_V1_SUPPORT := $(shell ! env printf "\x23include <linux/gpio.h>\n\x23ifndef GPIO_GET_LINEEVENT_IOCTL\n\x23error\n\x23endif" | $(CC) -E - >$(NULL) 2>&1; echo $$?)
GPIO_CDEV_V2_SUPPORT := $(shell ! env printf "\x23include <linux/gpio.h>\nint main(void) { GPIO_V2_LINE_FLAG_EVENT_CLOCK_REALTIME; return 0; }" | $(CC) -x c -fsyntax-only - >$(NULL) 2>&1; echo $$?)
GPIO_CDEV_SUPPORT = $(if $(filter 1,$(GPIO_CDEV_V2_SUPPORT)),2,$(if $(filter 1,$(GPIO_CDEV_V1_SUPPORT)),1,0))

COMMIT_ID := $(shell git describe --abbrev --always --tags --dirty 2>$(NULL) || echo "")

OPT ?= -O3
CFLAGS += -std=gnu2x -pedantic
CFLAGS += $(OPT)
CFLAGS += -Wall -Wextra -Wno-stringop-truncation $(DEBUG) -fPIC
CFLAGS += -DPERIPHERY_VERSION_COMMIT=\"$(COMMIT_ID)\" -DPERIPHERY_GPIO_CDEV_SUPPORT=$(GPIO_CDEV_SUPPORT)
LDFLAGS +=

ifdef CROSS_COMPILE
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
endif

###########################################################################

.PHONY: all
all: $(STATIC_LIB)

.PHONY: shared
shared: $(SHARED_LIB)

.PHONY: tests
tests: $(TEST_PROGRAMS)

.PHONY: clean
clean:
	rm -rf $(STATIC_LIB) $(SHARED_LIB) $(SHARED_LIB).$(SO_VERSION) $(SHARED_LIB).$(VERSION) $(OBJDIR) $(TEST_PROGRAMS)

###########################################################################

tests/%: tests/%.c $(STATIC_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(STATIC_LIB) -o $@ -lpthread

###########################################################################

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(STATIC_LIB): $(OBJECTS)
	$(AR) rcs $(STATIC_LIB) $(OBJECTS)

$(SHARED_LIB): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -Wl,-soname,$(SHARED_LIB).$(SO_VERSION) -o $(SHARED_LIB).$(VERSION) $(OBJECTS)
	ln -s $(SHARED_LIB).$(VERSION) $(SHARED_LIB).$(SO_VERSION)
	ln -s $(SHARED_LIB).$(SO_VERSION) $(SHARED_LIB)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

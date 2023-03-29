LIB = periphery.a
SRCS = src/gpio.c src/gpio_cdev_v2.c src/gpio_cdev_v1.c src/gpio_sysfs.c src/led.c src/pwm.c src/spi.c src/i2c.c src/mmio.c src/serial.c src/version.c

SRCDIR = src
OBJDIR = obj

TEST_PROGRAMS = $(basename $(wildcard tests/*.c))

###########################################################################

OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

GPIO_CDEV_V1_SUPPORT := $(shell ! echo -e "#include <linux/gpio.h>\n#ifndef GPIO_GET_LINEEVENT_IOCTL\n#error\n#endif" | $(CC) -E - >/dev/null 2>&1; echo $$?)
GPIO_CDEV_V2_SUPPORT := $(shell ! echo -e "#include <linux/gpio.h>\n#ifndef GPIO_V2_LINES_MAX\n#error\n#endif" | $(CC) -E - >/dev/null 2>&1; echo $$?)
GPIO_CDEV_SUPPORT = $(if $(filter 1,$(GPIO_CDEV_V2_SUPPORT)),2,$(if $(filter 1,$(GPIO_CDEV_V1_SUPPORT)),1,0))

COMMIT_ID := $(shell git describe --abbrev --always --tags --dirty 2>/dev/null || echo "")

OPT ?= -O3
CFLAGS += -std=gnu99 -pedantic
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
all: $(LIB)

.PHONY: tests
tests: $(TEST_PROGRAMS)

.PHONY: clean
clean:
	rm -rf $(LIB) $(OBJDIR) $(TEST_PROGRAMS)

###########################################################################

tests/%: tests/%.c $(LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LIB) -o $@ -lpthread

###########################################################################

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(LIB): $(OBJECTS)
	$(AR) rcs $(LIB) $(OBJECTS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@


# c-periphery v1.0.3

## C Wrappers for Linux Peripheral I/O (GPIO, SPI, I2C, MMIO, Serial)

c-periphery is a set of C wrapper functions for GPIO, SPI, I2C, MMIO, and Serial peripheral I/O interface access in userspace Linux. The c-periphery wrappers simplify and consolidate the native Linux APIs to these interfaces. c-periphery is useful in embedded Linux environments (including BeagleBone, Raspberry Pi, etc. platforms) for interfacing with external peripherals. c-periphery is re-entrant, uses static allocations, has no dependencies outside the standard C library and Linux, compiles into a static library for easy integration with other projects, and is MIT licensed.

## Examples

### GPIO

``` c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "gpio.h"

int main(void) {
    gpio_t gpio_in, gpio_out;
    bool value;

    /* Open GPIO 10 with input direction */
    if (gpio_open(&gpio_in, 10, GPIO_DIR_IN) < 0) {
        fprintf(stderr, "gpio_open(): %s\n", gpio_errmsg(&gpio_in));
        exit(1);
    }

    /* Open GPIO 12 with output direction */
    if (gpio_open(&gpio_out, 12, GPIO_DIR_OUT) < 0) {
        fprintf(stderr, "gpio_open(): %s\n", gpio_errmsg(&gpio_out));
        exit(1);
    }

    /* Read input GPIO into value */
    if (gpio_read(&gpio_in, &value) < 0) {
        fprintf(stderr, "gpio_read(): %s\n", gpio_errmsg(&gpio_in));
        exit(1);
    }

    /* Write output GPIO with !value */
    if (gpio_write(&gpio_out, !value) < 0) {
        fprintf(stderr, "gpio_write(): %s\n", gpio_errmsg(&gpio_out));
        exit(1);
    }

    gpio_close(&gpio_in);
    gpio_close(&gpio_out);
    return 0;
}
```

[Go to GPIO documentation.](docs/gpio.md)

### SPI

``` c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "spi.h"

int main(void) {
    spi_t spi;
    uint8_t buf[4] = { 0xaa, 0xbb, 0xcc, 0xdd };

    /* Open spidev1.0 with mode 0 and max speed 1MHz */
    if (spi_open(&spi, "/dev/spidev1.0", 0, 1000000) < 0) {
        fprintf(stderr, "spi_open(): %s\n", spi_errmsg(&spi));
        exit(1);
    }

    /* Shift out and in 4 bytes */
    if (spi_transfer(&spi, buf, buf, sizeof(buf)) < 0) {
        fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(&spi));
        exit(1);
    }

    printf("shifted in: 0x%02x 0x%02x 0x%02x 0x%02x\n", buf[0], buf[1], buf[2], buf[3]);

    spi_close(&spi);
    return 0;
}
```

[Go to SPI documentation.](docs/spi.md)

### I2C

``` c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "i2c.h"

#define EEPROM_I2C_ADDR 0x50

int main(void) {
    i2c_t i2c;

    /* Open the i2c-0 bus */
    if (i2c_open(&i2c, "/dev/i2c-0") < 0) {
        fprintf(stderr, "i2c_open(): %s\n", i2c_errmsg(&i2c));
        exit(1);
    }

    /* Read byte at address 0x100 of EEPROM */
    uint8_t msg_addr[2] = { 0x01, 0x00 };
    uint8_t msg_data[1] = { 0xff, };
    struct i2c_msg msgs[2] =
        {
            /* Write 16-bit address */
            { .addr = EEPROM_I2C_ADDR, .flags = 0, .len = 2, .buf = msg_addr },
            /* Read 8-bit data */
            { .addr = EEPROM_I2C_ADDR, .flags = I2C_M_RD, .len = 1, .buf = msg_data},
        };

    /* Transfer a transaction with two I2C messages */
    if (i2c_transfer(&i2c, msgs, 2) < 0) {
        fprintf(stderr, "i2c_transfer(): %s\n", i2c_errmsg(&i2c));
        exit(1);
    }

    printf("0x%02x%02x: %02x\n", msg_addr[0], msg_addr[1], msg_data[0]);

    i2c_close(&i2c);
    return 0;
}
```

[Go to I2C documentation.](docs/i2c.md)

### MMIO

``` c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <byteswap.h>

#include "mmio.h"

struct am335x_rtcss_registers {
    uint32_t seconds;       /* 0x00 */
    uint32_t minutes;       /* 0x04 */
    uint32_t hours;         /* 0x08 */
    /* ... */
};

int main(void) {
    mmio_t mmio;
    uint32_t mac_id0_lo, mac_id0_hi;
    volatile struct am335x_rtcss_registers *regs;

    /* Open Control Module */
    if (mmio_open(&mmio, 0x44E10000, 0x1000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(&mmio));
        exit(1);
    }

    /* Read lower 2 bytes of MAC address */
    if (mmio_read32(&mmio, 0x630, &mac_id0_lo) < 0) {
        fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(&mmio));
        exit(1);
    }

    /* Read upper 4 bytes of MAC address */
    if (mmio_read32(&mmio, 0x634, &mac_id0_hi) < 0) {
        fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(&mmio));
        exit(1);
    }

    printf("MAC address: %08X%04X\n", __bswap_32(mac_id0_hi), __bswap_16(mac_id0_lo));

    mmio_close(&mmio);

    /* Open RTC subsystem */
    if (mmio_open(&mmio, 0x44E3E000, 0x1000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(&mmio));
        exit(1);
    }

    regs = mmio_ptr(&mmio);

    /* Read current RTC time */
    printf("hours: %02x minutes: %02x seconds %02x\n", regs->hours, regs->minutes, regs->seconds);

    mmio_close(&mmio);

    return 0;
}
```

[Go to MMIO documentation.](docs/mmio.md)

### Serial

``` c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"

int main(void) {
    serial_t serial;
    uint8_t s[] = "Hello World!";
    uint8_t buf[128];
    int ret;

    /* Open /dev/ttyUSB0 with baudrate 115200, and defaults of 8N1, no flow control */
    if (serial_open(&serial, "/dev/ttyUSB0", 115200) < 0) {
        fprintf(stderr, "serial_open(): %s\n", serial_errmsg(&serial));
        exit(1);
    }

    /* Write to the serial port */
    if (serial_write(&serial, s, sizeof(s)) < 0) {
        fprintf(stderr, "serial_write(): %s\n", serial_errmsg(&serial));
        exit(1);
    }

    /* Read up to buf size or 2000ms timeout */
    if ((ret = serial_read(&serial, buf, sizeof(buf), 2000)) < 0) {
        fprintf(stderr, "serial_read(): %s\n", serial_errmsg(&serial));
        exit(1);
    }

    printf("read %d bytes: _%s_\n", ret, buf);

    serial_close(&serial);
    return 0;
}
```

[Go to Serial documentation.](docs/serial.md)

## Building

`make` will build c-periphery into a static library.

``` console
$ make
mkdir obj
cc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  -c src/gpio.c -o obj/gpio.o
cc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  -c src/spi.c -o obj/spi.o
cc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  -c src/i2c.c -o obj/i2c.o
cc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  -c src/mmio.c -o obj/mmio.o
cc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  -c src/serial.c -o obj/serial.o
ar rcs periphery.a obj/gpio.o obj/spi.o obj/i2c.o obj/mmio.o obj/serial.o
$
```

`make tests` will build the c-periphery tests.

``` console
$ make tests
cc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  tests/test_serial.c periphery.a -o tests/test_serial
cc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  tests/test_i2c.c periphery.a -o tests/test_i2c
cc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  tests/test_mmio.c periphery.a -o tests/test_mmio
cc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  tests/test_spi.c periphery.a -o tests/test_spi
cc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  tests/test_gpio.c periphery.a -o tests/test_gpio
$
```

### Cross-compilation

Set the `CC` environment variable with the cross-compiler when calling make:

``` console
$ CC=arm-linux-gcc make clean all tests
rm -rf periphery.a obj tests/test_serial tests/test_i2c tests/test_mmio tests/test_spi tests/test_gpio
mkdir obj
arm-linux-gcc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  -c src/gpio.c -o obj/gpio.o
arm-linux-gcc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  -c src/spi.c -o obj/spi.o
arm-linux-gcc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  -c src/i2c.c -o obj/i2c.o
arm-linux-gcc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  -c src/mmio.c -o obj/mmio.o
arm-linux-gcc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  -c src/serial.c -o obj/serial.o
ar rcs periphery.a obj/gpio.o obj/spi.o obj/i2c.o obj/mmio.o obj/serial.o
arm-linux-gcc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  tests/test_serial.c periphery.a -o tests/test_serial
arm-linux-gcc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  tests/test_i2c.c periphery.a -o tests/test_i2c
arm-linux-gcc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  tests/test_mmio.c periphery.a -o tests/test_mmio
arm-linux-gcc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  tests/test_spi.c periphery.a -o tests/test_spi
arm-linux-gcc -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast  -fPIC  tests/test_gpio.c periphery.a -o tests/test_gpio
$
```

## Building c-periphery into another project

Include the header files in `src/` (e.g. `gpio.h`, `spi.h`, `i2c.h`, `mmio.h`, `serial.h`) and link in the `periphery.a` static library.

``` console
$ gcc -I/path/to/periphery/src myprog.c /path/to/periphery/periphery.a -o myprog
```

## Documentation

`man` page style documentation for each interface wrapper is available in [docs](docs/) folder.

## Testing

The tests located in the [tests](tests/) folder may be run to test the correctness and functionality of the peripheral I/O wrappers. Some tests require interactive probing (e.g. with an oscilloscope), the installation of a physical loopback, or the existence of a particular device on a bus. See the usage of each test for more details on the required test setup.

## License

c-periphery is MIT licensed. See the included [LICENSE](LICENSE) file.


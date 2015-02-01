/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include "test.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../src/i2c.h"

const char *i2c_bus_path;
uint8_t eeprom_address;

void test_arguments(void) {
    ptest();

    /* No real argument validation needed in the i2c wrapper */
}

void test_open_config_close(void) {
    i2c_t i2c;

    ptest();

    /* Open invalid i2c bus */
    passert(i2c_open(&i2c, "/foo/bar") == I2C_ERROR_OPEN);

    /* Open legitimate i2c bus */
    passert(i2c_open(&i2c, i2c_bus_path) == 0);
    passert(i2c_close(&i2c) == 0);
}

void test_loopback(void) {
    i2c_t i2c;
    char sysfs_path[128];
    struct stat stat_buf;
    uint8_t eeprom[256];
    bool success;
    int fd;
    unsigned int i;

    ptest();

    /* "Loopback" plan:
     * 1. Read EEPROM via sysfs
     * 2. Read it ourselves via i2c_*() and compare data */

    /* Read EEPROM via sysfs */

    snprintf(sysfs_path, sizeof(sysfs_path), "/sys/bus/i2c/devices/%s-00%02x/eeprom", i2c_bus_path+(strlen(i2c_bus_path)-1), eeprom_address);
    passert(stat(sysfs_path, &stat_buf) == 0);
    passert((fd = open(sysfs_path, O_RDONLY)) >= 0);
    passert(read(fd, eeprom, sizeof(eeprom)) == sizeof(eeprom));
    passert(close(fd) == 0);

    /* Read it ourselves and compare */

    passert(i2c_open(&i2c, i2c_bus_path) == 0);

    struct i2c_msg msgs[2];
    uint8_t msg_addr[2];
    uint8_t msg_data[1];

    msgs[0].addr = eeprom_address;
    msgs[0].flags = 0; /* Write */
    msgs[0].len = 2;
    msgs[0].buf = msg_addr;
    msgs[1].addr = eeprom_address;
    msgs[1].flags = I2C_M_RD; /* Read */
    msgs[1].len = 1;
    msgs[1].buf = msg_data;

    success = true;

    for (i = 0; i < sizeof(eeprom); i++) {
        msg_addr[0] = 0;
        msg_addr[1] = i;

        if (i2c_transfer(&i2c, msgs, 2) < 0) {
            printf("Error with I2C transaction: %s\n", i2c_errmsg(&i2c));
            success = false;
            break;
        }

        if (msg_data[0] != eeprom[i]) {
            printf("Error: Data mismatch at address %d (expected %02x, got %02x)\n", i, eeprom[i], msg_data[0]);
            success = false;
        }
    }

    passert(i2c_close(&i2c) == 0);

    passert(success);
}

bool getc_yes(void) {
    char buf[4];
    fgets(buf, sizeof(buf), stdin);
    return (buf[0] == 'y' || buf[0] == 'Y');
}

void test_interactive(void) {
    i2c_t i2c;
    uint8_t msg1[] = { 0xaa, 0xbb, 0xcc, 0xdd };
    struct i2c_msg msgs[1];

    ptest();

    passert(i2c_open(&i2c, i2c_bus_path) == 0);

    printf("Starting interactive test. Get out your logic analyzer, buddy!\n");
    printf("Press enter to continue...\n");
    getc(stdin);

    /* There isn't much we can do without assuming a device on the other end,
     * because I2C needs an acknowledgement bit on each transferred byte.
     *
     * But we can send a transaction and expect it to time out. */

    /* S [ 0x7a W ] [0xaa] [0xbb] [0xcc] [0xdd] */
    msgs[0].addr = 0x7a;
    msgs[0].flags = 0; /* Write */
    msgs[0].len = sizeof(msg1);
    msgs[0].buf = msg1;

    printf("Press enter to start transfer...");
    getc(stdin);
    passert(i2c_transfer(&i2c, msgs, 1) < 0);
    passert(i2c_errno(&i2c) == EREMOTEIO);
    passert(i2c_close(&i2c) == 0);
    printf("I2C transfer occurred? y/n\n");
    passert(getc_yes());
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <I2C bus #1> <EEPROM address> <I2C bus #2>\n\n", argv[0]);
        fprintf(stderr, "[1/4] Arguments test: No requirements.\n");
        fprintf(stderr, "[2/4] Open/close test: I2C bus #1 should be real.\n");
        fprintf(stderr, "[3/4] Loopback test: I2C bus #1 should have an EEPROM attached.\n");
        fprintf(stderr, "[4/4] Interactive test: I2C bus #2 should be observed with a logic analyzer.\n\n");
        fprintf(stderr, "Hint: for BeagleBone Black, use onboard EEPROM on /dev/i2c-0, and export I2C1 to /dev/i2c-2 with:\n");
        fprintf(stderr, "    echo BB-I2C1A1 > /sys/devices/bone_capemgr.9/slots\n");
        fprintf(stderr, "to enable I2C1 (SCL=P9.24, SDA=P9.26), then run this test:\n");
        fprintf(stderr, "    %s /dev/i2c-0 0x50 /dev/i2c-2\n\n", argv[0]);
        exit(1);
    }

    i2c_bus_path = argv[1];
    eeprom_address = strtoul(argv[2], NULL, 0);

    test_arguments();
    printf(" " STR_OK "  Arguments test passed.\n\n");
    test_open_config_close();
    printf(" " STR_OK "  Open/close test passed.\n\n");
    test_loopback();
    printf(" " STR_OK "  Loopback test passed.\n\n");
    i2c_bus_path = argv[3];
    test_interactive();
    printf(" " STR_OK "  Interactive test passed.\n\n");

    printf("All tests passed!\n");
    return 0;
}


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

#define I2C_EEPROM_ADDRESS      0x51

const char *i2c_bus_path;

void test_arguments(void) {
    ptest();

    /* No real argument validation needed in the i2c wrapper */
}

void test_open_config_close(void) {
    i2c_t *i2c;

    ptest();

    /* Allocate I2C */
    i2c = i2c_new();
    passert(i2c != NULL);

    /* Open invalid i2c bus */
    passert(i2c_open(i2c, "/foo/bar") == I2C_ERROR_OPEN);

    /* Open legitimate i2c bus */
    passert(i2c_open(i2c, i2c_bus_path) == 0);
    passert(i2c_close(i2c) == 0);

    /* Free I2C */
    i2c_free(i2c);
}

void test_loopback(void) {
    i2c_t *i2c;
    uint8_t vector[32];
    uint8_t buf[2 + 32];
    unsigned int i;
    struct i2c_msg msgs[2];

    ptest();

    /* Allocate I2C */
    i2c = i2c_new();
    passert(i2c != NULL);

    passert(i2c_open(i2c, i2c_bus_path) == 0);

    /* Generate random byte vector */
    srandom(1234);
    for (i = 0; i < sizeof(vector); i++) {
        vector[i] = (uint8_t)random();
    }

    /* Write bytes to 0x100 */
    /* S [ 0x51 W ] [ 0x01 ] [ 0x00 ] [ Data... ] P */
    buf[0] = 0x01;
    buf[1] = 0x00;
    memcpy(buf + 2, vector, sizeof(vector));
    msgs[0].addr = I2C_EEPROM_ADDRESS;
    msgs[0].flags = 0; /* Write */
    msgs[0].len = sizeof(buf);
    msgs[0].buf = buf;
    passert(i2c_transfer(i2c, msgs, 1) == 0);

    /* Wait for Write Cycle */
    usleep(10000);

    /* Read bytes from 0x100 */
    /* S [ 0x51 W ] [ 0x01 ] [ 0x00 ] S [ 0x51 R ] [ Data... ] P */
    buf[0] = 0x01;
    buf[1] = 0x00;
    memset(buf + 2, 0, sizeof(vector));
    msgs[0].addr = I2C_EEPROM_ADDRESS;
    msgs[0].flags = 0; /* Write */
    msgs[0].len = 2;
    msgs[0].buf = buf;
    msgs[1].addr = I2C_EEPROM_ADDRESS;
    msgs[1].flags = I2C_M_RD; /* Read */
    msgs[1].len = sizeof(vector);
    msgs[1].buf = buf + 2;
    passert(i2c_transfer(i2c, msgs, 2) == 0);

    /* Verify bytes */
    passert(memcmp(buf + 2, vector, sizeof(vector)) == 0);

    /* Close I2C */
    passert(i2c_close(i2c) == 0);

    /* Free I2C */
    i2c_free(i2c);
}

bool getc_yes(void) {
    char buf[4];
    fgets(buf, sizeof(buf), stdin);
    return (buf[0] == 'y' || buf[0] == 'Y');
}

void test_interactive(void) {
    char str[256];
    i2c_t *i2c;
    uint8_t msg1[] = { 0xaa, 0xbb, 0xcc, 0xdd };
    struct i2c_msg msgs[1];

    ptest();

    /* Allocate I2C */
    i2c = i2c_new();
    passert(i2c != NULL);

    passert(i2c_open(i2c, i2c_bus_path) == 0);

    printf("Starting interactive test. Get out your logic analyzer, buddy!\n");
    printf("Press enter to continue...\n");
    getc(stdin);

    /* Check tostring */
    passert(i2c_tostring(i2c, str, sizeof(str)) > 0);
    printf("I2C description: %s\n", str);
    printf("I2C description looks OK? y/n\n");
    passert(getc_yes());

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
    passert(i2c_transfer(i2c, msgs, 1) < 0);
    passert(i2c_errno(i2c) > 0);
    printf("I2C transfer occurred? y/n\n");
    passert(getc_yes());

    passert(i2c_close(i2c) == 0);

    /* Free I2C */
    i2c_free(i2c);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <I2C device>\n\n", argv[0]);
        fprintf(stderr, "[1/4] Arguments test: No requirements.\n");
        fprintf(stderr, "[2/4] Open/close test: I2C device should be real.\n");
        fprintf(stderr, "[3/4] Loopback test: Expects 24XX32 EEPROM (or similar) at address 0x51.\n");
        fprintf(stderr, "[4/4] Interactive test: I2C bus should be observed with an oscilloscope or logic analyzer.\n\n");
        fprintf(stderr, "Hint: for Raspberry Pi 3, enable I2C1 with:\n");
        fprintf(stderr, "   $ echo \"dtparam=i2c_arm=on\" | sudo tee -a /boot/firmware/config.txt\n");
        fprintf(stderr, "   $ sudo reboot\n");
        fprintf(stderr, "Use pins I2C1 SDA (header pin 2) and I2C1 SCL (header pin 3),\n");
        fprintf(stderr, "and run this test with:\n");
        fprintf(stderr, "    %s /dev/i2c-1\n\n", argv[0]);
        exit(1);
    }

    i2c_bus_path = argv[1];

    test_arguments();
    printf(" " STR_OK "  Arguments test passed.\n\n");
    test_open_config_close();
    printf(" " STR_OK "  Open/close test passed.\n\n");
    test_loopback();
    printf(" " STR_OK "  Loopback test passed.\n\n");
    test_interactive();
    printf(" " STR_OK "  Interactive test passed.\n\n");

    printf("All tests passed!\n");
    return 0;
}


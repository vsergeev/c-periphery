/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include "test.h"

#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>

#include "../src/serial.h"

const char *device;

void test_arguments(void) {
    serial_t serial;

    ptest();

    /* Invalid data bits (4 and 9) */
    passert(serial_open_advanced(&serial, device, 115200, 4, PARITY_NONE, 1, false, false) == SERIAL_ERROR_ARG);
    passert(serial_open_advanced(&serial, device, 115200, 9, PARITY_NONE, 1, false, false) == SERIAL_ERROR_ARG);
    /* Invalid parity */
    passert(serial_open_advanced(&serial, device, 115200, 8, PARITY_EVEN+1, 1, false, false) == SERIAL_ERROR_ARG);
    /* Invalid stopbits */
    passert(serial_open_advanced(&serial, device, 115200, 8, PARITY_NONE, 0, false, false) == SERIAL_ERROR_ARG);
    passert(serial_open_advanced(&serial, device, 115200, 8, PARITY_NONE, 3, false, false) == SERIAL_ERROR_ARG);

    /* Everything else is fair game, although termios might not like it. */
}

void test_open_config_close(void) {
    serial_t serial;
    uint32_t baudrate;
    unsigned int databits;
    serial_parity_t parity;
    unsigned int stopbits;
    bool xonxoff;
    bool rtscts;

    ptest();

    passert(serial_open(&serial, device, 115200) == 0);

    /* Check default settings */
    passert(serial_get_baudrate(&serial, &baudrate) == 0);
    passert(baudrate == 115200);
    passert(serial_get_databits(&serial, &databits) == 0);
    passert(databits == 8);
    passert(serial_get_parity(&serial, &parity) == 0);
    passert(parity == PARITY_NONE);
    passert(serial_get_stopbits(&serial, &stopbits) == 0);
    passert(stopbits == 1);
    passert(serial_get_xonxoff(&serial, &xonxoff) == 0);
    passert(xonxoff == false);
    passert(serial_get_rtscts(&serial, &rtscts) == 0);
    passert(rtscts == false);

    /* Change some stuff around */
    passert(serial_set_baudrate(&serial, 4800) == 0);
    passert(serial_get_baudrate(&serial, &baudrate) == 0);
    passert(baudrate == 4800);
    passert(serial_set_baudrate(&serial, 9600) == 0);
    passert(serial_get_baudrate(&serial, &baudrate) == 0);
    passert(baudrate == 9600);
    passert(serial_set_databits(&serial, 7) == 0);
    passert(serial_get_databits(&serial, &databits) == 0);
    passert(databits == 7);
    passert(serial_set_parity(&serial, PARITY_ODD) == 0);
    passert(serial_get_parity(&serial, &parity) == 0);
    passert(parity == PARITY_ODD);
    passert(serial_set_stopbits(&serial, 2) == 0);
    passert(serial_get_stopbits(&serial, &stopbits) == 0);
    passert(stopbits == 2);
    passert(serial_set_xonxoff(&serial, true) == 0);
    passert(serial_get_xonxoff(&serial, &xonxoff) == 0);
    passert(xonxoff == true);
    #if 0
    passert(serial_set_rtscts(&serial, true) == 0);
    passert(serial_get_rtscts(&serial, &rtscts) == 0);
    passert(rtscts == true);
    #endif
    /* Test serial port may not support rtscts */

    passert(serial_close(&serial) == 0);
}

void test_loopback(void) {
    serial_t serial;
    unsigned int count;
    time_t start, stop;
    uint8_t lorem_ipsum[] = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
    uint8_t lorem_hugesum[4096*3];
    uint8_t buf[sizeof(lorem_hugesum)];

    ptest();

    passert(serial_open(&serial, device, 115200) == 0);

    /* Test write/flush/read */
    passert(serial_write(&serial, lorem_ipsum, sizeof(lorem_ipsum)) == sizeof(lorem_ipsum));
    passert(serial_flush(&serial) == 0);
    passert(serial_read(&serial, buf, sizeof(lorem_ipsum), -1) == sizeof(lorem_ipsum));
    passert(memcmp(lorem_ipsum, buf, sizeof(lorem_ipsum)) == 0);

    /* Test poll/write/flush/poll/input waiting/read */
    passert(serial_poll(&serial, 500) == 0); /* Should timeout */
    passert(serial_write(&serial, lorem_ipsum, sizeof(lorem_ipsum)) == sizeof(lorem_ipsum));
    passert(serial_flush(&serial) == 0);
    passert(serial_poll(&serial, 500) == 1);
    usleep(500000);
    passert(serial_input_waiting(&serial, &count) == 0);
    passert(count == sizeof(lorem_ipsum));
    passert(serial_read(&serial, buf, sizeof(lorem_ipsum), -1) == sizeof(lorem_ipsum));
    passert(memcmp(lorem_ipsum, buf, sizeof(lorem_ipsum)) == 0);

    /* Test non-blocking poll */
    passert(serial_poll(&serial, 0) == 0);

    /* Test a very large read-write (likely to exceed internal buffer size (~4096)) */
    memset(lorem_hugesum, 0xAA, sizeof(lorem_hugesum));
    passert(serial_write(&serial, lorem_hugesum, sizeof(lorem_hugesum)) == sizeof(lorem_hugesum));
    passert(serial_flush(&serial) == 0);
    passert(serial_read(&serial, buf, sizeof(lorem_hugesum), -1) == sizeof(lorem_hugesum));
    passert(memcmp(lorem_hugesum, buf, sizeof(lorem_hugesum)) == 0);

    /* Test read timeout */
    start = time(NULL);
    passert(serial_read(&serial, buf, sizeof(buf), 2000) == 0);
    stop = time(NULL);
    passert((stop - start) > 1);

    /* Test non-blocking read */
    start = time(NULL);
    passert(serial_read(&serial, buf, sizeof(buf), 0) == 0);
    stop = time(NULL);
    /* Assuming we weren't context switched out for a second and weren't on a
     * thin time boundary ;) */
    passert((stop - start) == 0);

    passert(serial_close(&serial) == 0);
}

bool getc_yes(void) {
    char buf[4];
    fgets(buf, sizeof(buf), stdin);
    return (buf[0] == 'y' || buf[0] == 'Y');
}

void test_interactive(void) {
    serial_t serial;
    uint8_t buf[] = "Hello World";

    ptest();

    passert(serial_open(&serial, device, 4800) == 0);

    printf("Starting interactive test. Get out your logic analyzer, buddy!\n");
    printf("Press enter to continue...\n");
    getc(stdin);

    printf("Press enter to start transfer...");
    getc(stdin);
    passert(serial_write(&serial, buf, sizeof(buf)) == sizeof(buf));
    printf("Serial transfer baudrate 4800, 8n1 occurred? y/n\n");
    passert(getc_yes());

    passert(serial_set_baudrate(&serial, 9600) == 0);

    printf("Press enter to start transfer...");
    getc(stdin);
    passert(serial_write(&serial, buf, sizeof(buf)) == sizeof(buf));
    printf("Serial transfer baudrate 9600, 8n1 occurred? y/n\n");
    passert(getc_yes());

    passert(serial_set_baudrate(&serial, 115200) == 0);

    printf("Press enter to start transfer...");
    getc(stdin);
    passert(serial_write(&serial, buf, sizeof(buf)) == sizeof(buf));
    printf("Serial transfer baudrate 115200, 8n1 occurred? y/n\n");
    passert(getc_yes());

    passert(serial_close(&serial) == 0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <serial port device>\n\n", argv[0]);
        fprintf(stderr, "[1/4] Arguments test: No requirements.\n");
        fprintf(stderr, "[2/4] Open/close test: Serial port device should be real.\n");
        fprintf(stderr, "[3/4] Loopback test: Serial TX and RX should be connected with a wire.\n");
        fprintf(stderr, "[4/4] Interactive test: Serial TX should be observed with a logic analyzer.\n\n");
        fprintf(stderr, "Hint: for BeagleBone Black, export UART4 to /dev/ttyO4 with:\n");
        fprintf(stderr, "    echo BB-UART4 > /sys/devices/bone_capemgr.9/slots\n");
        fprintf(stderr, "to enable UART4 (TX=P9.13 RX=P9.11).\n");
        fprintf(stderr, "Connect a wire between TX and RX, then run this test:\n");
        fprintf(stderr, "    %s /dev/ttyO4\n\n", argv[0]);
        exit(1);
    }

    device = argv[1];

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


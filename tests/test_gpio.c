/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include "test.h"

#include <stdlib.h>
#include <errno.h>

#include <sys/wait.h>
#include <unistd.h>

#include "../src/gpio.h"

unsigned int pin_input, pin_output;

void test_arguments(void) {
    gpio_t *gpio;

    ptest();

    /* Allocate GPIO */
    gpio = gpio_new();
    passert(gpio != NULL);

    /* Invalid direction */
    passert(gpio_open(gpio, pin_input, 5) == GPIO_ERROR_ARG);

    /* Free GPIO */
    gpio_free(gpio);
}

void test_open_config_close(void) {
    gpio_t *gpio;
    bool value, interrupts_supported;
    gpio_direction_t direction;
    gpio_edge_t edge;

    ptest();

    /* Allocate GPIO */
    gpio = gpio_new();
    passert(gpio != NULL);

    /* Open non-existent GPIO -- export should fail with EINVAL */
    passert(gpio_open(gpio, -1, GPIO_DIR_IN) == GPIO_ERROR_EXPORT);
    passert(gpio_errno(gpio) == EINVAL);

    /* Open legitimate GPIO */
    passert(gpio_open(gpio, pin_output, GPIO_DIR_IN) == 0);

    /* Invalid direction */
    passert(gpio_set_direction(gpio, 5) == GPIO_ERROR_ARG);
    /* Invalid interrupt edge */
    passert(gpio_set_edge(gpio, 5) == GPIO_ERROR_ARG);

    /* Set direction out, check direction out, check value low */
    passert(gpio_set_direction(gpio, GPIO_DIR_OUT) == 0);
    passert(gpio_get_direction(gpio, &direction) == 0);
    passert(direction == GPIO_DIR_OUT);
    passert(gpio_read(gpio, &value) == 0);
    passert(value == false);
    /* Set direction out low, check direction out, check value low */
    passert(gpio_set_direction(gpio, GPIO_DIR_OUT_LOW) == 0);
    passert(gpio_get_direction(gpio, &direction) == 0);
    passert(direction == GPIO_DIR_OUT);
    passert(gpio_read(gpio, &value) == 0);
    passert(value == false);
    /* Set direction out high, check direction out, check value high */
    passert(gpio_set_direction(gpio, GPIO_DIR_OUT_HIGH) == 0);
    passert(gpio_get_direction(gpio, &direction) == 0);
    passert(direction == GPIO_DIR_OUT);
    passert(gpio_read(gpio, &value) == 0);
    passert(value == true);
    /* Set direction in, check direction in */
    passert(gpio_set_direction(gpio, GPIO_DIR_IN) == 0);
    passert(gpio_get_direction(gpio, &direction) == 0);
    passert(direction == GPIO_DIR_IN);
    passert(gpio_read(gpio, &value) == 0);

    /* Check interrupt edge support */
    passert(gpio_supports_interrupts(gpio, &interrupts_supported) == 0);
    passert(interrupts_supported == true);

    /* Set edge none, check edge none */
    passert(gpio_set_edge(gpio, GPIO_EDGE_NONE) == 0);
    passert(gpio_get_edge(gpio, &edge) == 0);
    passert(edge == GPIO_EDGE_NONE);
    /* Set edge rising, check edge rising */
    passert(gpio_set_edge(gpio, GPIO_EDGE_RISING) == 0);
    passert(gpio_get_edge(gpio, &edge) == 0);
    passert(edge == GPIO_EDGE_RISING);
    /* Set edge falling, check edge falling */
    passert(gpio_set_edge(gpio, GPIO_EDGE_FALLING) == 0);
    passert(gpio_get_edge(gpio, &edge) == 0);
    passert(edge == GPIO_EDGE_FALLING);
    /* Set edge both, check edge both */
    passert(gpio_set_edge(gpio, GPIO_EDGE_BOTH) == 0);
    passert(gpio_get_edge(gpio, &edge) == 0);
    passert(edge == GPIO_EDGE_BOTH);
    /* Set edge none, check edge none */
    passert(gpio_set_edge(gpio, GPIO_EDGE_NONE) == 0);
    passert(gpio_get_edge(gpio, &edge) == 0);
    passert(edge == GPIO_EDGE_NONE);

    /* Close GPIO */
    passert(gpio_close(gpio) == 0);

    /* Open GPIO as out */
    passert(gpio_open(gpio, pin_output, GPIO_DIR_OUT) == 0);
    /* Close GPIO */
    passert(gpio_close(gpio) == 0);
    /* Open GPIO with preserved direction */
    passert(gpio_open(gpio, pin_output, GPIO_DIR_PRESERVE) == 0);
    /* Check direction is still out */
    passert(gpio_get_direction(gpio, &direction) == 0);
    passert(direction == GPIO_DIR_OUT);

    /* Close GPIO */
    passert(gpio_close(gpio) == 0);

    /* Free GPIO */
    gpio_free(gpio);
}

void test_loopback(void) {
    gpio_t *gpio_in, *gpio_out;
    bool value;

    ptest();

    /* Allocate GPIO */
    gpio_in = gpio_new();
    passert(gpio_in != NULL);
    gpio_out = gpio_new();
    passert(gpio_out != NULL);

    /* Open in and out pins */
    passert(gpio_open(gpio_in, pin_input, GPIO_DIR_IN) == 0);
    passert(gpio_open(gpio_out, pin_output, GPIO_DIR_OUT) == 0);

    /* Drive out low, check in low */
    passert(gpio_write(gpio_out, false) == 0);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == false);

    /* Drive out high, check in high */
    passert(gpio_write(gpio_out, true) == 0);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == true);

    /* Check poll falling 1 -> 0 interrupt */
    passert(gpio_set_edge(gpio_in, GPIO_EDGE_FALLING) == 0);
    passert(gpio_write(gpio_out, false) == 0);
    passert(gpio_poll(gpio_in, 1000) == 1);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == false);

    /* Check poll rising 0 -> 1 interrupt */
    passert(gpio_set_edge(gpio_in, GPIO_EDGE_RISING) == 0);
    passert(gpio_write(gpio_out, true) == 0);
    passert(gpio_poll(gpio_in, 1000) == 1);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == true);

    /* Check poll timeout */
    passert(gpio_poll(gpio_in, 1000) == 0);

    passert(gpio_close(gpio_in) == 0);
    passert(gpio_close(gpio_out) == 0);

    /* Free GPIO */
    gpio_free(gpio_in);
    gpio_free(gpio_out);
}

bool getc_yes(void) {
    char buf[4];
    fgets(buf, sizeof(buf), stdin);
    return (buf[0] == 'y' || buf[0] == 'Y');
}

void test_interactive(void) {
    gpio_t *gpio;

    ptest();

    /* Allocate GPIO */
    gpio = gpio_new();
    passert(gpio != NULL);

    passert(gpio_open(gpio, pin_output, GPIO_DIR_OUT) == 0);

    printf("Starting interactive test. Get out your multimeter, buddy!\n");
    printf("Press enter to continue...\n");
    getc(stdin);

    /* Drive GPIO out low */
    passert(gpio_write(gpio, false) == 0);
    printf("GPIO out is low? y/n\n");
    passert(getc_yes());

    /* Drive GPIO out high */
    passert(gpio_write(gpio, true) == 0);
    printf("GPIO out is high? y/n\n");
    passert(getc_yes());

    /* Drive GPIO out low */
    passert(gpio_write(gpio, false) == 0);
    printf("GPIO out is low? y/n\n");
    passert(getc_yes());

    passert(gpio_close(gpio) == 0);

    /* Free GPIO */
    gpio_free(gpio);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <GPIO #1> <GPIO #2>\n\n", argv[0]);
        fprintf(stderr, "[1/4] Argument test: No requirements.\n");
        fprintf(stderr, "[2/4] Open/close test: GPIO #2 should be real.\n");
        fprintf(stderr, "[3/4] Loopback test: GPIOs #1 and #2 should be connected with a wire.\n");
        fprintf(stderr, "[4/4] Interactive test: GPIO #2 should be observed with a multimeter.\n\n");
        fprintf(stderr, "Hint: for Raspberry Pi 3,\n");
        fprintf(stderr, "Use GPIO 17 (header pin 11) and GPIO 27 (header pin 13),\n");
        fprintf(stderr, "connect a loopback between them, and run this test with:\n");
        fprintf(stderr, "    %s 17 27\n\n", argv[0]);
        exit(1);
    }

    pin_input = strtoul(argv[1], NULL, 10);
    pin_output = strtoul(argv[2], NULL, 10);

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


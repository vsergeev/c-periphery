/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include "test.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "../src/led.h"

const char *device;

void test_arguments(void) {
    ptest();

    /* No real argument validation needed in the LED wrapper */
}

void test_open_config_close(void) {
    led_t *led;
    char name[64];
    char trigger[64];
    unsigned int max_brightness;
    unsigned int brightness;
    unsigned int triggers_count;
    bool value;

    ptest();

    /* Allocate LED */
    led = led_new();
    passert(led != NULL);

    /* Open non-existent LED */
    passert(led_open(led, "nonexistent") == LED_ERROR_OPEN);

    /* Open legitimate LED */
    passert(led_open(led, device) == 0);

    /* Check properties */
    passert(led_name(led, name, sizeof(name)) == 0);
    passert(strcmp(name, device) == 0);

    /* Check max brightness */
    passert(led_get_max_brightness(led, &max_brightness) == 0);
    passert(max_brightness > 0);

    /* Check setting invalid brightness */
    passert(led_set_brightness(led, max_brightness + 1) == LED_ERROR_ARG);

    /* Read trigger */
    passert(led_get_trigger(led, trigger, sizeof(trigger)) == 0);
    /* Read triggers count */
    passert(led_get_triggers_count(led, &triggers_count) == 0);
    passert(triggers_count > 0);
    /* Read each trigger */
    for (unsigned int i = 0; i < triggers_count; i++) {
        passert(led_get_triggers_entry(led, i, trigger, sizeof(trigger)) == 0);
    }

    /* Write true, read true, check brightness is non-zero */
    passert(led_write(led, true) == 0);
    usleep(10000);
    passert(led_read(led, &value) == 0);
    passert(value == true);
    passert(led_get_brightness(led, &brightness) == 0);
    passert(brightness > 0);

    /* Write false, read false, check brightness is zero */
    passert(led_write(led, false) == 0);
    usleep(10000);
    passert(led_read(led, &value) == 0);
    passert(value == false);
    passert(led_get_brightness(led, &brightness) == 0);
    passert(brightness == 0);

    /* Set brightness to 1, check brightness */
    passert(led_set_brightness(led, 1) == 0);
    usleep(10000);
    passert(led_get_brightness(led, &brightness) == 0);
    passert(brightness >= 1);

    /* Set brightness to 0, check brightness */
    passert(led_set_brightness(led, 0) == 0);
    usleep(10000);
    passert(led_get_brightness(led, &brightness) == 0);
    passert(brightness == 0);

    /* Set trigger to default, check isn't none */
    passert(led_set_trigger(led, "default") == 0);
    passert(led_get_trigger(led, trigger, sizeof(trigger)) == 0);
    passert(strcmp(trigger, "none") != 0);

    /* Set trigger to none, check trigger */
    passert(led_set_trigger(led, "none") == 0);
    passert(led_get_trigger(led, trigger, sizeof(trigger)) == 0);
    passert(strcmp(trigger, "none") == 0);

    passert(led_close(led) == 0);

    /* Free LED */
    led_free(led);
}

void test_loopback(void) {
    ptest();
}

bool getc_yes(void) {
    char buf[4];
    fgets(buf, sizeof(buf), stdin);
    return (buf[0] == 'y' || buf[0] == 'Y');
}

void test_interactive(void) {
    char str[256];
    char trigger[64];
    unsigned int triggers_count;
    led_t *led;

    ptest();

    /* Allocate LED */
    led = led_new();
    passert(led != NULL);

    passert(led_open(led, device) == 0);

    printf("Starting interactive test...\n");
    printf("Press enter to continue...\n");
    getc(stdin);

    /* Check tostring */
    passert(led_tostring(led, str, sizeof(str)) > 0);
    printf("LED description: %s\n", str);
    printf("LED description looks OK? y/n\n");
    passert(getc_yes());

    /* Check triggers */
    passert(led_get_trigger(led, trigger, sizeof(trigger)) == 0);
    passert(led_get_triggers_count(led, &triggers_count) == 0);
    printf("LED active trigger: %s\n", trigger);
    printf("LED available triggers (%d): ", triggers_count);
    for (unsigned int i = 0; i < triggers_count; i++) {
        assert(led_get_triggers_entry(led, i, trigger, sizeof(trigger)) == 0);
        printf("%s ", trigger);
    }
    printf("\nLED triggers look OK? y/n\n");
    passert(getc_yes());

    /* Turn LED off */
    passert(led_write(led, false) == 0);
    printf("LED is off? y/n\n");
    passert(getc_yes());

    /* Turn LED on */
    passert(led_write(led, true) == 0);
    printf("LED is on? y/n\n");
    passert(getc_yes());

    /* Turn LED off */
    passert(led_write(led, false) == 0);
    printf("LED is off? y/n\n");
    passert(getc_yes());

    /* Turn LED on */
    passert(led_write(led, true) == 0);
    printf("LED is on? y/n\n");
    passert(getc_yes());

    /* Restore default trigger */
    passert(led_set_trigger(led, "default") == 0);

    passert(led_close(led) == 0);

    /* Free LED */
    led_free(led);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <LED name>\n\n", argv[0]);
        fprintf(stderr, "[1/4] Arguments test: No requirements.\n");
        fprintf(stderr, "[2/4] Open/close test: LED should be real.\n");
        fprintf(stderr, "[3/4] Loopback test: No test.\n");
        fprintf(stderr, "[4/4] Interactive test: LED should be observed.\n\n");
        fprintf(stderr, "Hint: for Raspberry Pi 3, disable triggers for PWR:\n");
        fprintf(stderr, "    $ echo none > /sys/class/leds/PWR/trigger\n");
        fprintf(stderr, "Observe PWR (red power LED), and run this test:\n");
        fprintf(stderr, "    %s PWR\n\n", argv[0]);
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


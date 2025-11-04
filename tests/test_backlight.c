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

#include "../src/backlight.h"

const char *device;

void test_arguments(void) {
    ptest();

    /* No real argument validation needed in the BACKLIGHT wrapper */
}

void test_open_config_close(void) {
    backlight_t *backlight;
    char name[64];
    unsigned int max_brightness;
    unsigned int brightness;
    bool value;

    ptest();

    /* Allocate BACKLIGHT */
    backlight = backlight_new();
    passert(backlight != NULL);

    /* Open non-existent BACKLIGHT */
    passert(backlight_open(backlight, "nonexistent") == BL_ERROR_OPEN);

    /* Open legitimate BACKLIGHT */
    passert(backlight_open(backlight, device) == 0);


    /* Check properties */
    passert(backlight_name(backlight, name, sizeof(name)) == 0);
    passert(strcmp(name, device) == 0);
    /* Check max brightness */
    passert(backlight_get_max_brightness(backlight, &max_brightness) == 0);
    passert(max_brightness > 0);

    /* Check setting invalid brightness */
    passert(backlight_set_brightness(backlight, max_brightness + 1) == BL_ERROR_ARG);

    /* Write true, read true, check brightness is max */
    passert(backlight_write(backlight, true) == 0);
    usleep(10000);
    passert(backlight_read(backlight, &value) == 0);
    passert(value == true);
    passert(backlight_get_brightness(backlight, &brightness) == 0);
    passert(brightness == max_brightness);

    /* Write false, read false, check brightness is zero */
    passert(backlight_write(backlight, false) == 0);
    usleep(10000);
    passert(backlight_read(backlight, &value) == 0);
    passert(value == false);
    passert(backlight_get_brightness(backlight, &brightness) == 0);
    passert(brightness == 0);

    /* Set brightness to 0, check brightness */
    passert(backlight_set_brightness(backlight, 0) == 0);
    sleep(1);
    passert(backlight_get_brightness(backlight, &brightness) == 0);
    passert(brightness == 0);

    /* Set brightness to max_brightness, check brightness */
    passert(backlight_set_brightness(backlight, max_brightness) == 0);
    sleep(1);
    passert(backlight_get_brightness(backlight, &brightness) == 0);
    passert(brightness >= max_brightness);


    passert(backlight_close(backlight) == 0);
    
    /* Free BACKLIGHT */
    backlight_free(backlight);
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
    backlight_t *backlight;
    unsigned int bl_val;
    unsigned int max_brightness;

    ptest();

    /* Allocate BACKLIGHT */
    backlight = backlight_new();
    passert(backlight != NULL);

    passert(backlight_open(backlight, device) == 0);

    printf("Starting interactive test...\n");
    printf("Press enter to continue...\n");
    getc(stdin);

    /* Check tostring */
    passert(backlight_tostring(backlight, str, sizeof(str)) > 0);
    printf("BACKLIGHT description: %s\n", str);
    printf("BACKLIGHT description looks OK? y/n\n");
    passert(getc_yes());

    /* Turn BACKLIGHT off */
    passert(backlight_write(backlight, false) == 0);
    printf("BACKLIGHT is off? y/n\n");
    passert(getc_yes());

    /* Turn BACKLIGHT on */
    passert(backlight_write(backlight, true) == 0);
    printf("BACKLIGHT is on? y/n\n");
    passert(getc_yes());

    /* Check max brightness */
    passert(backlight_get_max_brightness(backlight, &max_brightness) == 0);
    passert(max_brightness > 0);

    /* Turn BACKLIGHT off */
    bl_val = max_brightness * 10 / 100;
    passert(backlight_set_brightness(backlight, bl_val) == 0);
    printf("Has the brightness changed (10%%)? y/n\n");
    passert(getc_yes());

    /* Turn BACKLIGHT on */
    bl_val = max_brightness * 80 / 100;
    passert(backlight_set_brightness(backlight, bl_val) == 0);
    printf("Has the brightness changed (80%%)? y/n\n");
    passert(getc_yes());

    passert(backlight_close(backlight) == 0);

    /* Free BACKLIGHT */
    backlight_free(backlight);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <BACKLIGHT name>\n\n", argv[0]);
        fprintf(stderr, "[1/4] Arguments test: No requirements.\n");
        fprintf(stderr, "[2/4] Open/close test: BACKLIGHT should be real.\n");
        fprintf(stderr, "[3/4] Loopback test: No test.\n");
        fprintf(stderr, "[4/4] Interactive test: BACKLIGHT should be observed.\n\n");
        fprintf(stderr, "Please first verify your backlight device. (ls /sys/class/backlight):\n");
        fprintf(stderr, "Example command :\n");
        fprintf(stderr, "    %s sgm3735-backlight\n\n", argv[0]);
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


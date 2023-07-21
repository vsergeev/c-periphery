### NAME

BACKLIGHT wrapper functions for Linux userspace sysfs backlights.

### SYNOPSIS

``` c
#include <periphery/backlight.h>

/* Primary Functions */
backlight_t *backlight_new(void);
int backlight_open(backlight_t *bl, const char *name);
int backlight_read(backlight_t *bl, bool *value);
int backlight_write(backlight_t *bl, bool value);
int backlight_close(backlight_t *bl);
void backlight_free(backlight_t *bl);

/* Getters */
int backlight_get_brightness(backlight_t *bl, unsigned int *brightness);
int backlight_get_max_brightness(backlight_t *bl, unsigned int *max_brightness);

/* Setters */
int backlight_set_brightness(backlight_t *bl, unsigned int brightness);

/* Miscellaneous */
int backlight_name(backlight_t *bl, char *str, size_t len);
int backlight_tostring(backlight_t *bl, char *str, size_t len);

/* Error Handling */
int backlight_errno(backlight_t *bl);
const char *backlight_errmsg(backlight_t *bl);
```

### DESCRIPTION

``` c
backlight_t *backlight_new(void);
```
Allocate an BACKLIGHT handle.

Returns a valid handle on success, or NULL on failure.

------

``` c
int backlight_open(backlight_t *bl, const char *name);
```
Open the sysfs BACKLIGHT with the specified name.

`backlight` should be a valid pointer to an allocated BACKLIGHT handle structure.

Returns 0 on success, or a negative [BACKLIGHT error code](#return-value) on failure.

------

``` c
int backlight_read(backlight_t *bl, bool *value);
```
Read the state of the BACKLIGHT into `value`, where `true` is non-zero brightness, and `false` is zero brightness.

`backlight` should be a valid pointer to an BACKLIGHT handle opened with `backlight_open()`. `value` should be a pointer to an allocated bool.

Returns 0 on success, or a negative [BACKLIGHT error code](#return-value) on failure.

------

``` c
int backlight_write(backlight_t *bl, bool value);
```
Write the state of the BACKLIGHT to `value`, where `true` is max brightness, and `false` is zero brightness.

`backlight` should be a valid pointer to an BACKLIGHT handle opened with `backlight_open()`.

Returns 0 on success, or a negative [BACKLIGHT error code](#return-value) on failure.

------

``` c
int backlight_close(backlight_t *bl);
```
Close the BACKLIGHT.

`backlight` should be a valid pointer to an BACKLIGHT handle opened with `backlight_open()`.

Returns 0 on success, or a negative [BACKLIGHT error code](#return-value) on failure.

------

``` c
void backlight_free(backlight_t *bl);
```
Free an BACKLIGHT handle.

------

``` c
int backlight_get_brightness(backlight_t *bl, unsigned int *brightness);
```
Get the brightness of the BACKLIGHT.

`backlight` should be a valid pointer to an BACKLIGHT handle opened with `backlight_open()`.

Returns 0 on success, or a negative [BACKLIGHT error code](#return-value) on failure.

------

``` c
int backlight_get_max_brightness(backlight_t *bl, unsigned int *max_brightness);
```
Get the max brightness of the BACKLIGHT.

`backlight` should be a valid pointer to an BACKLIGHT handle opened with `backlight_open()`.

Returns 0 on success, or a negative [BACKLIGHT error code](#return-value) on failure.

------

``` c
int backlight_set_brightness(backlight_t *bl, unsigned int brightness);
```
Set the brightness of the BACKLIGHT.

`backlight` should be a valid pointer to an BACKLIGHT handle opened with `backlight_open()`.

Returns 0 on success, or a negative [BACKLIGHT error code](#return-value) on failure.

------

``` c
int backlight_name(backlight_t *bl, char *str, size_t len);
```
Return the name of the sysfs BACKLIGHT.

`backlight` should be a valid pointer to an BACKLIGHT handle opened with `backlight_open()`.

Returns 0 on success, or a negative [BACKLIGHT error code](#return-value) on failure.

------

``` c
int backlight_tostring(backlight_t *bl, char *str, size_t len);
```
Return a string representation of the BACKLIGHT handle.

`backlight` should be a valid pointer to an BACKLIGHT handle opened with `backlight_open()`.

This function behaves and returns like `snprintf()`.

------

``` c
int backlight_errno(backlight_t *bl);
```
Return the libc errno of the last failure that occurred.

`backlight` should be a valid pointer to an BACKLIGHT handle opened with `backlight_open()`.

------

``` c
const char *backlight_errmsg(backlight_t *bl);
```
Return a human readable error message of the last failure that occurred.

`backlight` should be a valid pointer to an BACKLIGHT handle opened with `backlight_open()`.

### RETURN VALUE

The periphery BACKLIGHT functions return 0 on success or one of the negative error codes below on failure.

The libc errno of the failure in an underlying libc library call can be obtained with the `backlight_errno()` helper function. A human readable error message can be obtained with the `backlight_errmsg()` helper function.

| Error Code            | Description                       |
|-----------------------|-----------------------------------|
| `BL_ERROR_ARG`       | Invalid arguments                 |
| `BL_ERROR_OPEN`      | Opening BACKLIGHT                       |
| `BL_ERROR_QUERY`     | Querying BACKLIGHT attributes           |
| `BL_ERROR_IO`        | Reading/writing BACKLIGHT brightness    |
| `BL_ERROR_CLOSE`     | Closing BACKLIGHT                       |

### EXAMPLE

``` c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "backlight.h"

int main(void) {
    backlight_t *bl;
    unsigned int max_brightness;
    unsigned int bl_val;

    backlight = backlight_new();

    /* Open sgm3735-backlight backlight device on /sys/class/backlight */
    if (backlight_open(backlight, "sgm3735-backlight") < 0) {
        fprintf(stderr, "backlight_open(): %s\n", backlight_errmsg(backlight));
        exit(1);
    }

    /* Turn on BACKLIGHT (set max brightness) */
    if (backlight_write(backlight, true) < 0) {
        fprintf(stderr, "backlight_write(): %s\n", backlight_errmsg(backlight));
        exit(1);
    }

    /* Get max brightness */
    if (backlight_get_max_brightness(backlight, &max_brightness) < 0) {
        fprintf(stderr, "backlight_get_max_brightness(): %s\n", backlight_errmsg(backlight));
        exit(1);
    }

    /* Set half brightness */
    if (backlight_set_brightness(backlight, max_brightness / 2) < 0) {
        fprintf(stderr, "backlight_set_brightness(): %s\n", backlight_errmsg(backlight));
        exit(1);
    }

    /* Set 10 % brightness */
    bl_val = max_brightness * 10 / 100;
    if (backlight_set_brightness(backlight, bl_val) < 0) {
        fprintf(stderr, "backlight_set_brightness(): %s\n", backlight_errmsg(backlight));
        exit(1);
    }

    backlight_close(backlight);

    backlight_free(backlight);

    return 0;
}
```


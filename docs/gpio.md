### NAME

GPIO wrapper functions for Linux userspace sysfs GPIOs.

### SYNOPSIS

``` c
#include <periphery/gpio.md>

/* Primary Functions */
int gpio_open(gpio_t *gpio, unsigned int pin, gpio_direction_t direction);
int gpio_read(gpio_t *gpio, bool *value);
int gpio_write(gpio_t *gpio, bool value);
int gpio_poll(gpio_t *gpio, int timeout_ms);
int gpio_close(gpio_t *gpio);

/* Getters */
int gpio_supports_interrupts(gpio_t *gpio, bool *supported);
int gpio_get_direction(gpio_t *gpio, gpio_direction_t *direction);
int gpio_get_edge(gpio_t *gpio, gpio_edge_t *edge);

/* Setters */
int gpio_set_direction(gpio_t *gpio, gpio_direction_t direction);
int gpio_set_edge(gpio_t *gpio, gpio_edge_t edge);

/* Miscellaneous */
unsigned int gpio_pin(gpio_t *gpio);
int gpio_fd(gpio_t *gpio);
int gpio_tostring(gpio_t *gpio, char *str, size_t len);

/* Error Handling */
int gpio_errno(gpio_t *gpio);
const char *gpio_errmsg(gpio_t *gpio);
```

### ENUMERATIONS

* `gpio_direction_t`
    * `GPIO_DIR_IN`: In
    * `GPIO_DIR_OUT`: Out, initialized to low
    * `GPIO_DIR_OUT_LOW`: Out, initialized to low
    * `GPIO_DIR_OUT_HIGH`: Out, initialized to high

* `gpio_edge_t`
    * `GPIO_EDGE_NONE`: No interrupt edge
    * `GPIO_EDGE_RISING`: Rising edge (0 -> 1 transition)
    * `GPIO_EDGE_FALLING`: Falling edge (1 -> 0 transition)
    * `GPIO_EDGE_BOTH`: Both edges (X -> !X transition)

### DESCRIPTION

``` c
int gpio_open(gpio_t *gpio, unsigned int pin, gpio_direction_t direction);
```
Open the sysfs GPIO corresponding to the specified pin, with the specified direction.

`gpio` should be a valid pointer to an allocated GPIO handle structure. `pin` is the Linux GPIO pin number. `direction` is one of the direction values enumerated [above](#enumerations).

Returns 0 on success, or a negative [GPIO error code](#return-value) on failure.

------

``` c
int gpio_read(gpio_t *gpio, bool *value);
```
Read the state of the GPIO into `value`.

`gpio` should be a valid pointer to a GPIO handle opened with `gpio_open()`. `value` should be a pointer to an allocated bool.

Returns 0 on success, or a negative [GPIO error code](#return-value) on failure.

------

``` c
int gpio_write(gpio_t *gpio, bool value);
```
Set the state of the GPIO to `value`.

`gpio` should be a valid pointer to a GPIO handle opened with `gpio_open()`.

Returns 0 on success, or a negative [GPIO error code](#return-value) on failure.

------

``` c
int gpio_poll(gpio_t *gpio, int timeout_ms);
```
Poll a GPIO for the edge event configured with `gpio_set_edge`.

`gpio` should be a valid pointer to a GPIO handle opened with `gpio_open()`. `timeout_ms` can be positive for a timeout in milliseconds, 0 for a non-blocking poll, or a negative number for a blocking poll.

Returns 1 on success (an edge event occurred), 0 on timeout, or a negative [GPIO error code](#return-value) on failure.

------

``` c
int gpio_close(gpio_t *gpio);
```
Close the sysfs GPIO.

`gpio` should be a valid pointer to a GPIO handle opened with `gpio_open()`.

Returns 0 on success, or a negative [GPIO error code](#return-value) on failure.

------

```c
int gpio_supports_interrupts(gpio_t *gpio, bool *supported);
```
Query a GPIO for edge interrupt support.

`gpio` should be a valid pointer to a GPIO handle opened with `gpio_open()`.

Returns 0 on success, or a negative [GPIO error code](#return-value) on failure.

------

```c
int gpio_get_direction(gpio_t *gpio, gpio_direction_t *direction);
int gpio_get_edge(gpio_t *gpio, gpio_edge_t *edge);
```
Query the configured direction or interrupt edge, respectively, of the GPIO.

`gpio` should be a valid pointer to a GPIO handle opened with `gpio_open()`.

Returns 0 on success, or a negative [GPIO error code](#return-value) on failure.

------

```c
int gpio_set_direction(gpio_t *gpio, gpio_direction_t direction);
int gpio_set_edge(gpio_t *gpio, gpio_edge_t edge);
```
Set the direction or interrupt edge, respectively, of the GPIO.

`gpio` should be a valid pointer to a GPIO handle opened with `gpio_open()`.

Returns 0 on success, or a negative [GPIO error code](#return-value) on failure.

------

``` c
unsigned int gpio_pin(gpio_t *gpio);
```
Return the pin the GPIO handle was opened with.

`gpio` should be a valid pointer to a GPIO handle opened with `gpio_open()` or `gpio_open_advanced()`.

This function is a simple accessor to the GPIO handle structure and always succeeds.

------

``` c
int gpio_fd(gpio_t *gpio);
```
Return the file descriptor (for the underlying sysfs GPIO "value" file) of the GPIO handle.

`gpio` should be a valid pointer to a GPIO handle opened with `gpio_open()` or `gpio_open_advanced()`.

This function is a simple accessor to the GPIO handle structure and always succeeds.

------

``` c
int gpio_tostring(gpio_t *gpio, char *str, size_t len);
```
Return a string representation of the GPIO handle.

`gpio` should be a valid pointer to a GPIO handle opened with `gpio_open()` or `gpio_open_advanced()`.

This function behaves and returns like `snprintf()`.

------

``` c
int gpio_errno(gpio_t *gpio);
```
Return the libc errno of the last failure that occurred.

`gpio` should be a valid pointer to a GPIO handle opened with `gpio_open()` or `gpio_open_advanced()`.

This function is a simple accessor to the GPIO handle structure and always succeeds.

------

``` c
const char *gpio_errmsg(gpio_t *gpio);
```
Return a human readable error message of the last failure that occurred. The returned string should not be modified by the application.

`gpio` should be a valid pointer to a GPIO handle opened with `gpio_open()` or `gpio_open_advanced()`.

This function is a simple accessor to the GPIO handle structure and always succeeds.

### RETURN VALUE

The periphery GPIO functions return 0 on success or one of the negative error codes below on failure.

The libc errno of the failure in an underlying libc library call can be obtained with the `gpio_errno()` helper function. A human readable error message can be obtained with the `gpio_errmsg()` helper function.

| Error Code                    | Description                   |
|-------------------------------|-------------------------------|
| `GPIO_ERROR_ARG`              | Invalid arguments             |
| `GPIO_ERROR_EXPORT`           | Exporting GPIO                |
| `GPIO_ERROR_OPEN`             | Opening GPIO value            |
| `GPIO_ERROR_IO`               | Reading/writing GPIO value    |
| `GPIO_ERROR_CLOSE`            | Closing GPIO value            |
| `GPIO_ERROR_SET_DIRECTION`    | Setting GPIO direction        |
| `GPIO_ERROR_GET_DIRECTION`    | Getting GPIO direction        |
| `GPIO_ERROR_SET_EDGE`         | Setting GPIO interrupt edge   |
| `GPIO_ERROR_GET_EDGE`         | Getting GPIO interrupt edge   |

### EXAMPLE

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


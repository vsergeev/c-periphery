/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#ifndef _PERIPHERY_GPIO_H
#define _PERIPHERY_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

enum gpio_error_code {
    GPIO_ERROR_ARG                  = -1, /* Invalid arguments */
    GPIO_ERROR_EXPORT               = -2, /* Exporting GPIO */
    GPIO_ERROR_OPEN                 = -3, /* Opening GPIO value */
    GPIO_ERROR_IO                   = -4, /* Reading/writing GPIO value */
    GPIO_ERROR_CLOSE                = -5, /* Closing GPIO value */
    GPIO_ERROR_SET_DIRECTION        = -6, /* Setting GPIO direction */
    GPIO_ERROR_GET_DIRECTION        = -7, /* Getting GPIO direction */
    GPIO_ERROR_SET_EDGE             = -8, /* Setting GPIO interrupt edge */
    GPIO_ERROR_GET_EDGE             = -9, /* Getting GPIO interrupt edge */
};

typedef struct gpio_handle {
    unsigned int pin;
    int fd;

    struct {
        int c_errno;
        char errmsg[96];
    } error;
} gpio_t;

typedef enum gpio_direction {
    GPIO_DIR_IN,        /* Input */
    GPIO_DIR_OUT,       /* Output, initialized to low */
    GPIO_DIR_OUT_LOW,   /* Output, initialized to low */
    GPIO_DIR_OUT_HIGH,  /* Output, initialized to high */
    GPIO_DIR_PRESERVE,  /* Preserve existing direction */
} gpio_direction_t;

typedef enum gpio_edge {
    GPIO_EDGE_NONE,     /* No interrupt edge */
    GPIO_EDGE_RISING,   /* Rising edge 0 -> 1 */
    GPIO_EDGE_FALLING,  /* Falling edge 1 -> 0 */
    GPIO_EDGE_BOTH      /* Both edges X -> !X */
} gpio_edge_t;

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

#ifdef __cplusplus
}
#endif

#endif


/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <errno.h>

#include "gpio.h"

#define P_PATH_MAX  256

static const char *gpio_direction_to_string[] = {
    [GPIO_DIR_IN]         = "in",
    [GPIO_DIR_OUT]        = "out",
    [GPIO_DIR_OUT_LOW]    = "low",
    [GPIO_DIR_OUT_HIGH]   = "high",
};

static const char *gpio_edge_to_string[] = {
    [GPIO_EDGE_NONE]      = "none",
    [GPIO_EDGE_RISING]    = "rising",
    [GPIO_EDGE_FALLING]   = "falling",
    [GPIO_EDGE_BOTH]      = "both",
};

static int _gpio_error(struct gpio_handle *gpio, int code, int c_errno, const char *fmt, ...) {
    va_list ap;

    gpio->error.c_errno = c_errno;

    va_start(ap, fmt);
    vsnprintf(gpio->error.errmsg, sizeof(gpio->error.errmsg), fmt, ap);
    va_end(ap);

    /* Tack on strerror() and errno */
    if (c_errno) {
        char buf[64];
        strerror_r(c_errno, buf, sizeof(buf));
        snprintf(gpio->error.errmsg+strlen(gpio->error.errmsg), sizeof(gpio->error.errmsg)-strlen(gpio->error.errmsg), ": %s [errno %d]", buf, c_errno);
    }

    return code;
}

int gpio_open(gpio_t *gpio, unsigned int pin, gpio_direction_t direction) {
    char gpio_path[P_PATH_MAX];
    struct stat stat_buf;
    char buf[16];
    int fd;

    if (direction != GPIO_DIR_IN && direction != GPIO_DIR_OUT && direction != GPIO_DIR_OUT_LOW && direction != GPIO_DIR_OUT_HIGH && direction != GPIO_DIR_PRESERVE)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO direction (can be in, out, low, high, preserve)");

    /* Check if GPIO directory exists */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%d", pin);
    if (stat(gpio_path, &stat_buf) < 0) {
        /* Write pin number to export file */
        snprintf(buf, sizeof(buf), "%d", pin);
        if ((fd = open("/sys/class/gpio/export", O_WRONLY)) < 0)
            return _gpio_error(gpio, GPIO_ERROR_EXPORT, errno, "Exporting GPIO: opening 'export'");
        if (write(fd, buf, strlen(buf)+1) < 0) {
            int errsv = errno;
            close(fd);
            return _gpio_error(gpio, GPIO_ERROR_EXPORT, errsv, "Exporting GPIO: writing 'export'");
        }
        if (close(fd) < 0)
            return _gpio_error(gpio, GPIO_ERROR_EXPORT, errno, "Exporting GPIO: closing 'export'");

        /* Check if GPIO direction exists now */
        if (stat(gpio_path, &stat_buf) < 0)
            return _gpio_error(gpio, GPIO_ERROR_EXPORT, errno, "Exporting GPIO: stat 'gpio%d/'", pin);
    }

    /* If not preserving existing direction */
    if (direction != GPIO_DIR_PRESERVE) {
        /* Write direction */
        snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%d/direction", pin);
        if ((fd = open(gpio_path, O_WRONLY)) < 0)
            return _gpio_error(gpio, GPIO_ERROR_SET_DIRECTION, errno, "Configuring GPIO: opening 'direction'");
        if (write(fd, gpio_direction_to_string[direction], strlen(gpio_direction_to_string[direction])+1) < 0) {
            int errsv = errno;
            close(fd);
            return _gpio_error(gpio, GPIO_ERROR_SET_DIRECTION, errsv, "Configuring GPIO: writing 'direction'");
        }
        if (close(fd) < 0)
            return _gpio_error(gpio, GPIO_ERROR_SET_DIRECTION, errno, "Configuring GPIO: closing 'direction'");
    }

    memset(gpio, 0, sizeof(struct gpio_handle));
    gpio->pin = pin;

    /* Open value */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%d/value", pin);
    if ((gpio->fd = open(gpio_path, O_RDWR)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_OPEN, errno, "Opening GPIO 'gpio%d/value'", pin);

    return 0;
}

int gpio_close(gpio_t *gpio) {
    if (gpio->fd < 0)
        return 0;

    /* Close fd */
    if (close(gpio->fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_CLOSE, errno, "Closing GPIO 'value'");

    gpio->fd = -1;

    return 0;
}

int gpio_read(gpio_t *gpio, bool *value) {
    char buf[2];

    /* Read fd */
    if (read(gpio->fd, buf, 2) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Reading GPIO 'value'");

    /* Rewind */
    if (lseek(gpio->fd, 0, SEEK_SET) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Rewinding GPIO 'value'");

    if (buf[0] == '0')
        *value = false;
    else if (buf[0] == '1')
        *value = true;
    else
        return _gpio_error(gpio, GPIO_ERROR_IO, 0, "Unknown GPIO value");

    return 0;
}

int gpio_write(gpio_t *gpio, bool value) {
    char value_str[][2] = {"0", "1"};

    /* Write fd */
    if (write(gpio->fd, value_str[value], 2) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Writing GPIO 'value'");

    /* Rewind */
    if (lseek(gpio->fd, 0, SEEK_SET) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Rewinding GPIO 'value'");

    return 0;
}

int gpio_poll(gpio_t *gpio, int timeout_ms) {
    struct pollfd fds[1];
    char buf[1];
    int ret;

    /* Dummy read before poll */
    if (read(gpio->fd, buf, 1) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Reading GPIO 'value'");

    /* Seek to end */
    if (lseek(gpio->fd, 0, SEEK_END) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Seeking to end of GPIO 'value'");

    /* Poll */
    fds[0].fd = gpio->fd;
    fds[0].events = POLLPRI | POLLERR;
    if ((ret = poll(fds, 1, timeout_ms)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Polling GPIO 'value'");

    /* GPIO edge interrupt occurred */
    if (ret) {
        /* Rewind */
        if (lseek(gpio->fd, 0, SEEK_SET) < 0)
            return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Rewinding GPIO 'value'");

        return 1;
    }

    /* Timed out */
    return 0;
}

int gpio_set_direction(gpio_t *gpio, gpio_direction_t direction) {
    char gpio_path[P_PATH_MAX];
    int fd;

    if (direction != GPIO_DIR_IN && direction != GPIO_DIR_OUT && direction != GPIO_DIR_OUT_LOW && direction != GPIO_DIR_OUT_HIGH)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO direction (can be in, out, low, high)");

    /* Write direction */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%d/direction", gpio->pin);
    if ((fd = open(gpio_path, O_WRONLY)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_SET_DIRECTION, errno, "Opening GPIO 'direction'");
    if (write(fd, gpio_direction_to_string[direction], strlen(gpio_direction_to_string[direction])+1) < 0) {
        int errsv = errno;
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_SET_DIRECTION, errsv, "Writing GPIO 'direction'");
    }
    if (close(fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_SET_DIRECTION, errno, "Closing GPIO 'direction'");

    return 0;
}

int gpio_get_direction(gpio_t *gpio, gpio_direction_t *direction) {
    char gpio_path[P_PATH_MAX];
    char buf[8];
    int fd, ret;

    /* Read direction */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%d/direction", gpio->pin);
    if ((fd = open(gpio_path, O_RDONLY)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_GET_DIRECTION, errno, "Opening GPIO 'direction'");
    if ((ret = read(fd, buf, sizeof(buf))) < 0) {
        int errsv = errno;
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_GET_DIRECTION, errsv, "Writing GPIO 'direction'");
    }
    if (close(fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_GET_DIRECTION, errno, "Closing GPIO 'direction'");

    buf[ret] = '\0';

    if (strcmp(buf, "in\n") == 0)
        *direction = GPIO_DIR_IN;
    else if (strcmp(buf, "out\n") == 0)
        *direction = GPIO_DIR_OUT;
    else
        return _gpio_error(gpio, GPIO_ERROR_GET_DIRECTION, 0, "Unknown GPIO direction");

    return 0;
}

int gpio_supports_interrupts(gpio_t *gpio, bool *supported) {
    char gpio_path[P_PATH_MAX];
    struct stat stat_buf;

    /* Check for edge */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%d/edge", gpio->pin);

    if (stat(gpio_path, &stat_buf) < 0) {
        if (errno == ENOENT) {
            *supported = false;
            return 0;
        }

        /* Other error */
        return _gpio_error(gpio, GPIO_ERROR_IO, errno, "Exporting GPIO: stat 'gpio%d/edge'", gpio->pin);
    }

    *supported = true;
    return 0;
}

int gpio_set_edge(gpio_t *gpio, gpio_edge_t edge) {
    char gpio_path[P_PATH_MAX];
    int fd;

    if (edge != GPIO_EDGE_NONE && edge != GPIO_EDGE_RISING && edge != GPIO_EDGE_FALLING && edge != GPIO_EDGE_BOTH)
        return _gpio_error(gpio, GPIO_ERROR_ARG, 0, "Invalid GPIO interrupt edge (can be none, rising, falling, both)");

    /* Write edge */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%d/edge", gpio->pin);
    if ((fd = open(gpio_path, O_WRONLY)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_SET_EDGE, errno, "Opening GPIO 'edge'");
    if (write(fd, gpio_edge_to_string[edge], strlen(gpio_edge_to_string[edge])+1) < 0) {
        int errsv = errno;
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_SET_EDGE, errsv, "Writing GPIO 'edge'");
    }
    if (close(fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_SET_EDGE, errno, "Closing GPIO 'edge'");

    return 0;
}

int gpio_get_edge(gpio_t *gpio, gpio_edge_t *edge) {
    char gpio_path[P_PATH_MAX];
    char buf[16];
    int fd, ret;

    /* Read edge */
    snprintf(gpio_path, sizeof(gpio_path), "/sys/class/gpio/gpio%d/edge", gpio->pin);
    if ((fd = open(gpio_path, O_RDONLY)) < 0)
        return _gpio_error(gpio, GPIO_ERROR_GET_EDGE, errno, "Opening GPIO 'edge'");
    if ((ret = read(fd, buf, sizeof(buf))) < 0) {
        int errsv = errno;
        close(fd);
        return _gpio_error(gpio, GPIO_ERROR_GET_EDGE, errsv, "Writing GPIO 'edge'");
    }
    if (close(fd) < 0)
        return _gpio_error(gpio, GPIO_ERROR_GET_EDGE, errno, "Closing GPIO 'edge'");

    buf[ret] = '\0';

    if (strcmp(buf, "none\n") == 0)
        *edge = GPIO_EDGE_NONE;
    else if (strcmp(buf, "rising\n") == 0)
        *edge = GPIO_EDGE_RISING;
    else if (strcmp(buf, "falling\n") == 0)
        *edge = GPIO_EDGE_FALLING;
    else if (strcmp(buf, "both\n") == 0)
        *edge = GPIO_EDGE_BOTH;
    else
        return _gpio_error(gpio, GPIO_ERROR_GET_EDGE, 0, "Unknown GPIO edge");

    return 0;
}

int gpio_tostring(gpio_t *gpio, char *str, size_t len) {
    gpio_direction_t direction;
    const char *direction_str;
    gpio_edge_t edge;
    const char *edge_str;

    if (gpio_get_direction(gpio, &direction) < 0)
        direction_str = "?";
    else
        direction_str = gpio_direction_to_string[direction];

    if (gpio_get_edge(gpio, &edge) < 0)
        edge_str = "?";
    else
        edge_str = gpio_edge_to_string[edge];

    return snprintf(str, len, "GPIO %d (fd=%d, direction=%s, edge=%s)", gpio->pin, gpio->fd, direction_str, edge_str);
}

const char *gpio_errmsg(gpio_t *gpio) {
    return gpio->error.errmsg;
}

int gpio_errno(gpio_t *gpio) {
    return gpio->error.c_errno;
}

unsigned int gpio_pin(gpio_t *gpio) {
    return gpio->pin;
}

int gpio_fd(gpio_t *gpio) {
    return gpio->fd;
}

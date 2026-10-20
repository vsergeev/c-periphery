#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "backlight.h"

/*=====================
 * private
 *====================*/
static int _backlight_error(backlight_t *bl, int code, int c_errno, const char *fmt, ...) {
    va_list ap;

    bl->error.c_errno = c_errno;

    va_start(ap, fmt);
    vsnprintf(bl->error.errmsg, sizeof(bl->error.errmsg), fmt, ap);
    va_end(ap);

    /* Tack on strerror() and errno */
    if (c_errno) {
        char buf[64];
        strerror_r(c_errno, buf, sizeof(buf));
        snprintf(bl->error.errmsg+strlen(bl->error.errmsg), sizeof(bl->error.errmsg)-strlen(bl->error.errmsg), ": %s [errno %d]", buf, c_errno);
    }

    return code;
}

/*=====================
 * Getter
 *====================*/

int backlight_get_brightness(backlight_t *bl, unsigned int *brightness) {
    char backlight_path[P_PATH_MAX];
    char buf[16];
    int fd, ret;

    snprintf(backlight_path, sizeof(backlight_path), DEV_CLASS_PREFIX"/%s/brightness", bl->name);

    if ((fd = open(backlight_path, O_RDONLY)) < 0)
        return _backlight_error(bl, BL_ERROR_IO, errno, "Opening BACKLIGHT 'brightness'");

    if ((ret = read(fd, buf, sizeof(buf))) < 0) {
        int errsv = errno;
        close(fd);
        return _backlight_error(bl, BL_ERROR_IO, errsv, "Reading BACKLIGHT 'brightness'");
    }

    if (close(fd) < 0)
        return _backlight_error(bl, BL_ERROR_IO, errno, "Closing BACKLIGHT 'brightness'");

    /* Null-terminate over newline */
    buf[ret] = '\0';

    *brightness = strtoul(buf, NULL, 10);

    return 0;
}

int backlight_get_max_brightness(backlight_t *bl, unsigned int *max_brightness) {
    char backlight_path[P_PATH_MAX];
    char buf[16];
    int fd, ret;

    snprintf(backlight_path, sizeof(backlight_path), DEV_CLASS_PREFIX"/%s/max_brightness", bl->name);

    if ((fd = open(backlight_path, O_RDONLY)) < 0)
        return _backlight_error(bl, BL_ERROR_QUERY, errno, "Opening BACKLIGHT 'max_brightness'");

    if ((ret = read(fd, buf, sizeof(buf))) < 0) {
        int errsv = errno;
        close(fd);
        return _backlight_error(bl, BL_ERROR_QUERY, errsv, "Reading BACKLIGHT 'max_brightness'");
    }

    if (close(fd) < 0)
        return _backlight_error(bl, BL_ERROR_QUERY, errno, "Closing BACKLIGHT 'max_brightness'");

    /* Null-terminate over newline */
    buf[ret] = '\0';

    bl->max_brightness = strtoul(buf, NULL, 10);

    *max_brightness = bl->max_brightness;

    return 0;
}


/*=====================
 * Setter
 *====================*/
int backlight_set_brightness(backlight_t *bl, unsigned int brightness) {
    char backlight_path[P_PATH_MAX];
    char buf[16];
    int fd, len;

    if (brightness > bl->max_brightness)
        return _backlight_error(bl, BL_ERROR_ARG, 0, "Brightness out of bounds (max is %u)", bl->max_brightness);

    snprintf(backlight_path, sizeof(backlight_path), DEV_CLASS_PREFIX"/%s/brightness", bl->name);

    if ((fd = open(backlight_path, O_WRONLY)) < 0)
        return _backlight_error(bl, BL_ERROR_IO, errno, "Opening BACKLIGHT 'brightness'");

    len = snprintf(buf, sizeof(buf), "%u\n", brightness);

    if (write(fd, buf, len) < 0) {
        int errsv = errno;
        close(fd);
        return _backlight_error(bl, BL_ERROR_IO, errsv, "Writing BACKLIGHT 'brightness'");
    }

    if (close(fd) < 0)
        return _backlight_error(bl, BL_ERROR_IO, errno, "Closing BACKLIGHT 'brightness'");

    return 0;
}

/*=====================
 * Primary Functions
 *====================*/
backlight_t *backlight_new(void) {
    backlight_t *bl = calloc(1, sizeof(backlight_t));
    if (bl == NULL)
        return NULL;

    return bl;
}

int backlight_open(backlight_t *bl, const char *name) {
    char backlight_path[P_PATH_MAX];
    int fd, ret;

    snprintf(backlight_path, sizeof(backlight_path), DEV_CLASS_PREFIX"/%s/brightness", name);

    if ((fd = open(backlight_path, O_RDWR)) < 0)
        return _backlight_error(bl, BL_ERROR_OPEN, errno, "Opening BACKLIGHT: opening 'brightness'");

    close(fd);

    strncpy(bl->name, name, sizeof(bl->name) - 1);
    bl->name[sizeof(bl->name) - 1] = '\0';

    if ((ret = backlight_get_max_brightness(bl, &bl->max_brightness)) < 0)
        return ret;

    return 0;
}

int backlight_close(backlight_t *bl) {
    (void)bl;
    return 0;
}

int backlight_read(backlight_t *bl, bool *value) {
    int ret;
    unsigned int brightness;

    if ((ret = backlight_get_brightness(bl, &brightness)) < 0)
        return ret;

    *value = brightness != 0;

    return 0;
}

int backlight_write(backlight_t *bl, bool value) {
    return backlight_set_brightness(bl, value ? bl->max_brightness : 0);
}

void backlight_free(backlight_t *bl) {
    free(bl);
}

/*=====================
 * Miscellaneous
 *====================*/
int backlight_name(backlight_t *bl, char *str, size_t len) {
    if (!len)
        return 0;

    strncpy(str, bl->name, len - 1);
    str[len - 1] = '\0';

    return 0;
}

int backlight_tostring(backlight_t *bl, char *str, size_t len) {
    unsigned int brightness;
    char brightness_str[16];
    unsigned int max_brightness;
    char max_brightness_str[16];

    if (backlight_get_brightness(bl, &brightness) < 0)
        strcpy(brightness_str, "<error>");
    else
        snprintf(brightness_str, sizeof(brightness_str), "%u", brightness);

    if (backlight_get_max_brightness(bl, &max_brightness) < 0)
        strcpy(max_brightness_str, "<error>");
    else
        snprintf(max_brightness_str, sizeof(max_brightness_str), "%u", max_brightness);

    return snprintf(str, len, "Backlight %s (brightness=%s, max_brightness=%s)", bl->name, brightness_str, max_brightness_str);
}

/*=====================
 * Error Handling
 *====================*/
int backlight_errno(backlight_t *bl) {
    return bl->error.c_errno;
}

const char *backlight_errmsg(backlight_t *bl) {
    return bl->error.errmsg;
}

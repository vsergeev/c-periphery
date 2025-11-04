/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */
#ifndef _PERIPHERY_BACKLIGHT_H
#define _PERIPHERY_BACKLIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>


#define P_PATH_MAX  256
#define DEV_CLASS_PREFIX  "/sys/class/backlight"

enum backlight_error_code {
    BL_ERROR_ARG       = -1, /* Invalid arguments */
    BL_ERROR_OPEN      = -2, /* Opening backlight */
    BL_ERROR_QUERY     = -3, /* Querying backlight attributes */
    BL_ERROR_IO        = -4, /* Reading/writing backlight brightness */
    BL_ERROR_CLOSE     = -5, /* Closing backlight */
};

struct backlight_handle {
    char name[64];
    unsigned int max_brightness;

    struct {
        int c_errno;
        char errmsg[96];
    } error;
};

typedef struct backlight_handle backlight_t;

/* Primary Functions */
backlight_t *backlight_new(void);
int backlight_open(backlight_t *bl, const char *name);
int backlight_close(backlight_t *bl);
int backlight_read(backlight_t *bl, bool *value);
int backlight_write(backlight_t *bl, bool value);
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

#ifdef __cplusplus
}/*extern "C"*/
#endif

#endif //_PERIPHERY_BACKLIGHT_H


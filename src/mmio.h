/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#ifndef _PERIPHERY_MMIO_H
#define _PERIPHERY_MMIO_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

enum mmio_error_code {
    MMIO_ERROR_ARG          = -1, /* Invalid arguments */
    MMIO_ERROR_OPEN         = -2, /* Opening /dev/mem */
    MMIO_ERROR_MAP          = -3, /* Mapping memory */
    MMIO_ERROR_CLOSE        = -4, /* Closing /dev/mem */
    MMIO_ERROR_UNMAP        = -5, /* Unmapping memory */
};

typedef struct mmio_handle {
    uintptr_t base, aligned_base;
    size_t size, aligned_size;
    void *ptr;

    struct {
        int c_errno;
        char errmsg[96];
    } error;
} mmio_t;

/* Primary Functions */
int mmio_open(mmio_t *mmio, uintptr_t base, size_t size);
void *mmio_ptr(mmio_t *mmio);
int mmio_read32(mmio_t *mmio, uintptr_t offset, uint32_t *value);
int mmio_read16(mmio_t *mmio, uintptr_t offset, uint16_t *value);
int mmio_read8(mmio_t *mmio, uintptr_t offset, uint8_t *value);
int mmio_read(mmio_t *mmio, uintptr_t offset, uint8_t *buf, size_t len);
int mmio_write32(mmio_t *mmio, uintptr_t offset, uint32_t value);
int mmio_write16(mmio_t *mmio, uintptr_t offset, uint16_t value);
int mmio_write8(mmio_t *mmio, uintptr_t offset, uint8_t value);
int mmio_write(mmio_t *mmio, uintptr_t offset, const uint8_t *buf, size_t len);
int mmio_close(mmio_t *mmio);

/* Miscellaneous */
uintptr_t mmio_base(mmio_t *mmio);
size_t mmio_size(mmio_t *mmio);
int mmio_tostring(mmio_t *mmio, char *str, size_t len);

/* Error Handling */
int mmio_errno(mmio_t *mmio);
const char *mmio_errmsg(mmio_t *mmio);

#endif


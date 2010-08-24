#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t len;
    uint16_t id;
    uint16_t ver;
    uint16_t sum;
} table_t;

void storage_clear(void);
uint16_t storage_find(uint16_t, table_t *);
bool storage_get(uint16_t, table_t *, uint16_t);

#endif

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

uint16_t storage_sum(table_t const *);
void storage_clear(void);
uint16_t storage_find(uint16_t, table_t *);
bool storage_get(table_t *);
bool storage_set(table_t *);

#endif

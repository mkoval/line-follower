#include <avr/eeprom.h>
#include "libarduino2.h"
#include "storage.h"

#define EEPROM_BEGIN 0
#define EEPROM_END   1023
#define EEPROM_EOF   (EEPROM_END + 1)
#define EEPROM_ERROR (EEPROM_END + 2)

void storage_clear(void) {
    table_t end = { 0 };
    eeprom_write_block(&end, (void *)EEPROM_BEGIN, sizeof(table_t));
}

uint16_t storage_find(uint16_t id, table_t *table) {
    uint16_t i = EEPROM_BEGIN;
    
    while (i + sizeof(table_t) < EEPROM_END) {
        eeprom_read_block(table, (void *)i, sizeof(table_t));

        /* Our EEPROM usage is terminated by a table of length zero. */
        if (table->len == 0) {
            return EEPROM_EOF;
        }
        /* Table is invalid; it extends past the end of available memory. */
        else if (i + sizeof(table_t) + table->len >= EEPROM_END) {
            return EEPROM_ERROR;
        }
        /* Found a match (although the checksum may fail...). */
        else if (id == table->id) {
            return i;
        }
        else {
            i += sizeof(table_t) + table->len;
        }
    }
    return EEPROM_EOF;
}

bool storage_get(uint16_t id, table_t *table, uint16_t capacity) {
    uint8_t *data = (uint8_t *)table + sizeof(table_t);
    uint16_t i = storage_find(id, table);
    uint16_t sum = 0;
    uint16_t j;

    /* There is no table with the desired ID. */
    if (i == EEPROM_EOF) {
        return false;
    }
    /* Table extends past the end of available EEPROM. */
    else if (i == EEPROM_ERROR) {
        ERROR("storage_get", "table intersects end of available memory");
        return false;
    }

    /* Copy the table's data from EEPROM into RAM. */
    eeprom_read_block(data, (void *)(i + sizeof(table_t)), table->len);

    /* Verify the table's checksum. */
    for (j = 0; j < table->len; ++j) {
        sum += data[j];
    }

    if (sum != table->sum) {
        ERROR("storage_get", "table failed checksum");
        return false;
    } else {
        return true;
    }
}

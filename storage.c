#include <avr/eeprom.h>
#include "libarduino2.h"
#include "storage.h"

#define EEPROM_BEGIN 0
#define EEPROM_END   1024
#define EEPROM_EOF   (EEPROM_END + 1)
#define EEPROM_ERROR (EEPROM_END + 2)

uint16_t storage_sum(table_t const *table) {
    uint8_t const *data = (uint8_t const *)(table + sizeof(table_t));
    uint16_t sum = 0;
    int i;

    for (i = 0; i < table->len; ++i) {
        sum += data[i];
    }
    return sum;
}

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

uint16_t storage_end(void) {
    table_t table;
    uint16_t i = EEPROM_BEGIN;
    
    while (i + sizeof(table_t) < EEPROM_END) {
        eeprom_read_block(&table, (void *)i, sizeof(table_t));

        /* Found the end of valid tables in the file. */
        if (table.len == 0 || i + sizeof(table_t) + table.len >= EEPROM_END) {
            break;
        } else {
            i += sizeof(table_t) + table.len;
        }
    }
    return i;
}


bool storage_get(table_t *table) {
    uint16_t i;
    table_t buf;
    
    i = storage_find(table->id, &buf);

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
    eeprom_read_block(table, (void *)i, sizeof(table_t) + table->len);

    /* Verify the table's checksum. */
    if (storage_sum(table) != table->sum) {
        ERROR("storage_get", "table failed checksum");
        return true; /* TODO: Fix this error instead of squelching it. */
    } else {
        return true;
    }
}

bool storage_set(table_t *table) {
    uint16_t end = storage_end();
    uint16_t i;
    table_t buf;
    
    i = storage_find(table->id, &buf);
    table->sum = storage_sum(table);

    /* Table extends past the end of available EEPROM. */
    if (i == EEPROM_ERROR) {
        ERROR("storage_set", "table intersects end of available memory");
        return false;
    }
    /* There is not enough room to append the table to the end of the EEPROM. */
    else if (i == EEPROM_EOF && end + sizeof(table_t) + table->len >= EEPROM_END) {
        ERROR("storage_set", "insufficient memory");
        return false;
    }
    /* Append the table to the end of the EEPROM. */
    else if (i == EEPROM_EOF) {
        eeprom_write_block(table, (void *)end, table->len + sizeof(table_t));
        return true;
    }
    /* Table in EEPROM has a different structure than the one we're writing. */
    else if (table->len != buf.len || table->ver != buf.ver) {
        ERROR("storage_set", "incorrect table size or version");
        return false;
    }
    /* Table header is the same, only the data is different. */
    else {
        table->sum = storage_sum(table);
        eeprom_write_block(table, (void *)i, table->len + sizeof(table_t));
        return true;
    }
}

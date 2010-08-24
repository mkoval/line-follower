#ifndef SENSOR_H_
#define SENSOR_H_

#include "storage.h"

#define SENSOR_VERSION 1

enum {
    SENSOR_FARRIGHT = 0,
    SENSOR_MIDRIGHT,
    SENSOR_MIDLEFT,
    SENSOR_FARLEFT,
    SENSOR_NUM
};

typedef struct {
    table_t table;
    uint16_t floor[SENSOR_NUM];
    uint16_t line[SENSOR_NUM];
} sensor_config_t;
typedef float sensor_t[4];

void sensor_init(sensor_config_t *);
void sensor_line(sensor_config_t *);
void sensor_floor(sensor_config_t *);
void sensor_update(sensor_config_t const *, sensor_t);

#endif

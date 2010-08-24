#include "libarduino2.h"
#include "config.h"
#include "sensor.h"

#define CONSTRAIN(_x_, _min_, _max_) (  \
    ((_x_) < (_min_)) ? (_min_) :       \
    ((_x_) > (_max_)) ? (_max_) : (_x_) \
)

void sensor_init_config(sensor_config_t *config) {
}

void sensor_init(sensor_config_t *config) {
    uint8_t i;
    bool read;
    analog_init();

    /* Read stored configuration values from EEPROM. */
    config->table.len = sizeof(sensor_config_t);
    config->table.id  = TABLE_SENSOR;
    config->table.ver = SENSOR_VERSION;
    read = storage_get(&config->table);

    /* Fall back on default values if the read fails. */
    if (!read) {
        ERROR("sensor_init", "no configuration data in EEPROM");

        for (i = 0; i < SENSOR_NUM; ++i) {
            config->floor[i] = 0;
            config->line[i]  = 1024;
        }
    }
}

void sensor_floor(sensor_config_t *config) {
    uint8_t i;
    for (i = 0; i < SENSOR_NUM; ++i) {
        config->floor[i] = analog_get(i);
    }
}

void sensor_line(sensor_config_t *config) {
    uint8_t i;
    for(i = 0; i < SENSOR_NUM; ++i) {
        config->line[i] = analog_get(i);
    }
}

void sensor_update(sensor_config_t const *config, sensor_t *sen) {
    uint8_t i;
    for (i = 0; i < SENSOR_NUM; ++i) {
        float value = analog_get(i);

        /* Force the line to produce higher sensor readings than the floor. */
        if (config->line[i] > config->floor[i]) {
            sen->value[i] = (value - config->floor[i]) / config->line[i];
        } else {
            sen->value[i] = 1.0f - (value - config->line[i]) / config->floor[i];
        }
        sen->value[i] = CONSTRAIN(sen->value[i], 0.0f, 1.0f);
    }
}

#include "libarduino2.h"
#include "sensor.h"

#define CONSTRAIN(_x_, _min_, _max_) (  \
    ((_x_) < (_min_)) ? (_min_) :       \
    ((_x_) > (_max_)) ? (_max_) : (_x_) \
)

static uint16_t g_floor[SENSOR_NUM];
static uint16_t g_line[SENSOR_NUM];

void sensor_init(void) {
    uint8_t i;
    analog_init();

    for (i = 0; i < SENSOR_NUM; ++i) {
        g_floor[i] = 0;
        g_line[i]  = 1024;
    }
}

void sensor_floor(void) {
    uint8_t i;
    for (i = 0; i < SENSOR_NUM; ++i) {
        g_floor[i] = analog_get(i);
    }
}

void sensor_line(void) {
    uint8_t i;
    for(i = 0; i < SENSOR_NUM; ++i) {
        g_line[i] = analog_get(i);
    }
}

void sensor_update(sensor_t sen) {
    uint8_t i;
    for (i = 0; i < SENSOR_NUM; ++i) {
        float value = analog_get(i);

        /* Force the line to produce higher sensor readings than the floor. */
        if (g_line[i] > g_floor[i]) {
            sen[i] = (value - g_floor[i]) / g_line[i];
        } else {
            sen[i] = 1.0f - (value - g_line[i]) / g_floor[i];
        }
        sen[i] = CONSTRAIN(sen[i], 0.0f, 1.0f);
    }
}

#ifndef SENSOR_H_
#define SENSOR_H_

enum {
    SENSOR_FARRIGHT = 0,
    SENSOR_MIDRIGHT,
    SENSOR_MIDLEFT,
    SENSOR_FARLEFT,
    SENSOR_NUM
};
typedef float sensor_t[4];

void sensor_init(void);
void sensor_line(void);
void sensor_floor(void);
void sensor_update(sensor_t);

#endif

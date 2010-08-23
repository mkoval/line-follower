#ifndef MOTOR_H_
#define MOTOR_H_

enum {
    MOTOR_RIGHT = 0,
    MOTOR_LEFT,
    MOTOR_NUM
};
typedef float motor_t[MOTOR_NUM];

void motor_init(void);
void motor_update(motor_t const);

#endif

#ifndef MOTOR_H_
#define MOTOR_H_

typedef struct {
    float right;
    float left;
} motor_t;

void motor_init(void);
void motor_update(motor_t const *);

#endif

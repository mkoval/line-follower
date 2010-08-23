#include "libarduino2.h"
#include "motor.h"

#define CONSTRAIN(_x_, _min_, _max_) (  \
    ((_x_) < (_min_)) ? (_min_) :       \
    ((_x_) > (_max_)) ? (_max_) : (_x_) \
)

#define MOTOR_LEFT_DIR   7
#define MOTOR_RIGHT_DIR  8
#define MOTOR_LEFT_PWM   (9 - 8)
#define MOTOR_RIGHT_PWM  (10 - 8)
#define MOTOR_LEFT_EN    11
#define MOTOR_RIGHT_EN   12
#define MOTOR_LEFT_SIGN  -1
#define MOTOR_RIGHT_SIGN +1

void motor_init(void) {
    digital_init(MOTOR_LEFT_DIR, PIN_OUTPUT);
    digital_init(MOTOR_RIGHT_DIR, PIN_OUTPUT);
    digital_init(MOTOR_LEFT_EN, PIN_OUTPUT);
    digital_init(MOTOR_RIGHT_EN, PIN_OUTPUT);
    pwm_init(MOTOR_LEFT_PWM);
    pwm_init(MOTOR_RIGHT_PWM);

    digital_set(MOTOR_LEFT_DIR, 0);
    digital_set(MOTOR_RIGHT_DIR, 0);
    digital_set(MOTOR_LEFT_EN, 1);
    digital_set(MOTOR_RIGHT_EN, 1);
}

void motor_update(motor_t const motor) {
    float l = MOTOR_LEFT_SIGN * CONSTRAIN(motor[MOTOR_LEFT], -1.0f, +1.0f);
    float r = MOTOR_RIGHT_SIGN * CONSTRAIN(motor[MOTOR_RIGHT], -1.0f, +1.0f);
    bool l_sign = l < 0.0f;
    bool r_sign = r < 0.0f;

    digital_set(MOTOR_LEFT_DIR, l_sign);
    digital_set(MOTOR_RIGHT_DIR, r_sign);
    pwm_set(MOTOR_LEFT_PWM,  (uint8_t)(l * 255));
    pwm_set(MOTOR_RIGHT_PWM, (uint8_t)(r * 255));
}

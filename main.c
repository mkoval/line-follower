#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "libarduino2.h"
#include "motor.h"
#include "sensor.h"

typedef enum {
    MODE_MENU,
    MODE_FOLLOW,
    MODE_TRAIN
} mode_t;

int main(void) {
    mode_t   mode = MODE_MENU;
    sensor_t sen;
    motor_t  mot;
    char cmd[16];

    motor_init();
    sensor_init();
    serial_init();

    for (;;) {
        mot[MOTOR_LEFT]  = 0.0f;
        mot[MOTOR_RIGHT] = 0.0f;
        sensor_update(sen);

        switch (mode) {
        case MODE_MENU:
            /* Prompt the user for a command. */
            printf_P(PSTR(" >>> "));
            readline(cmd, cmd + sizeof(cmd));

            /* Calibrate the line following sensors. */
            if (!strcmp(cmd, "line")) {
                sensor_line();
                printf_P(PSTR("Line calibration complete.\n"));
            } else if (!strcmp(cmd, "floor")) {
                sensor_floor();
                printf_P(PSTR("Floor calibration complete.\n"));
            }
            /* Follow the line with a machine learning algorithm. */
            else if (!strcmp(cmd, "follow")) {
                mode = MODE_FOLLOW;
            } else if (!strcmp(cmd, "train")) {
                mode = MODE_TRAIN;
            }
            /* Unrecognized command. */
            else {
                printf_P(PSTR("Unrecognized command.\n"));
            }
            break;

        case MODE_FOLLOW:
            break;

        case MODE_TRAIN:
            break;
        }
    }

    motor_update(mot);
}

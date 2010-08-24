#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "libarduino2.h"
#include "motor.h"
#include "sensor.h"
#include "storage.h"

typedef enum {
    MODE_MENU,
    MODE_FOLLOW,
    MODE_TRAIN
} mode_t;

typedef struct {
    table_t store;
    char value;
} storage_test_t;

static sensor_config_t g_config_sen;

int main(void) {
    mode_t   mode = MODE_MENU;
    sensor_t sen;
    motor_t  mot;
    char cmd[16];

    motor_init();
    serial_init();
    timer_init(200);
    
    sensor_init(&g_config_sen);

    for (;;) {
        mot[MOTOR_LEFT]  = 0.0f;
        mot[MOTOR_RIGHT] = 0.0f;
        sensor_update(&g_config_sen, &sen);

        switch (mode) {
        case MODE_MENU:
            /* Prompt the user for a command. */
            printf_P(PSTR(" >>> "));
            readline(cmd, cmd + sizeof(cmd));

            /* Save current the current configuration to EEPROM. */
            if (!strcmp(cmd, "save")) {
                storage_set(&g_config_sen.table);
                printf_P(PSTR("Configuration saved to EEPROM.\r\n"));
            }
            /* Clear all data stored in EEPROM. */
            else if (!strcmp(cmd, "reset")) {
                for (;;) {
                    printf_P(PSTR("Are you sure? <Y/N> "));
                    readline(cmd, cmd + 1);

                    if (*cmd == 'y' || *cmd == 'Y') {
                        storage_clear();
                        printf_P(PSTR("EEPROM cleared.\r\n"));
                        break;
                    } else if (*cmd == 'n' || *cmd == 'N') {
                        break;
                    }
                }
            }
            /* Calibrate the line following sensors. */
            else if (!strcmp(cmd, "line")) {
                sensor_line(&g_config_sen);
                printf_P(PSTR("Line calibration complete.\r\n"));
            } else if (!strcmp(cmd, "floor")) {
                sensor_floor(&g_config_sen);
                printf_P(PSTR("Floor calibration complete.\r\n"));
            }
            /* Follow the line with a machine learning algorithm. */
            else if (!strcmp(cmd, "follow")) {
                mode = MODE_FOLLOW;
            } else if (!strcmp(cmd, "train")) {
                mode = MODE_TRAIN;
            }
            /* Unrecognized command. */
            else if (*cmd) {
                printf_P(PSTR("Unrecognized command.\r\n"));
            }
            break;

        case MODE_FOLLOW:
            printf("%4d %4d %4d %4d\r\n",
                (int)(sen.value[0] * 1000),
                (int)(sen.value[1] * 1000),
                (int)(sen.value[2] * 1000),
                (int)(sen.value[3] * 1000)
            );
            break;

        case MODE_TRAIN:
            break;
        }
    }

    motor_update(mot);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include "libarduino2.h"
#include "learn.h"
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

static learn_config_t  g_config_learn;
static sensor_config_t g_config_sen;

int main(void) {
    mode_t   mode = MODE_MENU;
    sensor_t sen;
    motor_t  mot;
    char cmd[16];
    char ch;

    motor_init();
    serial_init();
    timer_init(LEARN_TICK);
    learn_init(&g_config_learn);
    sensor_init(&g_config_sen);

    srand(analog_get(0));

    for (;;) {
        sensor_update(&g_config_sen, &sen);

        switch (mode) {
        case MODE_MENU:
            mot.left  = 0.0f;
            mot.right = 0.0f;
            motor_update(&mot);

            /* Prompt the user for a command. */
            printf_P(PSTR("\r\n>>> "));
            readline(cmd, cmd + sizeof(cmd));

            sensor_update(&g_config_sen, &sen);

            /* Save current the current configuration to EEPROM. */
            if (!strcmp(cmd, "save")) {
                uint16_t size = 0;
                bool saved = true;
                saved = saved && storage_set(&g_config_learn.table);
                saved = saved && storage_set(&g_config_sen.table);
                size += g_config_learn.table.len;
                size += g_config_sen.table.len;

                if (saved) {
                    printf_P(PSTR("Saved %d bytes to EEPROM.\r\n"), (int)size);
                } else {
                    printf_P(PSTR("Error saving %d bytes to EEPROM.\r\n"), (int)size);
                }
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
            /* Debugging printf()'s. */
            else if (!strcmp(cmd, "debug")) {
                uint8_t i;

                for (i = 0; i < SENSOR_NUM; ++i) {
                    printf_P(PSTR("%2d "), (int)(sen.value[i] * 100));
                }
                printf_P(PSTR("\r\n"));
            }
            /* Follow the line with a machine learning algorithm. */
            else if (!strcmp(cmd, "follow")) {
                mode = MODE_FOLLOW;
                printf_P(PSTR("Press 'x' to return to the shell\r\n"));
            } else if (!strcmp(cmd, "train")) {
                mode = MODE_TRAIN;
                learn_train_start(&g_config_learn);
                printf_P(PSTR("Press 'x' to return to the shell\r\n"));
            } else if (!strcmp(cmd, "brain")) {
                uint8_t i, j;

                for (i = 0; i < LEARN_STATES; ++i) {
                    printf("  ");
                    for (j = 0; j < LEARN_ACTIONS; ++j) {
                        printf_P(PSTR("%4d  "), (int)(g_config_learn.q[i][j]));
                    }
                    printf("\r\n");
                }
            }
            /* Unrecognized command. */
            else if (*cmd) {
                printf_P(PSTR("Unrecognized command.\r\n"));
            }
            break;

        case MODE_TRAIN:
            /* Throttle samples used for learning to reduce memory usage. */
            if (timer_done()) {
                learn_train(&g_config_learn, &sen, &mot);
            }
            motor_update(&mot);

            while ((ch = serial_getc()) != EOF) {
                if (ch == 'x' || ch == 'X') {
                    mode = MODE_MENU;
                    learn_train_end(&g_config_learn);
                    printf_P(PSTR("Training complete.\r\n"));
                }
            }
            break;

        case MODE_FOLLOW:
            /* Throttle sampling to match the training rate. */
            if (timer_done()) {
                learn_greed(&g_config_learn, &sen, &mot);
            }
            motor_update(&mot);

            while ((ch = serial_getc()) != EOF) {
                if (ch == 'x' || ch == 'X') {
                    mot.left  = 0.0f;
                    mot.right = 0.0f;
                    mode = MODE_MENU;
                    learn_train_end(&g_config_learn);
                }
            }
            break;
        }

    }
}

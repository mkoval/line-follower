#include <stdio.h>
#include <util/delay.h>
#include "libarduino2.h"

int main(void) {
    analog_init();
    serial_init();

    for (;;) {
        if (analog_available()) {
            printf("%4d %4d %4d %4d\n",
                analog_get(0),
                analog_get(1),
                analog_get(2),
                analog_get(3)
            );
        } else {
            printf("wait\n");
        }
    }
}

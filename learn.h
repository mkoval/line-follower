#ifndef LEARN_H_
#define LEARN_H_

#include "motor.h"
#include "sensor.h"
#include "storage.h"

#define LEARN_STATES  7
#define LEARN_ACTIONS 5
#define LEARN_HISTORY 100

#define LEARN_START   0.00f
#define LEARN_MID     0.75f;

#define LEARN_VERSION 1

typedef struct {
    uint8_t id;
} state_t;

typedef struct {
    uint8_t id;
} action_t;

typedef struct {
    state_t  state;
    action_t action;
} learn_pair_t;

typedef struct {
    table_t table;
    float q[LEARN_STATES][LEARN_ACTIONS];
    learn_pair_t history[LEARN_HISTORY];
    float r;
    uint16_t n;
} learn_config_t;

void learn_init(learn_config_t *);

void learn_train(learn_config_t *, sensor_t const *, motor_t *);
void learn_train_start(learn_config_t *);
void learn_train_end(learn_config_t *);

void learn_greed(learn_config_t *, sensor_t const *, motor_t *);

#endif

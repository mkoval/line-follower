#include <stdlib.h>
#include "config.h"
#include "libarduino2.h"
#include "learn.h"

#define ABS(_x_) (((_x_) < 0.0f) ? -(_x_) : (_x_))

static float g_reward[LEARN_STATES] = {
    10.0f, /* between middle sensors */
    1.0,   /* far-right sensor */
    5.0,   /* mid-right sensor */
    5.0,   /* mid-left sensor */
    1.0    /* far-left sensor */
};

void learn_state(sensor_t const *sen, state_t *state) {
    uint8_t i, max;

    /* Identify the sensor that is most likely over the line. */
    max = 0;
    for (i = 1; i < SENSOR_NUM; ++i) {
        if (sen->value[i] > sen->value[max]) {
            max = i;
        }
    }

    /* Assume the line is dead-center between the two middle sensors if it is
     * detected by none of them.
     */
    if (sen->value[max] < SENSOR_LINEMIN) {
        state->id = 0;
    /* Line is firmly detected by one of the sensors. */
    } else {
        state->id = max + 1;
    }
}

void learn_motor(action_t const *act, motor_t *motor) {
    int8_t middle = LEARN_ACTIONS / 2;
    int8_t delta  = act->id - middle;
    float  speed  = 1.0f - (float)ABS(delta) / middle;

    /* Turn Right */
    if (delta < 0.0f) {
        motor->left  = 1.0f;
        motor->right = speed;
    }
    /* Turn Left */
    else if (delta > 0.0f) {
        motor->left  = speed;
        motor->right = 1.0f;
    }
    /* Straight */
    else {
        motor->left  = 1.0f;
        motor->right = 1.0f;
    }
}

float learn_reward(state_t const *state) {
    if (0 <= state->id && state->id < LEARN_STATES) {
        return g_reward[state->id];
    } else {
        ERROR("learn_reward", "invalid state");
        return 0.0f;
    }
}

void learn_init(learn_config_t *config) {
    bool read;

    /* TODO: Avoid saving training data to EEPROM. */
    config->table.len = sizeof(learn_config_t);
    config->table.id  = TABLE_LEARN;
    config->table.ver = LEARN_VERSION;
    read = storage_get(&config->table);

    if (!read) {
        uint8_t i, j;
        ERROR("learn_init", "no configuration data in EEPROM");

        for (i = 0; i < LEARN_STATES; ++i)
        for (j = 0; j < LEARN_ACTIONS; ++j) {
            config->q[i][j] = LEARN_START;
        }
    }
}

void learn_train_start(learn_config_t *config) {
    config->r = 0.0f;
    config->n = 0;
}

void learn_train_end(learn_config_t *config) {
    uint16_t i;

    /* Update our estimate of Q using an incremental average. */
    for (i = 0; i < config->n; ++i) {
        state_t  *state  = &config->history[i].state;
        action_t *action = &config->history[i].action;
        float *q = &config->q[state->id][action->id];
        float  r = config->r;
        *q = *q + (r - *q) / (config->n + 1);
    }
}

void learn_train(learn_config_t *config, sensor_t const *sen, motor_t *mot) {
    action_t action;
    state_t  state;

    /* Randomly choose an action (off-policy learning). */
    learn_state(sen, &state);
    action.id = rand() % LEARN_ACTIONS;

    /* Log this decision for use by a Monte Carlo learning algorithm. */
    if (config->n < LEARN_HISTORY) {
        config->history[config->n].state  = state;
        config->history[config->n].action = action;
        config->r += learn_reward(&state);
        config->n += 1;
    } else {
        ERROR("learn_train", "insufficient space to store training history");
    }

    learn_motor(&action, mot);
}

void learn_greed(learn_config_t *config, sensor_t const *sen, motor_t *mot) {
    action_t action;
    state_t  state;
    uint16_t i;

    learn_state(sen, &state);

    /* Greedily select the best action using Q. */
    action.id = 0;
    for (i = 1; i < LEARN_ACTIONS; ++i) {
        if (config->q[state.id][i] > config->q[state.id][action.id]) {
            action.id = i;
        }
    }

    learn_motor(&action, mot);
}


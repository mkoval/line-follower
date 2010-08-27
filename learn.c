#include <stdlib.h>
#include "config.h"
#include "libarduino2.h"
#include "learn.h"

static float g_reward[LEARN_STATES] = {
    1.0, 5.0, 5.0, 1.0
};

void learn_state(sensor_t const *sen, state_t *state) {
    float weight[LEARN_STATES];
    uint8_t i;
    
    weight[0] = sen->value[SENSOR_FARRIGHT];
    weight[1] = sen->value[SENSOR_MIDRIGHT];
    weight[2] = sen->value[SENSOR_MIDLEFT];
    weight[3] = sen->value[SENSOR_FARLEFT];

    /* Select the state that is most probable. */
    state->id = 0;
    for (i = 1; i < LEARN_STATES; ++i) {
        if (weight[i] > weight[state->id]) {
            state->id = i;
        }
    }
}

void learn_motor(action_t const *act, motor_t *motor) {
    int16_t normal = LEARN_ACTIONS / 2;
    int16_t middle = (LEARN_ACTIONS + 1) / 2;
    int16_t delta  = act->id - middle;

    /* Turn right by slowing the left motor. */
    if (delta < 0) {
        motor->left  = -delta / (float)normal;
        motor->right = 1.0f;
    }
    /* Turn left by slowing the right motor. */
    else if (delta > 0) {
        motor->left  = 1.0f;
        motor->right = +delta / (float)normal;
    }
    /* Drive perfectly straight. */
    else {
        motor->left  = 0.0f;
        motor->right = 0.0f;
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


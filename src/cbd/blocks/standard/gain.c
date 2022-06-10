#include "standard.h"
#include "../../block.h"
#include "../../../sim.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{
    double * in;
    double * out;
    double * gain;
} eval_state_gain_t;

void eval_gain(cbd_block_t * block, sim_state_t * state){
    if(!block->cache){
        // Create simulation cache

        d_array_t * pin = (d_array_t*)d_array_at(&state->arrays, block->ports_in);
        d_array_t * pout = (d_array_t*)d_array_at(&state->arrays, block->ports_out);
        d_array_t * params = (d_array_t*)d_array_at(&state->arrays, block->params);

        eval_state_gain_t * cache = malloc(sizeof(eval_state_gain_t));

        int in = ((int*)pin->begin)[0];
        int out = ((int*)pout->begin)[0];
        int gain = ((int*)params->begin)[0];

        cbd_signal_t * s_in = d_array_at(&state->cbd_signals, in);
        cache->in = d_array_at(&state->values, s_in->value);
        cbd_signal_t * s_out = d_array_at(&state->cbd_signals, out);
        cache->out = d_array_at(&state->values, s_out->value);

        cbd_signal_t * p_gain = d_array_at(&state->cbd_params, gain);
        cache->gain = d_array_at(&state->values, p_gain->value);

        block->cache = cache;
    }
    eval_state_gain_t * c = (eval_state_gain_t*)block->cache;

    *c->out = *c->gain* *c->in;
}

int cbd_block_standard_gain(int in, int out, int gain, const char * name, sim_state_t * state){
    const int pin[1] = {in};
    const int pout[1] = {out};
    const int params[1] = {gain};
    int block = cbd_block_add(name, pin, 1, pout, 1, params, 1, eval_gain, 0, state);

    return block;
}
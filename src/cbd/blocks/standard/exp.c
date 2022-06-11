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
} eval_state_exp_t;

void eval_exp(cbd_block_t * block, sim_state_t * state){
    if(!block->cache){
        // Create simulation cache

        d_array_t * pin = (d_array_t*)d_array_at(&state->arrays, block->ports_in);
        d_array_t * pout = (d_array_t*)d_array_at(&state->arrays, block->ports_out);
        d_array_t * params = (d_array_t*)d_array_at(&state->arrays, block->params);

        eval_state_exp_t * cache = malloc(sizeof(eval_state_exp_t));

        int in = ((int*)pin->begin)[0];
        int out = ((int*)pout->begin)[0];
        int gain = ((int*)params->begin)[0];

        cbd_signal_t * s_in = d_array_at(&state->cbd_signals, in);
        cache->in = d_array_at(&state->values, s_in->value);
        cbd_signal_t * s_out = d_array_at(&state->cbd_signals, out);
        cache->out = d_array_at(&state->values, s_out->value);

        block->cache = cache;
    }
    eval_state_exp_t * c = (eval_state_exp_t*)block->cache;

    *c->out = exp(*c->in);
}

int cbd_block_standard_exp(int in, int out, const char * name, sim_state_t * state){
    const int pin[1] = {in};
    const int pout[1] = {out};
    const int params[0] = {};
    int block = cbd_block_add(name, pin, 1, pout, 1, params, 0, STANDARD_EXP, 0, state);

    return block;
}
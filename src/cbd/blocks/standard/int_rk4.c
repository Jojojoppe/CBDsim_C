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
    double * state;
    double * initial;
    double * timestep;
    double in_prev1;
    double in_prev2;
    int sample;
} eval_state_int_rk4_t;

void eval_int_rk4(cbd_block_t * block, sim_state_t * state){
    if(!block->cache){
        // Create simulation cache

        d_array_t * pin = (d_array_t*)d_array_at(&state->arrays, block->ports_in);
        d_array_t * pout = (d_array_t*)d_array_at(&state->arrays, block->ports_out);
        d_array_t * params = (d_array_t*)d_array_at(&state->arrays, block->params);

        eval_state_int_rk4_t * cache = malloc(sizeof(eval_state_int_rk4_t));

        int in = ((int*)pin->begin)[0];
        int intstate = ((int*)pin->begin)[1];
        int out = ((int*)pout->begin)[0];
        int initial = ((int*)params->begin)[0];
        int timestep = state->timestep;

        cbd_signal_t * s_in = d_array_at(&state->cbd_signals, in);
        cache->in = d_array_at(&state->values, s_in->value);
        cbd_signal_t * s_out = d_array_at(&state->cbd_signals, out);
        cache->out = d_array_at(&state->values, s_out->value);
        cbd_signal_t * s_state = d_array_at(&state->cbd_signals, intstate);
        cache->state = d_array_at(&state->values, s_state->value);

        cbd_param_t * p_0 = d_array_at(&state->cbd_params, initial);
        cache->initial = d_array_at(&state->values, p_0->value);

        cbd_param_t * p_1 = d_array_at(&state->cbd_params, timestep);
        cache->timestep = d_array_at(&state->values, p_1->value);

        cache->in_prev1 = *cache->in;
        cache->in_prev2 = *cache->in;
        *cache->state = *cache->initial;
        cache->sample = 0;

        // FIXME this should kinda happen but not like this...
        // *cache->timestep /= 2.0;

        block->cache = cache;
    }
    eval_state_int_rk4_t * c = (eval_state_int_rk4_t*)block->cache;

    if(c->sample){
        *c->out = *c->state + *c->timestep/3.0 * (c->in_prev1 + 4*c->in_prev2 + *c->in);
        *c->state = *c->out;
    }
    c->in_prev1 = c->in_prev2;
    c->in_prev2 = *c->in;
    c->sample ^= -1;
}

int cbd_block_standard_int_rk4(int in, int out, int initial, const char * name, sim_state_t * state){
    char * nnew = calloc(strlen(name)+10, 1);
    sprintf(nnew, "%s_state", name);
    int intstate = cbd_signal_add(nnew, state);
    free(nnew);
    const int pin[2] = {in, intstate};
    const int pout[2] = {out, intstate};
    const int params[1] = {initial};
    int block = cbd_block_add(name, pin, 2, pout, 2, params, 1, STANDARD_INT_RK4, 1, state);

    return block;
}
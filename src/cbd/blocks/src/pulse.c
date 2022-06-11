#include "src.h"
#include "../../block.h"
#include "../../../sim.h"

#include <math.h>
#include <stdlib.h>

typedef struct{
    double * time;
    double * out;
    double * A;
    double * t1;
    double * t2;
} eval_state_pulse_t;

void eval_pulse(cbd_block_t * block, sim_state_t * state){
    if(!block->cache){
        // Create simulation cache

        d_array_t * pin = (d_array_t*)d_array_at(&state->arrays, block->ports_in);
        d_array_t * pout = (d_array_t*)d_array_at(&state->arrays, block->ports_out);
        d_array_t * params = (d_array_t*)d_array_at(&state->arrays, block->params);

        eval_state_pulse_t * cache = malloc(sizeof(eval_state_pulse_t));

        int time = ((int*)pin->begin)[0];
        int out = ((int*)pout->begin)[0];
        int A = ((int*)params->begin)[0];
        int t1 = ((int*)params->begin)[1];
        int t2 = ((int*)params->begin)[2];
        
        cbd_signal_t * s_t = d_array_at(&state->cbd_signals, time);
        cache->time = d_array_at(&state->values, s_t->value);
        cbd_signal_t * s_out0 = d_array_at(&state->cbd_signals, out);
        cache->out = d_array_at(&state->values, s_out0->value);
        cbd_param_t * p_0 = d_array_at(&state->cbd_params, A);
        cache->A = d_array_at(&state->values, p_0->value);
        cbd_param_t * p_1 = d_array_at(&state->cbd_params, t1);
        cache->t1 = d_array_at(&state->values, p_1->value);
        cbd_param_t * p_2 = d_array_at(&state->cbd_params, t2);
        cache->t2 = d_array_at(&state->values, p_2->value);

        block->cache = cache;
    }
    eval_state_pulse_t * c = (eval_state_pulse_t*)block->cache;
    if(*c->time<*c->t1)
        *c->out = 0.0;
    else if(*c->time>=*c->t1 && *c->time<*c->t2)
        *c->out = *c->A;
    else
        *c->out = 0.0;
}

int cbd_blocks_src_pulse(int out, int A, int t1, int t2, const char * name, sim_state_t * state){
    const int pin[1] = {state->time};
    const int pout[1] = {out};
    const int params[3] = {A, t1, t2};
    int block = cbd_block_add(name, pin, 1, pout, 1, params, 3, SRC_PULSE, 0, state);

    return block;
}
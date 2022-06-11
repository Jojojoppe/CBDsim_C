#include "src.h"
#include "../../block.h"
#include "../../../sim.h"

#include <math.h>
#include <stdlib.h>

typedef struct{
    double * time;
    double * out;
    double * A;
} eval_state_constant_t;

void eval_constant(cbd_block_t * block, sim_state_t * state){
    if(!block->cache){
        // Create simulation cache

        d_array_t * pin = (d_array_t*)d_array_at(&state->arrays, block->ports_in);
        d_array_t * pout = (d_array_t*)d_array_at(&state->arrays, block->ports_out);
        d_array_t * params = (d_array_t*)d_array_at(&state->arrays, block->params);

        eval_state_constant_t * cache = malloc(sizeof(eval_state_constant_t));

        int time = ((int*)pin->begin)[0];
        int out = ((int*)pout->begin)[0];
        int A = ((int*)params->begin)[0];
        
        cbd_signal_t * s_t = d_array_at(&state->cbd_signals, time);
        cache->time = d_array_at(&state->values, s_t->value);
        cbd_signal_t * s_out0 = d_array_at(&state->cbd_signals, out);
        cache->out = d_array_at(&state->values, s_out0->value);
        cbd_param_t * p_0 = d_array_at(&state->cbd_params, A);
        cache->A = d_array_at(&state->values, p_0->value);

        block->cache = cache;
    }
    eval_state_constant_t * c = (eval_state_constant_t*)block->cache;
    *c->out = *c->A;
}

int cbd_blocks_src_constant(int out, int A, const char * name, sim_state_t * state){
    const int pin[1] = {state->time};
    const int pout[1] = {out};
    const int params[1] = {A};
    int block = cbd_block_add(name, pin, 1, pout, 1, params, 1, SRC_CONSTANT, 0, state);

    return block;
}
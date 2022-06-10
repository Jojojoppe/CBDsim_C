#include "../../block.h"
#include "../../../sim.h"

#include <math.h>

typedef struct{
    double * time;
    double * out;
    double * A;
    double * f;
} eval_state_sin_t;

void eval_sin(cbd_block_t * block, sim_state_t * state){
    if(!block->cache){
        // Create simulation cache
        eval_state_sin_t * cache = malloc(sizeof(eval_state_sin_t));

        int * pin = (int*)d_array_at(&state->arrays, block->ports_in);
        int * pout = (int*)d_array_at(&state->arrays, block->ports_out);
        int * params = (int*)d_array_at(&state->arrays, block->params);

        int time = pin[0];
        int out = pout[0];
        int A = params[0];
        int f = params[1];
        
        cbd_signal_t * s_t = d_array_at(&state->cbd_signals, time);
        cache->time = d_array_at(&state->values, s_t->value);
        cbd_signal_t * s_out0 = d_array_at(&state->cbd_signals, out);
        cache->out = d_array_at(&state->values, s_out0->value);
        cbd_param_t * p_0 = d_array_at(&state->cbd_signals, A);
        cache->A = d_array_at(&state->values, p_0->value);
        cbd_param_t * p_1 = d_array_at(&state->cbd_signals, f);
        cache->f = d_array_at(&state->values, p_1->value);

        block->cache = cache;
    }
    eval_state_sin_t * c = (eval_state_sin_t*)block->cache;
    *c->out = *c->A * sin(2*M_PI* *c->f * *c->time);
}

int cbd_blocks_src_sin(int time, int out, int A, int f, const char * name, sim_state_t * state){
    const int pin[1] = {time};
    const int pout[1] = {out};
    const int params[2] = {A, f};
    int block = cbd_block_add(name, pin, 1, pout, 1, params, 2, eval_sin, state);

    return block;
}
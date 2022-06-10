#include "standard.h"
#include "../../block.h"
#include "../../../sim.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int inp_n;
    int inm_n;
    double * out;
    double ** in;
} eval_plusmin_state_t;

void eval_plusmin(cbd_block_t * block, sim_state_t * state){
    if(!block->cache){
        // Create simulation cache

        d_array_t * pin = (d_array_t*)d_array_at(&state->arrays, block->ports_in);
        d_array_t * pout = (d_array_t*)d_array_at(&state->arrays, block->ports_out);
        d_array_t * params = (d_array_t*)d_array_at(&state->arrays, block->params);

        eval_plusmin_state_t * cache = malloc(sizeof(eval_plusmin_state_t)+sizeof(double*)*pin->filled_size);
        cache->in = (double**)(cache+1);
        cache->inp_n = *(int*)d_array_at(params, 0);
        cache->inm_n = *(int*)d_array_at(params, 1);
        
        int out = ((int*)pout->begin)[0];
        cbd_signal_t * s_out0 = d_array_at(&state->cbd_signals, out);
        cache->out = d_array_at(&state->values, s_out0->value);

        block->cache = cache;
    }
    eval_plusmin_state_t * c = (eval_plusmin_state_t*)block->cache;
    double v = 0.0;
    for(int i=0; i<c->inp_n; i++) v+=*c->in[i];
    for(int i=c->inm_n; i<c->inp_n+c->inm_n; i++) v-=*c->in[i];
    *c->out = v;
}

int cbd_block_standard_plusmin(const int * inp, int inp_n, const int * inm, int inm_n, int out, const char * name, sim_state_t * state){
    int * pin = malloc(sizeof(int)*(inp_n + inm_n));
    for(int i=0; i<inp_n; i++) pin[i] = inp[i];
    for(int i=inp_n; i<inp_n+inm_n; i++) pin[i] = inm[i-inp_n];
    const int params[2] = {inp_n, inm_n};
    const int pout[1] = {out};
    int block = cbd_block_add(name, pin, inp_n+inm_n, pout, 1, params, 1, eval_plusmin, 0, state);
    free(pin);
    return block;
}
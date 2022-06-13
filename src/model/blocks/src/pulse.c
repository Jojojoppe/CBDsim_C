#include "../blocks.h"
#include "../../model.h"
#include "../../block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void src_pulse_generate(FILE * f, block_t * block){
    int pv0 = D_ARRAY_ATV(int, &block->parameters, 0);
    int pv1 = D_ARRAY_ATV(int, &block->parameters, 1);
    int pt0 = D_ARRAY_ATV(int, &block->parameters, 2);
    int pt1 = D_ARRAY_ATV(int, &block->parameters, 3);
    int out = D_ARRAY_ATV(int, &block->ports_out, 0);
    fprintf(f, "signals[%d] = time>params[%d] && time<=params[%d] ? params[%d] : params[%d];\n", out, pt0, pt1, pv1, pv0);
}

void src_pulse_generate_init(FILE * f, block_t * block){
}

int blocks_add_src_pulse(double v0, double v1, double t0, double t1, const char * name, int out, model_t * model){
    // Create parameters
    BLOCK_PARAM(v0, name, v0, model)
    BLOCK_PARAM(v1, name, v1, model)
    BLOCK_PARAM(t0, name, t0, model)
    BLOCK_PARAM(t1, name, t1, model)

    int pin[0] = {};
    int pout[1] = {out};
    int params[4] = {pv0, pv1, pt0, pt1};
    return model_add_block(pin, 0, pout, 1, params, 4, name, 
        "src/pulse", 0, src_pulse_generate, src_pulse_generate_init, model
    );
}
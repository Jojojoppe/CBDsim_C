#include "../blocks.h"
#include "../../model.h"
#include "../../block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void src_impulse_generate(FILE * f, block_t * block){
    int pA = D_ARRAY_ATV(int, &block->parameters, 0);
    int pt = D_ARRAY_ATV(int, &block->parameters, 1);
    int out = D_ARRAY_ATV(int, &block->ports_out, 0);
    fprintf(f, "signals[%d] = time>=params[%d] && time<params[%d]+1.5*timestep ? params[%d]/timestep : 0.0;\n", out, pt, pt, pA);
}

void src_impulse_generate_init(FILE * f, block_t * block){
}

int blocks_add_src_impulse(double A, double t, const char * name, int out, model_t * model){
    // Create parameters
    BLOCK_PARAM(A, name, A, model)
    BLOCK_PARAM(t, name, t, model)

    int pin[0] = {};
    int pout[1] = {out};
    int params[2] = {pA, pt};
    return model_add_block(pin, 0, pout, 1, params, 2, name, 
        "src/impulse", 0, src_impulse_generate, src_impulse_generate_init, model
    );
}
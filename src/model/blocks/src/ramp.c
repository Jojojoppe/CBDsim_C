#include "../blocks.h"
#include "../../model.h"
#include "../../block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void src_ramp_generate(FILE * f, block_t * block){
    int pa = D_ARRAY_ATV(int, &block->parameters, 0);
    int pb = D_ARRAY_ATV(int, &block->parameters, 1);
    int out = D_ARRAY_ATV(int, &block->ports_out, 0);
    fprintf(f, "signals[%d] = time*params[%d] + params[%d];\n", out, pa, pb);
}

void src_ramp_generate_init(FILE * f, block_t * block){
}

int blocks_add_src_ramp(double a, double b, const char * name, int out, model_t * model){
    // Create parameters
    BLOCK_PARAM(a, name, a, model)
    BLOCK_PARAM(b, name, b, model)

    int pin[0] = {};
    int pout[1] = {out};
    int params[2] = {pa, pb};
    return model_add_block(pin, 0, pout, 1, params, 2, name, 
        "src/ramp", 0, src_ramp_generate, src_ramp_generate_init, model
    );
}
#include "../blocks.h"
#include "../../model.h"
#include "../../block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void src_constant_generate(FILE * f, block_t * block){
    int pv = D_ARRAY_ATV(int, &block->parameters, 0);
    int out = D_ARRAY_ATV(int, &block->ports_out, 0);
    fprintf(f, "signals[%d] = params[%d];\n", out, pv);
}

void src_constant_generate_init(FILE * f, block_t * block){
}

int blocks_add_src_constant(double v, const char * name, int out, model_t * model){
    // Create parameters
    BLOCK_PARAM(v, name, v, model)

    int pin[0] = {};
    int pout[1] = {out};
    int params[1] = {pv};
    return model_add_block(pin, 0, pout, 1, params, 1, name, 
        "src/step", 0, src_constant_generate, src_constant_generate_init, model
    );
}
#include "../blocks.h"
#include "../../model.h"
#include "../../block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void src_istep_generate(FILE * f, block_t * block){
    int pv = D_ARRAY_ATV(int, &block->parameters, 0);
    int pt = D_ARRAY_ATV(int, &block->parameters, 1);
    int out = D_ARRAY_ATV(int, &block->ports_out, 0);
    fprintf(f, "signals[%d] = time>params[%d] ? 0.0 : params[%d];\n", out, pt, pv);
}

void src_istep_generate_init(FILE * f, block_t * block){
}

int blocks_add_src_istep(double v, double t, const char * name, int out, model_t * model){
    // Create parameters
    BLOCK_PARAM(v, name, v, model)
    BLOCK_PARAM(t, name, t, model)

    int pin[0] = {};
    int pout[1] = {out};
    int params[2] = {pv, pt};
    return model_add_block(pin, 0, pout, 1, params, 2, name, 
        "src/istep", 0, src_istep_generate, src_istep_generate_init, model
    );
}
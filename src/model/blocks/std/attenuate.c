#include "../blocks.h"
#include "../../model.h"
#include "../../block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void std_attenuate_generate(FILE * f, block_t * block){
    int pk = D_ARRAY_ATV(int, &block->parameters, 0);
    int out = D_ARRAY_ATV(int, &block->ports_out, 0);
    int in = D_ARRAY_ATV(int, &block->ports_in, 0);
    fprintf(f, "signals[%d] = 1/params[%d]*signals[%d];\n", out, pk, in);
}

void std_attenuate_generate_init(FILE * f, block_t * block){
}

int blocks_add_std_attenuate(double k, const char * name, int in, int out, model_t * model){
    // Create parameters
    BLOCK_PARAM(k, name, k, model)

    int pin[1] = {in};
    int pout[1] = {out};
    int params[1] = {pk};
    return model_add_block(pin, 1, pout, 1, params, 1, name, 
        "std/attenuate", 0, std_attenuate_generate, std_attenuate_generate_init, model
    );
}
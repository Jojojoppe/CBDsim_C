#include "../blocks.h"
#include "../../model.h"
#include "../../block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void std_integrate_generate(FILE * f, block_t * block){
    int pinitial = D_ARRAY_ATV(int, &block->parameters, 0);
    int out = D_ARRAY_ATV(int, &block->ports_out, 0);
    int in = D_ARRAY_ATV(int, &block->ports_in, 0);
    fprintf(f, "signals[%d] = INT(signals[%d], params[%d]);\n", out, in, pinitial);
}

void std_integrate_generate_init(FILE * f, block_t * block){
}

int blocks_add_std_integrate(double initial, const char * name, int in, int out, model_t * model){
    // Create parameters
    BLOCK_PARAM(initial, name, initial, model)

    int pin[1] = {in};
    int pout[1] = {out};
    int params[1] = {pinitial};
    return model_add_block(pin, 1, pout, 1, params, 1, name, 
        "std/integrate", 0, std_integrate_generate, std_integrate_generate_init, model
    );
}
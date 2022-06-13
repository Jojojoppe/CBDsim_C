#include "../blocks.h"
#include "../../model.h"
#include "../../block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void std_tan_generate(FILE * f, block_t * block){
    int out = D_ARRAY_ATV(int, &block->ports_out, 0);
    int in = D_ARRAY_ATV(int, &block->ports_in, 0);
    fprintf(f, "signals[%d] = tan(signals[%d]);\n", out, in);
}

void std_tan_generate_init(FILE * f, block_t * block){
}

int blocks_add_std_tan(const char * name, int in, int out, model_t * model){
    int pin[1] = {in};
    int pout[1] = {out};
    int params[0] = {};
    return model_add_block(pin, 1, pout, 1, params, 0, name, 
        "std/tan", 0, std_tan_generate, std_tan_generate_init, model
    );
}
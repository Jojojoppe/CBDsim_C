#include "../blocks.h"
#include "../../model.h"
#include "../../block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void std_log_generate(FILE * f, block_t * block){
    int out = D_ARRAY_ATV(int, &block->ports_out, 0);
    int in = D_ARRAY_ATV(int, &block->ports_in, 0);
    fprintf(f, "signals[%d] = log(signals[%d]);\n", out, in);
}

void std_log_generate_init(FILE * f, block_t * block){
}

int blocks_add_std_log(const char * name, int in, int out, model_t * model){
    int pin[1] = {in};
    int pout[1] = {out};
    int params[0] = {};
    return model_add_block(pin, 1, pout, 1, params, 0, name, 
        "std/log", 0, std_log_generate, std_log_generate_init, model
    );
}
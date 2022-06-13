#include "../blocks.h"
#include "../../model.h"
#include "../../block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void std_plusmin_generate(FILE * f, block_t * block){
    int out = D_ARRAY_ATV(int, &block->ports_out, 0);
    int pin_n = D_ARRAY_ATV(int, &block->genparams_int, 0);
    fprintf(f, "signals[%d] = 0.0 ", out);
    for(int i=0; i<pin_n; i++) fprintf(f, "+ signals[%d] ", D_ARRAY_ATV(int, &block->ports_in, i));
    for(int i=pin_n; i<D_ARRAY_LEN(block->ports_in); i++) fprintf(f, "- signals[%d] ", D_ARRAY_ATV(int, &block->ports_in, i));

    fprintf(f, ";\n");
}

void std_plusmin_generate_init(FILE * f, block_t * block){
}

int blocks_add_std_plusmin(const char * name, int * pin, int pin_n, int * min, int min_n, int out, model_t * model){

    int * ps = calloc(pin_n + min_n, sizeof(int));
    for(int i=0; i<pin_n; i++) ps[i] = pin[i];
    for(int i=pin_n; i<pin_n+min_n; i++) ps[i] = min[i-pin_n];

    int pout[1] = {out};
    int params[0] = {};
    int b = model_add_block(ps, pin_n + min_n, pout, 1, params, 0, name, 
        "std/plusmin", 0, std_plusmin_generate, std_plusmin_generate_init, model
    );

    // Add generation parameter
    block_t * block = D_ARRAY_ATV(block_t *, &model->blocks, b);
    d_array_insert(&block->genparams_int, &pin_n);

    free(ps);
    return b;
}
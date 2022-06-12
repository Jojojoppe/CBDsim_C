#include "../blocks.h"
#include "../../model.h"
#include "../../block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void src_cosine_generate(FILE * f, block_t * block){
    int pA = D_ARRAY_ATV(int, &block->parameters, 0);
    int pf = D_ARRAY_ATV(int, &block->parameters, 1);
    int out = D_ARRAY_ATV(int, &block->ports_out, 0);
    fprintf(f, "signals[%d] = params[%d]*cos(2*M_PI*params[%d]*time);\n", out, pA, pf);
}

void src_cosine_generate_init(FILE * f, block_t * block){
}

int blocks_add_src_cosine(double A, double f, const char * name, int out, model_t * model){
    // Create parameters
    char * pAname = calloc(1, strlen(name)+strlen("/A")+1);
    char * pfname = calloc(1, strlen(name)+strlen("/f")+1);
    sprintf(pAname, "%s/A", name);
    sprintf(pfname, "%s/f", name);
    int pA = model_add_param(pAname, A, model);
    int pf = model_add_param(pfname, f, model);
    free(pAname);
    free(pfname);

    const int pin[0] = {};
    const int pout[1] = {out};
    const int params[2] = {pA, pf};
    return model_add_block(pin, 0, pout, 1, params, 2, name, 
        "src/cosine", 0, src_cosine_generate, src_cosine_generate_init, model
    );
}
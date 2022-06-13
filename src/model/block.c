#include "block.h"

#include <stdlib.h>
#include <string.h>

block_t * block_init(int * ports_in, size_t ports_in_n, int * ports_out, size_t ports_out_n, 
            int * parameters, size_t parameters_n, const char * name, const char * type, int statefull, 
            void (*generate)(FILE* f, block_t * block), void (*generate_init)(FILE * f, block_t * block))
        {
    block_t * b = calloc(1, sizeof(block_t));

    D_ARRAY_INIT(int, &b->ports_in);
    D_ARRAY_INIT(int, &b->ports_out);
    D_ARRAY_INIT(int, &b->parameters);
    for(size_t i=0; i<ports_in_n; i++) d_array_insert(&b->ports_in, &ports_in[i]);
    for(size_t i=0; i<ports_out_n; i++) d_array_insert(&b->ports_out, &ports_out[i]);
    for(size_t i=0; i<parameters_n; i++) d_array_insert(&b->parameters, &parameters[i]);

    b->name = malloc(strlen(name)+1);
    strcpy(b->name, name);

    b->type = malloc(strlen(type)+1);
    strcpy(b->type, type);

    b->statefull = statefull;
    b->generate = generate;
    b->generate_init = generate_init;

    return b;
}

void block_deinit(block_t * block){
    if(!block) return;
    d_array_deinit(&block->ports_in);
    d_array_deinit(&block->ports_out);
    d_array_deinit(&block->parameters);
    free(block->name);
    free(block->type);
    free(block);
}
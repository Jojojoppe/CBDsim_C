#ifndef __H_MODEL_BLOCK
#define __H_MODEL_BLOCK

#include "../dynamic_array.h"

#include <stdio.h>

typedef struct block_s {
    d_array_t ports_in;
    d_array_t ports_out;
    d_array_t parameters;
    char * name;
    char * type;
    int statefull;
    void (*generate)(FILE * f, struct block_s * block);
    void (*generate_init)(FILE * f, struct block_s * block);
} block_t;

block_t * block_init(int * ports_in, size_t ports_in_n, int * ports_out, size_t ports_out_n, 
    int * parameters, size_t parameters_n, const char * name, const char * type, 
    int statefull, void (*generate)(FILE* f, block_t * block), void (*generate_init)(FILE * f, block_t * block)
);
void block_deinit(block_t * block);

#endif
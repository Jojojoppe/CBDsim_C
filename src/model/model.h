#ifndef __H_MODEL
#define __H_MODEL

#include "../dynamic_array.h"
#include "signal.h"
#include "param.h"
#include "block.h"

typedef struct model_s {
    d_array_t signals;
    d_array_t params;
    d_array_t blocks;
} model_t;

model_t * model_init();
void model_deinit(model_t * model);

int model_add_signal(const char * name, model_t * model);
int model_add_param(const char * name, double value, model_t * model);
int model_add_block(const int * ports_in, size_t ports_in_n, const int * ports_out, size_t ports_out_n, 
    const int * parameters, size_t parameters_n, const char * name, const char * type, int statefull, 
    void (*generate)(FILE* f, block_t * block), void (*generate_init)(FILE* f, block_t * block), model_t * model
);
int model_get_signal(const char * name, model_t * model);
int model_get_param(const char * name, model_t * model);
int model_get_block(const char * name, model_t * model);

int model_compile(const char * outfile, model_t * model);

#endif
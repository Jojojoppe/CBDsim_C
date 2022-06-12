#include "model.h"

#include <stdlib.h>
#include <string.h>

model_t * model_init(){
    model_t * m = calloc(1, sizeof(model_t));
    D_ARRAY_INIT(signal_t*, &m->signals);
    D_ARRAY_INIT(param_t*, &m->params);
    D_ARRAY_INIT(block_t*, &m->blocks);

    // All models have time and timestep
    model_add_signal("time", m);
    model_add_signal("timestep", m);

    return m;
}

void model_deinit(model_t * model){
    if(!model) return;
    for(int i=0; i<D_ARRAY_LEN(model->signals); i++)
        signal_deinit(D_ARRAY_ATV(signal_t*, &model->signals, i));
    d_array_deinit(&model->signals);
    for(int i=0; i<D_ARRAY_LEN(model->params); i++)
        param_deinit(D_ARRAY_ATV(param_t*, &model->params, i));
    d_array_deinit(&model->params);
    for(int i=0; i<D_ARRAY_LEN(model->blocks); i++)
        block_deinit(D_ARRAY_ATV(block_t*, &model->blocks, i));
    d_array_deinit(&model->blocks);
    free(model);
}

int model_add_signal(const char * name, model_t * model){
    if(!model) return -1;
    // TODO use hashmap to store name->id pairs
    signal_t * s = signal_init(name);
    d_array_insert(&model->signals, &s);
    return D_ARRAY_LEN(model->signals)-1;
}

int model_add_param(const char * name, double value, model_t * model){
    if(!model) return -1;
    // TODO use hashmap to store name->id pairs
    param_t * s = param_init(name, value);
    d_array_insert(&model->params, &s);
    return D_ARRAY_LEN(model->params)-1;
}

int model_add_block(const int * ports_in, size_t ports_in_n, const int * ports_out, size_t ports_out_n, 
            const int * parameters, size_t parameters_n, const char * name, const char * type, int statefull, 
            void (*generate)(FILE* f, block_t * block), void (*generate_init)(FILE* f, block_t * block), model_t * model)
        {
    if(!model) return -1;
    // TODO use hashmap to store name->id pairs
    block_t * s = block_init(ports_in, ports_in_n, ports_out, ports_out_n, parameters, parameters_n, 
        name, type, statefull, generate, generate_init
    );
    d_array_insert(&model->blocks, &s);
    return D_ARRAY_LEN(model->blocks)-1;
}

int model_get_signal(const char * name, model_t * model){
    if(!model) return -1;
    for(int i=0; i<D_ARRAY_LEN(model->signals); i++){
        char * n = (D_ARRAY_ATV(signal_t*, &model->signals, i))->name;
        if(!strcmp(name, n)) return i;
    }
    return -1;
}

int model_get_param(const char * name, model_t * model){
    if(!model) return -1;
    for(int i=0; i<D_ARRAY_LEN(model->params); i++){
        char * n = (D_ARRAY_ATV(param_t*, &model->params, i))->name;
        if(!strcmp(name, n)) return i;
    }
    return -1;
}

int model_get_block(const char * name, model_t * model){
    if(!model) return -1;
    for(int i=0; i<D_ARRAY_LEN(model->blocks); i++){
        char * n = (D_ARRAY_ATV(block_t*, &model->blocks, i))->name;
        if(!strcmp(name, n)) return i;
    }
    return -1;
}
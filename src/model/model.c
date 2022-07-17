#include "model.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

model_t * model_init(char * name){
    if(!name) return NULL;
    model_t * m = calloc(sizeof(model_t), 1);
    if(!m) return NULL;

    char * n = malloc(strlen(name)+1);
    if(!n){
        free(m);
        return NULL;
    };
    strcpy(n, name);
    m->name = n;

    D_ARRAY_INIT(parameter_t, &m->parameters);
    D_ARRAY_INIT(variable_t, &m->variables);
    D_ARRAY_INIT(signal_t, &m->signals);
    D_ARRAY_INIT(block_t, &m->blocks);

    D_ARRAY_INIT(int, &m->in);
    D_ARRAY_INIT(int, &m->out);

    m->hmap = hashmap_create();

    return m;
}

void model_deinit(model_t * model){
    if(!model) return;

    if(model->hmap) hashmap_free(model->hmap);

    if(model->name) free(model->name);

    d_array_deinit(&model->in);
    d_array_deinit(&model->out);

    for(D_ARRAY_LOOP(parameter_t, it, model->parameters)){
        if(it->name) free(it->name);
    }
    d_array_deinit(&model->parameters);

    for(D_ARRAY_LOOP(variable_t, it, model->variables)){
        if(it->name) free(it->name);
    }
    d_array_deinit(&model->variables);

    for(D_ARRAY_LOOP(signal_t, it, model->signals)){
        d_array_deinit(&it->to_index);
        if(it->name) free(it->name);
    }
    d_array_deinit(&model->signals);

    for(D_ARRAY_LOOP(block_t, it, model->blocks)){
        d_array_deinit(&it->in);
        d_array_deinit(&it->out);
        d_array_deinit(&it->parameters);
        d_array_deinit(&it->variables);
        if(it->name) free(it->name);
        if(it->gen_params) free(it->gen_params);
    }
    d_array_deinit(&model->blocks);

    free(model);
}

void model_debug(model_t * model){
    printf("Model debug print [%p]: ", model);
    if(!model){
        printf("xxx model not initialized\n");
        return;
    }
    printf("%s\n", model->name);

    printf("Paramters:\n");
    for(D_ARRAY_LOOP(parameter_t, it, model->parameters)){
        printf("\t%s = %f\n", it->name, it->value);
    }

    printf("Variables:\n");
    for(D_ARRAY_LOOP(variable_t, it, model->variables)){
        printf("\t%s = %f\n", it->name, it->value);
    }

    printf("Signals:\n");
    for(D_ARRAY_LOOP(signal_t, it, model->signals)){
        printf("\t%s [", it->name);
        if(it->from_index>=0){
            signal_t * from = D_ARRAY_ATP(signal_t, &model->signals, it->from_index);
            printf("%s --> {", from->name);
        }else{
            printf("xx --> {");
        }
        for(D_ARRAY_LOOP(int, jt, it->to_index)){
            if(*jt<0) continue;
            signal_t * to = D_ARRAY_ATP(signal_t, &model->signals, *jt);
            printf("%s,", to->name);
        }
        printf("}]\n");
    }

    printf("Blocks:\n");
    for(D_ARRAY_LOOP(block_t, it, model->blocks)){
        printf("\t%s [-> %s]\n", it->name, it->block_def->type);
    }
}

int model_add_parameter(char * name, double value, model_t * model){
    if(!model) return -1;
    if(!name) return -1;

    char * n = malloc(strlen(name)+1);
    if(!n) return -1;
    strcpy(n, name);

    parameter_t p = {
        .name = n,
        .value = value,
    };

    if(d_array_insert(&model->parameters, &p)!=D_ARRAY_ERROR_OKAY){
        free(n);
        return -1;
    }

    int id = D_ARRAY_LEN(model->parameters)-1;
    hashmap_set(model->hmap, n, strlen(n), id);
    return id;
}

int model_add_variable(char * name, double value, model_t * model){
    if(!model) return -1;
    if(!name) return -1;

    char * n = malloc(strlen(name)+1);
    if(!n) return -1;
    strcpy(n, name);

    variable_t v = {
        .name = n,
        .value = value,
    };

    if(d_array_insert(&model->variables, &v)!=D_ARRAY_ERROR_OKAY){
        free(n);
        return -1;
    }

    int id = D_ARRAY_LEN(model->variables)-1;
    hashmap_set(model->hmap, n, strlen(n), id);
    return id;
}

int model_add_signal(char * name, model_t * model){
    if(!model) return -1;
    if(!name) return -1;

    char * n = malloc(strlen(name)+1);
    if(!n) return -1;
    strcpy(n, name);

    signal_t s = {
        .name = n,
        .from_index = -1,
    };

    D_ARRAY_INIT(int, &s.to_index);

    if(d_array_insert(&model->signals, &s)!=D_ARRAY_ERROR_OKAY){
        free(n);
        d_array_deinit(&s.to_index);
        return -1;
    }

    int id = D_ARRAY_LEN(model->signals)-1;
    hashmap_set(model->hmap, n, strlen(n), id);
    return id;
}

int model_add_block(char * name, const block_definition_t * definition, double * parameters, model_t * model){
    if(!model) return -1;
    if(!name) return -1;
    if(!definition) return -1;

    char * n = malloc(strlen(name)+1);
    if(!n) return -1;
    strcpy(n, name);

    void * gp = NULL;
    if(definition->genparams_size){
        gp = calloc(1, definition->genparams_size);
        if(!gp){
            free(n);
            return -1;
        }
    }

    block_t b = {
        .name = n,
        .block_def = definition,
        .gen_params = gp,
    };

    D_ARRAY_INIT(int, &b.in);
    D_ARRAY_INIT(int, &b.out);
    D_ARRAY_INIT(int, &b.parameters);
    D_ARRAY_INIT(int, &b.variables);

    if(d_array_insert(&model->blocks, &b)!=D_ARRAY_ERROR_OKAY){
        free(n);
        free(gp);
        d_array_deinit(&b.in);
        d_array_deinit(&b.out);
        d_array_deinit(&b.parameters);
        d_array_deinit(&b.variables);
        return -1;
    }

    int id = D_ARRAY_LEN(model->blocks)-1;
    hashmap_set(model->hmap, n, strlen(n), id);

    // Create signals, parameters and variables if needed
    // If not needed (global) always add to hashmap
    int nindex = 0;
    int arr = 0;
    // Start with inputs
    while(definition->names[nindex]!=0){
        const char * in_name = definition->names[nindex];
        if((intptr_t)in_name==-1){
            // TODO implement
        }else if((intptr_t)in_name==-2){
            // TODO implement
        }else if((intptr_t)in_name<32 && (intptr_t)in_name>0){
            // TODO implement
        }else{
            size_t in_name_len = strlen(in_name);
            // Allocate space for combined name
            char * tot_name = calloc(in_name_len+strlen(name)+strlen("/")+5, 1);
            // TODO error handling
            if(arr) sprintf(tot_name, "%s/%s[%d]", name, in_name, arr--);
            else sprintf(tot_name, "%s/%s", name, in_name);
            model_add_signal(tot_name, model);
            free(tot_name);
        }
        nindex++;
    }
    // Goto outputs
    nindex++;
    arr = 0;
    while(definition->names[nindex]!=0){
        const char * out_name = definition->names[nindex];
        if((intptr_t) out_name==-1){
            // TODO implement
        }else if((intptr_t) out_name==-2){
            // TODO implement
        }else if((intptr_t) out_name<32 && (intptr_t)out_name>0){
            // TODO implement
        }else{
            size_t out_name_len = strlen(out_name);
            // Allocate space for combined name
            char * tot_name = calloc(out_name_len+strlen(name)+strlen("/")+5, 1);
            // TODO error handling
            if(arr) sprintf(tot_name, "%s/%s[%d]", name, out_name, arr--);
            else sprintf(tot_name, "%s/%s", name, out_name);
            model_add_signal(tot_name, model);
            free(tot_name);
        }
        nindex++;
    }
    // Goto parameters
    nindex++;
    int pindex = 0;
    arr = 0;
    while(definition->names[nindex]!=0){
        const char * par_name = definition->names[nindex];
        if((intptr_t) par_name==-1){
            // TODO implement
        }else if((intptr_t) par_name==-2){
            // TODO implement
        }else if((intptr_t) par_name<32 && (intptr_t)par_name>0){
            // TODO implement
        }else{
            size_t par_name_len = strlen(par_name);
            // Allocate space for combined name
            char * tot_name = calloc(par_name_len+strlen(name)+strlen("/")+5, 1);
            // TODO error handling
            if(arr) sprintf(tot_name, "%s/%s[%d]", name, par_name, arr--);
            else sprintf(tot_name, "%s/%s", name, par_name);
            model_add_parameter(tot_name, parameters[pindex++], model);
            free(tot_name);
        }
        nindex++;
    }
    // Goto variables
    nindex++;
    arr = 0;
    while(definition->names[nindex]!=0){
        const char * var_name = definition->names[nindex];
        if((intptr_t) var_name==-1){
            // TODO implement
        }else if((intptr_t) var_name==-2){
            // TODO implement
        }else if((intptr_t) var_name<32 && (intptr_t)var_name>0){
            // TODO implement
        }else{
            size_t var_name_len = strlen(var_name);
            // Allocate space for combined name
            char * tot_name = calloc(var_name_len+strlen(name)+strlen("/")+5, 1);
            // TODO error handling
            if(arr) sprintf(tot_name, "%s/%s[%d]", name, var_name, arr--);
            else sprintf(tot_name, "%s/%s", name, var_name);
            model_add_variable(tot_name, 0.0, model);
            free(tot_name);
        }
        nindex++;
    }

    return id;
}

int model_connect_signals(char * name_out, char * name_in, model_t * model){
    if(!model) return -1;
    if(!name_out) return -1;
    if(!name_in) return -1;

    // Get signal indices
    uintptr_t s_out_i;
    if(!hashmap_get(model->hmap, name_out, strlen(name_out), &s_out_i)){
        // TODO error handling
        return -1;
    }
    uintptr_t s_in_i;
    if(!hashmap_get(model->hmap, name_in, strlen(name_in), &s_in_i)){
        // TODO error handling
        return -1;
    }

    // Get signals
    signal_t * s_out, * s_in;
    s_out = D_ARRAY_ATP(signal_t, &model->signals, s_out_i);
    s_in = D_ARRAY_ATP(signal_t, &model->signals, s_in_i);

    // Check if name_in is already connected
    if(s_in->from_index>=0){
        // TODO error handling
        return -1;
    }

    // Connect signals
    s_in->from_index = s_out_i;
    d_array_insert(&s_out->to_index, &s_in_i);

    return 0;
}

int model_connect_signals_named(char * name, char * name_out, char * name_in, model_t * model){
    if(!model) return -1;
    if(!name) return -1;
    if(!name_out) return -1;
    if(!name_in) return -1;

    model_add_signal(name, model);
    model_connect_signals(name_out, name, model);
    model_connect_signals(name, name_in, model);

    return 0;
}

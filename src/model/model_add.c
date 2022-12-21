#include "model.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

    block_t * block = D_ARRAY_ATP(block_t, &model->blocks, id);

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
            int i = model_add_parameter(tot_name, parameters[pindex++], model);
            // Add to d_array
            d_array_insert(&block->parameters, &i);
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
            int i = model_add_variable(tot_name, 0.0, model);
            // Add to d_array
            d_array_insert(&block->variables, &i);
            free(tot_name);
        }
        nindex++;
    }

    return id;
}

int model_add_inout(char * name, int direction, int arr, model_t * model){
    if(!name) return -1;
    if(direction!=0 && direction!=1) return -1;
    if(!model) return -1;

    // Create signal for the in/output
    size_t name_len = strlen(name);
    // Allocate space for combined name
    char * tot_name = calloc(name_len+5, 1);
    // TODO error handling
    int ID = -1;
    if(arr){
        for(int i=0; i<arr; i++){
            sprintf(tot_name, "%s[%d]", name, i);
            int id = model_add_signal(tot_name, model);
            if(ID<0) ID = id;
            if(direction) d_array_insert(&model->in, &id);
            else d_array_insert(&model->out, &id);
        }
    }else{
        sprintf(tot_name, "%s", name);
        ID = model_add_signal(tot_name, model);
        if(direction) d_array_insert(&model->in, &ID);
        else d_array_insert(&model->out, &ID);
    }
    free(tot_name);
    return ID;
}

int model_add_submodel(char * name, model_t ** submodel, model_t * model){
    if(!name) return -1;
    if(!model) return -1;

    model_t * smod = model_init(name);
    if(!smod) return -1;

    if(d_array_insert(&model->submodels, &smod)!=D_ARRAY_ERROR_OKAY){
        model_deinit(smod);
        return -1;
    }

    // Reallocate name
    /*model_t * submdl = D_ARRAY_ATP(model_t, &model->submodels, model->submodels.filled_size-1);*/
    /*free(submdl->name);*/
    /*submdl->name = malloc(strlen(name)+1);*/
    /*strcpy(submdl->name, name);*/

    // Add input and output ports as signals to the parent model
    for(D_ARRAY_LOOP(int, it, model->in)){
        signal_t * sig = D_ARRAY_ATP(signal_t, &model->signals, *it);
        char * tot_name = calloc(strlen(sig->name)+strlen(name)+strlen("/")+5, 1);
        sprintf(tot_name, "%s/%s", name, sig->name);
        model_add_signal(tot_name, model);
        free(tot_name);
    }
    for(D_ARRAY_LOOP(int, it, model->out)){
        signal_t * sig = D_ARRAY_ATP(signal_t, &model->signals, *it);
        char * tot_name = calloc(strlen(sig->name)+strlen(name)+strlen("/")+5, 1);
        sprintf(tot_name, "%s/%s", name, sig->name);
        model_add_signal(tot_name, model);
        free(tot_name);
    }

    if(!submodel) return model->submodels.filled_size-1;
    *submodel = smod;
    return model->submodels.filled_size-1;
}

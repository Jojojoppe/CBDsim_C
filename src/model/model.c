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
    D_ARRAY_INIT(model_t*, &m->submodels);

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

    for(D_ARRAY_LOOP(model_t*, it, model->submodels)){
        model_deinit(*it);
    }
    d_array_deinit(&model->submodels);

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

void model_update(model_t * model){
    if(!model) return;

    // Loop over all submodels and add/delete input/output ports
    for(D_ARRAY_LOOP(model_t*, submodel, model->submodels)){
        // Loop over all inputs
        for(D_ARRAY_LOOP(int, it, (*submodel)->in)){
            // Check if input signal is already created
            signal_t * sig = D_ARRAY_ATP(signal_t, &(*submodel)->signals, *it);
            char * tot_name = calloc(strlen(sig->name)+strlen((*submodel)->name)+strlen("/")+5, 1);
            sprintf(tot_name, "%s/%s", (*submodel)->name, sig->name);
            intptr_t i;
            if(!hashmap_get(model->hmap, tot_name, strlen(tot_name), &i)){
                model_add_signal(tot_name, model);
            }
            free(tot_name);
        }
        // Loop over all output
        for(D_ARRAY_LOOP(int, it, (*submodel)->out)){
            // Check if output signal is already created
            signal_t * sig = D_ARRAY_ATP(signal_t, &(*submodel)->signals, *it);
            char * tot_name = calloc(strlen(sig->name)+strlen((*submodel)->name)+strlen("/")+5, 1);
            sprintf(tot_name, "%s/%s", (*submodel)->name, sig->name);
            intptr_t i;
            if(!hashmap_get(model->hmap, tot_name, strlen(tot_name), &i)){
                model_add_signal(tot_name, model);
            }
            free(tot_name);
        }
    }
}

int _model_flatten_recur(model_t * in, model_t * out, char * prefix){
    // Submodels
    for(D_ARRAY_LOOP(model_t*, it, in->submodels)){
        char * totname = calloc(strlen((*it)->name) + 1 + strlen(prefix) + 2, 1);
        sprintf(totname, "%s/%s", prefix, (*it)->name);
        if(_model_flatten_recur(*it, out, totname)){
            return -1;
        }
        free(totname);
    }

    // Blocks
    for(D_ARRAY_LOOP(block_t, it, in->blocks)){
        char * totname = calloc(strlen(it->name) + 1 + strlen(prefix) + 2, 1);
        sprintf(totname, "%s/%s", prefix, it->name);
        double * params = calloc(it->parameters.filled_size, sizeof(double));
        int cnt = 0;
        for(D_ARRAY_LOOP(int, i, it->parameters)){
            params[cnt++] = D_ARRAY_ATV(double, &in->parameters, *i);
        }
        model_add_block(totname, it->block_def, params, out);
        free(totname);
        free(params);
    }

    // Signals
    for(D_ARRAY_LOOP(signal_t, it, in->signals)){
        char * totname = calloc(strlen(it->name) + 1 + strlen(prefix) + 2, 1);
        sprintf(totname, "%s/%s", prefix, it->name);
        intptr_t i;
        if(!hashmap_get(out->hmap, totname, strlen(totname), &i)){
            model_add_signal(totname, out);
        }
        free(totname);
    }

    // Parameters
    for(D_ARRAY_LOOP(parameter_t, it, in->parameters)){
        char * totname = calloc(strlen(it->name) + 1 + strlen(prefix) + 2, 1);
        sprintf(totname, "%s/%s", prefix, it->name);
        intptr_t i;
        if(!hashmap_get(out->hmap, totname, strlen(totname), &i)){
            model_add_parameter(totname, it->value, out);
        }
        free(totname);
    }

    // Variables
    for(D_ARRAY_LOOP(variable_t, it, in->variables)){
        char * totname = calloc(strlen(it->name) + 1 + strlen(prefix) + 2, 1);
        sprintf(totname, "%s/%s", prefix, it->name);
        intptr_t i;
        if(!hashmap_get(out->hmap, totname, strlen(totname), &i)){
            model_add_variable(totname, it->value, out);
        }
        free(totname);
    }

    // Connect signals
    for(D_ARRAY_LOOP(signal_t, it, in->signals)){
        char * totname = calloc(strlen(it->name) + 1 + strlen(prefix) + 2, 1);
        sprintf(totname, "%s/%s", prefix, it->name);

        if(it->from_index>=0){
            signal_t * from = D_ARRAY_ATP(signal_t, &in->signals, it->from_index);
            char * totfname = calloc(strlen(from->name) + 1 + strlen(prefix) + 2, 1);
            sprintf(totfname, "%s/%s", prefix, from->name);
            model_connect_signals(totfname, totname, out);
            free(totfname);
        }

        for(D_ARRAY_LOOP(int, to_index, it->to_index)){
            signal_t * to = D_ARRAY_ATP(signal_t, &in->signals, *to_index);
            char * tottname = calloc(strlen(to->name) + 1 + strlen(prefix) + 2, 1);
            sprintf(tottname, "%s/%s", prefix, to->name);
            model_connect_signals(totname, tottname, out);
            free(tottname);
        }
        
        free(totname);
    }

    return 0;
}

int model_flatten(model_t * in, model_t ** out){
    if(!in) return -1;
    if(!out) return -1;

    // Initialze output
    model_t * flat = model_init(in->name);
    if(!flat) return -1;

    // Add toplevel inputs and outputs
    for(D_ARRAY_LOOP(int, it, in->in)){
        signal_t * sig = D_ARRAY_ATP(signal_t, &in->signals, *it);
        model_add_signal(sig->name, flat);
    }
    for(D_ARRAY_LOOP(int, it, in->out)){
        signal_t * sig = D_ARRAY_ATP(signal_t, &in->signals, *it);
        model_add_signal(sig->name, flat);
    }

    char * prefix = "";
    if(_model_flatten_recur(in, flat, prefix)){
        // TODO error handling
        model_deinit(flat);
        return -1;
    }

    *out = flat;
    return 0;
}


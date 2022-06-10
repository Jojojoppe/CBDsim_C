#include "sim.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void sim_init(sim_state_t * state){
    // Initialize all d_array_t's
    D_ARRAY_INIT(double, &state->values);
    D_ARRAY_INIT(char*, &state->names);
    D_ARRAY_INIT(d_array_t, &state->arrays);
    D_ARRAY_INIT(cbd_signal_t, &state->cbd_signals);
    D_ARRAY_INIT(cbd_param_t, &state->cbd_params);
    D_ARRAY_INIT(cbd_block_t, &state->cbd_blocks);
}

void sim_deinit(sim_state_t * state){
    // Deinitialize arrays array
    for(int i=0; i<state->arrays.filled_size; i++){
        d_array_deinit(d_array_at(&state->arrays, i));
    }
    d_array_deinit(&state->arrays);
    
    // Deinitialize object arrays
    for(int i=0; i<state->cbd_signals.filled_size; i++){
        cbd_signal_deinit(d_array_at(&state->cbd_signals, i));
    }
    d_array_deinit(&state->cbd_signals);
    for(int i=0; i<state->cbd_params.filled_size; i++){
        cbd_signal_deinit(d_array_at(&state->cbd_params, i));
    }
    d_array_deinit(&state->cbd_params);
    for(int i=0; i<state->cbd_blocks.filled_size; i++){
        cbd_block_deinit(d_array_at(&state->cbd_blocks, i));
    }
    d_array_deinit(&state->cbd_blocks);

    // Deinitialize name array
    for(int i=0; i<state->names.filled_size; i++){
        char * n = *(char**)d_array_at(&state->names, i);
        free(n);
    }
    d_array_deinit(&state->names);

    d_array_deinit(&state->values);
}

int sim_add_name(const char * name, sim_state_t * state){
    // Create string for name
    int nlen = strlen(name);
    char * n = (char*) malloc(nlen+1);
    strcpy(n, name);
    n[nlen] = 0;

    d_array_insert(&state->names, &n);
    return state->names.filled_size-1;
}

int sim_add_value(const double v, sim_state_t * state){
    d_array_insert(&state->values, (void*)&v);
    return state->values.filled_size-1;
}

int sim_add_array(const int * vals, int vals_n, sim_state_t * state){
    d_array_t darr;
    D_ARRAY_INIT(int, &darr);
    for(int i=0; i<vals_n; i++){
        d_array_insert(&darr, (void*)(vals + i));
    }
    d_array_insert(&state->arrays, &darr);
    return state->arrays.filled_size-1;
}

void dbg_sim_printall(sim_state_t * state){
    printf("+ signals:\n");
    for(int i=0; i<state->cbd_signals.filled_size; i++){
        cbd_signal_t * sig = (cbd_signal_t*)d_array_at(&state->cbd_signals, i);
        char * name = *(char**)d_array_at(&state->names, sig->name);
        double value = *(double*)d_array_at(&state->values, sig->value);
        printf("\t- %s \t %f\n", name, value);
    }
    printf("+ params:\n");
    for(int i=0; i<state->cbd_params.filled_size; i++){
        cbd_param_t * sig = (cbd_param_t*)d_array_at(&state->cbd_params, i);
        char * name = *(char**)d_array_at(&state->names, sig->name);
        double value = *(double*)d_array_at(&state->values, sig->value);
        printf("\t- %s \t %f\n", name, value);
    }
}
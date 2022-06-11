#include "sim.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void sim_init(sim_state_t * state, double timestep){
    // Initialize all d_array_t's
    D_ARRAY_INIT(double, &state->values);
    D_ARRAY_INIT(char*, &state->names);
    D_ARRAY_INIT(d_array_t, &state->arrays);
    D_ARRAY_INIT(cbd_signal_t, &state->cbd_signals);
    D_ARRAY_INIT(cbd_param_t, &state->cbd_params);
    D_ARRAY_INIT(cbd_block_t, &state->cbd_blocks);

    // Simulation defaults
    state->time = cbd_signal_add("time", state);
    state->timestep = cbd_param_add("timestep", timestep, state);

    D_ARRAY_INIT(int, &state->eval_order);
    D_ARRAY_INIT(int, &state->watchlist);

    state->plotter = popen("./plot", "w");
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
    d_array_deinit(&state->eval_order);
    d_array_deinit(&state->watchlist);

    fprintf(state->plotter, "X\n");
    pclose(state->plotter);
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

void _sim_evaluate(int i, sim_state_t * state, int * block_evaluated, int * block_evaluated_round, int * sig_from, d_array_t * sig_to){
    cbd_block_t * block = d_array_at(&state->cbd_blocks, i);
    d_array_t * ina = d_array_at(&state->arrays, block->ports_in);
    for(int j=0; j<ina->filled_size; j++){
        int in = *(int*)d_array_at(ina, j);
        int from = sig_from[in];
        if(from<0) continue;
        cbd_block_t * from_block = d_array_at(&state->cbd_blocks, from);

        if(!from_block->depchain_break){
            if(!block_evaluated[from]){
                _sim_evaluate(from, state, block_evaluated, block_evaluated_round, sig_from, sig_to);
                block_evaluated[from] = 1;
                if(block_evaluated_round[from]){
                    printf("ALGEBRAIC LOOP...\n");
                    exit(1);
                }
                block_evaluated_round[from] = 1;
                d_array_insert(&state->eval_order, &from);
            }
        }
    }
}

void sim_compile(sim_state_t * state){

    dbg_sim_printall(state);

    // Map signals to blocks
    int * sig_from = malloc(sizeof(int)*state->cbd_signals.filled_size);
    d_array_t * sig_to = malloc(sizeof(d_array_t)*state->cbd_signals.filled_size);
    for(int i=0; i<state->cbd_signals.filled_size; i++) D_ARRAY_INIT(int, &sig_to[i]);
    // Set time coming from nowhere
    sig_from[state->time] = -1;
    // Loop over blocks to create mapping
    for(int i=0; i<state->cbd_blocks.filled_size; i++){
        cbd_block_t * block = d_array_at(&state->cbd_blocks, i);
        d_array_t * pin = (d_array_t*)d_array_at(&state->arrays, block->ports_in);
        d_array_t * pout = (d_array_t*)d_array_at(&state->arrays, block->ports_out);
        // Set from values
        for(int j=0; j<pout->filled_size; j++){
            int sig = *(int*)d_array_at(pout, j);
            sig_from[sig] = i;
        };
        // Add block to to values
        for(int j=0; j<pin->filled_size; j++){
            int sig = *(int*)d_array_at(pin, j);
            d_array_insert(&sig_to[sig], &i);
        };
    }


    // Find block order
    int * block_evaluated = calloc(state->cbd_blocks.filled_size, sizeof(int));

    // Start from all chain breaking blocks and traverse down
    for(int i=0; i<state->cbd_blocks.filled_size; i++){
        cbd_block_t * block = d_array_at(&state->cbd_blocks, i);
        if(!block->depchain_break) continue;

        int * block_evaluated_round = calloc(state->cbd_blocks.filled_size, sizeof(int));
        _sim_evaluate(i, state, block_evaluated, block_evaluated_round, sig_from, sig_to);
        free(block_evaluated_round);
        d_array_insert(&state->eval_order, &i);
    }

    // TODO check for any non chain breaking blocks
    // Loop over all non-evaluated blocks, traverse to top and evaluate

    free(block_evaluated);

    free(sig_from);
    for(int i=0; i<state->cbd_signals.filled_size; i++) d_array_deinit(&sig_to[i]);
    free(sig_to);
}

void sim_watch_signal(int signal, sim_state_t * state){
    d_array_insert(&state->watchlist, &signal);
}

void sim_plot(const char * options, sim_state_t * state){
    fprintf(state->plotter, "plot %s\n", options);
}

void sim_csv(const char * options, sim_state_t * state){
    fprintf(state->plotter, "csv %s\n", options);
}

void sim_run(double runtime, sim_state_t * state){
    cbd_signal_t * s_time = d_array_at(&state->cbd_signals, state->time);
    double * time = d_array_at(&state->values, s_time->value);
    cbd_param_t * p_timestep = d_array_at(&state->cbd_params, state->timestep);
    double * timestep = d_array_at(&state->values, p_timestep->value);

    // Start plotter and send signal names
    d_array_t watch_values;
    D_ARRAY_INIT(int, &watch_values);
    fprintf(state->plotter, "cols ");
    for(int i=0; i<state->watchlist.filled_size; i++){
        cbd_signal_t * s = d_array_at(&state->cbd_signals, *(int*)d_array_at(&state->watchlist, i));
        d_array_insert(&watch_values, &s->value);
        char * name = *(char**)d_array_at(&state->names, s->name);
        fprintf(state->plotter, "%s ", name);
    }
    fprintf(state->plotter, "\n");

    fprintf(state->plotter, "data\n");
    while(*time<=runtime){

        for(int i=0; i<state->eval_order.filled_size; i++){
            int b_i = *(int*)d_array_at(&state->eval_order, i);
            cbd_block_t * block = d_array_at(&state->cbd_blocks, b_i);
            block->eval(block, state);
        }
        *time += *timestep;

        for(int i=0; i<watch_values.filled_size; i++){
            int vid = *(int*)d_array_at(&watch_values, i);
            double * v = d_array_at(&state->values, vid);
            fprintf(state->plotter, "%g ", *v);
        }
        fprintf(state->plotter, "\n");
    }
    fprintf(state->plotter, "e\n");

    d_array_deinit(&watch_values);
}

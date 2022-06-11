#include "sim.h"
#include "cbd/block.h"

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

    // Create eval function array
    state->cbd_block_eval_functions = calloc(_CBD_BLOCK_FUNCTION_SIZE, sizeof(void **));
    cbd_block_register_eval_functions(state);

    state->plotter = popen("./plot", "w");
    state->_compiled = 0;
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
    free(state->cbd_block_eval_functions);
    state->_compiled = 0;
}

int sim_add_name(const char * name, sim_state_t * state){
    if(state->_compiled==1) return -1;
    // Create string for name
    int nlen = strlen(name);
    char * n = (char*) malloc(nlen+1);
    strcpy(n, name);
    n[nlen] = 0;

    d_array_insert(&state->names, &n);
    return state->names.filled_size-1;
}

int sim_add_value(const double v, sim_state_t * state){
    if(state->_compiled==1) return -1;
    d_array_insert(&state->values, (void*)&v);
    return state->values.filled_size-1;
}

int sim_add_array(const int * vals, int vals_n, sim_state_t * state){
    if(state->_compiled==1) return -1;
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
                    sim_deinit(state);
                    exit(1);
                }
                block_evaluated_round[from] = 1;
                d_array_insert(&state->eval_order, &from);
            }
        }
    }
}

int _sim_findend(int i, sim_state_t * state, int * visited, int * sig_from, d_array_t * sig_to){
    cbd_block_t * block = d_array_at(&state->cbd_blocks, i);
    d_array_t * ina = d_array_at(&state->arrays, block->ports_in);
    d_array_t * outa = d_array_at(&state->arrays, block->ports_out);
    if(visited[i]){
        printf("ALGEBRAIC LOOP...\n");
        sim_deinit(state);
        exit(1);
    }
    visited[i] = 1;
    for(int j=0; j<outa->filled_size; j++){
        int out = *(int*)d_array_at(outa, j);
        d_array_t * toa = &sig_to[out];
        if(toa->filled_size==0){
            // Found end of path
            return i;
        }
        for(int k=0; k<toa->filled_size; k++){
            int toblock = *(int*)d_array_at(toa, k);
            int r = _sim_findend(toblock, state, visited, sig_from, sig_to);
            if(r!=-1) return r;
        }
    }
    return -1;
}

void sim_compile(sim_state_t * state){
    if(state->_compiled) return;

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

    // Loop over all non-evaluated blocks (non-chain breaking / algebraic paths), traverse to top and evaluate
    for(int i=0; i<state->cbd_blocks.filled_size; i++){
        cbd_block_t * block = d_array_at(&state->cbd_blocks, i);
        if(block->depchain_break || block_evaluated[i]) continue;

        int * visited = calloc(state->cbd_blocks.filled_size, sizeof(int));
        int end = _sim_findend(i, state, visited, sig_from, sig_to);
        if(end!=-1){
            int * block_evaluated_round = calloc(state->cbd_blocks.filled_size, sizeof(int));
            _sim_evaluate(end, state, block_evaluated, block_evaluated_round, sig_from, sig_to);
            free(block_evaluated_round);
            d_array_insert(&state->eval_order, &end);
            block_evaluated[end] = 1;
        }
        free(visited);
    }

    free(block_evaluated);

    free(sig_from);
    for(int i=0; i<state->cbd_signals.filled_size; i++) d_array_deinit(&sig_to[i]);
    free(sig_to);

    state->_compiled = 1;
}

void sim_watch_signal(int signal, sim_state_t * state){
    if(!state->_compiled) return;
    d_array_insert(&state->watchlist, &signal);
}

void sim_plot(const char * options, sim_state_t * state){
    if(!state->_compiled) return;
    fprintf(state->plotter, "plot %s\n", options);
}

void sim_csv(const char * options, sim_state_t * state){
    if(!state->_compiled) return;
    fprintf(state->plotter, "csv %s\n", options);
}

void sim_run(double runtime, sim_state_t * state){
    if(!state->_compiled) return;
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

    double starttime = *time;

    fprintf(state->plotter, "data\n");
    while(*time<=runtime+starttime){

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

void sim_serialize(const char * fname, sim_state_t * state){
    if(state->_compiled) return;
    FILE * f = fopen(fname, "w");
    // Write sizes of arrays on one line
    fprintf(f, "%08d ", state->names.filled_size-2);
    fprintf(f, "%08d ", state->values.filled_size-2);
    fprintf(f, "%08d ", state->arrays.filled_size);
    fprintf(f, "%08d ", state->cbd_signals.filled_size-1);
    fprintf(f, "%08d ", state->cbd_params.filled_size-1);
    fprintf(f, "%08d\n", state->cbd_blocks.filled_size);
    fprintf(f, "\n");

    // Write names (one per line) (skip time and timestep)
    // TODO make safe -> encode
    for(int i=2; i<state->names.filled_size; i++){
        char * name = *(char**)d_array_at(&state->names, i);
        fprintf(f, "%s\n", name);
    }
    fprintf(f, "\n");

    // Write values (one per line) (skip time and timestep)
    for(int i=2; i<state->values.filled_size; i++){
        double * value = d_array_at(&state->values, i);
        fprintf(f, "%e\n", *value);
    }
    fprintf(f, "\n");

    // Write arrays (one per line)
    for(int i=0; i<state->arrays.filled_size; i++){
        d_array_t * arr = d_array_at(&state->arrays, i);
        fprintf(f, "%08d ", arr->filled_size);
        for(int j=0; j<arr->filled_size; j++){
            int index = *(int*)d_array_at(arr, j);
            fprintf(f, "%08d ", index);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\n");

    // Write signals (one per line) (skip time)
    for(int i=1; i<state->cbd_signals.filled_size; i++){
        cbd_signal_t * sig = d_array_at(&state->cbd_signals, i);
        fprintf(f, "%08d %08d\n", sig->name, sig->value);
    }
    fprintf(f, "\n");

    // Write params (one per line) (skip timestep)
    for(int i=1; i<state->cbd_params.filled_size; i++){
        cbd_param_t * par = d_array_at(&state->cbd_params, i);
        fprintf(f, "%08d %08d\n", par->name, par->value);
    }
    fprintf(f, "\n");

    // Write blocks (one per line)
    for(int i=0; i<state->cbd_blocks.filled_size; i++){
        cbd_block_t * block = d_array_at(&state->cbd_blocks, i);
        fprintf(f, "%08d %08d %08d %08d %08d %08d\n", block->ports_in, block->ports_out, block->params, block->name, block->depchain_break, block->eval_num);
    }

    fclose(f);
}

void sim_load(const char * fname, double timestep, sim_state_t * state){
    // Reinitialize sim_state
    sim_init(state, timestep);

    FILE * f = fopen(fname, "r");

    int names, values, arrays, signals, parameters, blocks;
    fscanf(f, "%d %d %d %d %d %d\n", &names, &values, &arrays, &signals, &parameters, &blocks);

    // Read names
    for(int i=0; i<names; i++){
        char name[256];
        fscanf(f, "%s\n", name);
        sim_add_name(name, state);
    }

    // Read values
    for(int i=0; i<values; i++){
        float value;
        fscanf(f, "%e\n", &value);
        sim_add_value(value, state);
    }

    // Read arrays
    for(int i=0; i<arrays; i++){
        int arsize;
        fscanf(f, "%d ", &arsize);
        int * array = calloc(arsize, sizeof(int));
        for(int j=0; j<arsize; j++){
            fscanf(f, "%d", array+j);
        }
        sim_add_array(array, arsize, state);
        free(array);
    }

    // Read signals
    for(int i=0; i<signals; i++){
        int name, value;
        fscanf(f, "%d %d\n", &name, &value);
        // Create signal object
        // TODO move to signal file
        cbd_signal_t sig = {
            value,
            name
        };
        d_array_insert(&state->cbd_signals, &sig);
    }

    // Read params
    for(int i=0; i<parameters; i++){
        int name, value;
        fscanf(f, "%d %d\n", &name, &value);
        // Create signal object
        // TODO move to params file
        cbd_param_t par = {
            value,
            name
        };
        d_array_insert(&state->cbd_params, &par);
    }

    // Read blocks
    for(int i=0; i<blocks; i++){
        int pin, pout, params, name, depchain_break, eval_num;
        fscanf(f, "%d %d %d %d %d %d\n", &pin, &pout, &params, &name, &depchain_break, &eval_num);
        // Create block object
        // TODO move to block file
        cbd_block_t block = {
            pin,
            pout,
            params,
            name,
            NULL,
            state->cbd_block_eval_functions[eval_num],
            eval_num,
            depchain_break,
        };
        d_array_insert(&state->cbd_blocks, &block);
    }

    fclose(f);
}

int sim_get_signal(const char * name, sim_state_t * state){
    for(int i=1; i<state->cbd_signals.filled_size; i++){
        cbd_signal_t * sig = d_array_at(&state->cbd_signals, i);
        char * sname = *(char**)d_array_at(&state->names, sig->name);
        if(!strcmp(name, sname)) return i;
    }
    return -1;
}

void sim_viz(sim_state_t * state){
    if(state->_compiled) return;
    FILE * f = popen("dot -Tpng > output.png", "w");

    fprintf(f, "digraph G {\n\tsplines=\"FALSE\";\n\trankdir=LR;\n");
    
    for(int i=0; i<state->cbd_blocks.filled_size; i++){
        cbd_block_t * block = d_array_at(&state->cbd_blocks, i);
        char * name = *(char**)d_array_at(&state->names, block->name);
        if(block->eval_num==STANDARD_PLUSMIN){
            fprintf(f, "\t%s [label=\"+\", shape=\"circle\"]\n", name);
        }else if(block->eval_num==STANDARD_MULDIV){
            fprintf(f, "\t%s [label=\"x\", shape=\"circle\"]\n", name);
        }else{
            fprintf(f, "\t%s [label=\"%s\", shape=\"square\"]\n", name, name);
        }
    }

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

    for(int i=1; i<state->cbd_signals.filled_size; i++){
        cbd_signal_t * sig = d_array_at(&state->cbd_signals, i);
        char * name = *(char**)d_array_at(&state->names, sig->name);
        cbd_block_t * from = d_array_at(&state->cbd_blocks, sig_from[i]);
        char * fromname = *(char**)d_array_at(&state->names, from->name);
        if(sig_to[i].filled_size==0){
            fprintf(f, "\tnowhere%d [style=invis,shape=point]\n", i);
            fprintf(f, "\t%s -> nowhere%d [label=\"%s\"];\n", fromname, i, name);
            continue;
        }
        for(int j=0; j<sig_to[i].filled_size; j++){
            cbd_block_t * to = d_array_at(&state->cbd_blocks, *(int*)d_array_at(&sig_to[i], j));
            char * toname = *(char**)d_array_at(&state->names, to->name);
            fprintf(f, "\t%s -> %s [label=\"%s\"];\n", fromname, toname, name);
        }
    }

    free(sig_from);
    for(int i=0; i<state->cbd_signals.filled_size; i++) d_array_deinit(&sig_to[i]);
    free(sig_to);

    fprintf(f, "}");
    pclose(f);
}

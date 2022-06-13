#include "sim.h"

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

void sim_csv_start(const char * fname, sim_state_t * state){
    if(!state) return; if(!fname) return;
    state->csv = fopen(fname, "w");
    for(int i=0; i<state->model_values(); i++){
        char * name = state->model_value_name(i);
        fprintf(state->csv, "%s ", name);
    }
    fprintf(state->csv, "\n");
}

void sim_csv_end(sim_state_t * state){
    if(!state) return;
    if(state->csv){
        fclose(state->csv);
    }
}

void sim_csv_data_all(sim_state_t * state){
    if(!state) return; if(!state->csv);
    if(state->csv){
        for(int i=0; i<state->model_values(); i++){
            fprintf(state->csv, "%g ", state->values[i]);
        }
        fprintf(state->csv, "\n");
    }
}
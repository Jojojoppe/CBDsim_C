#include "param.h"
#include "../sim.h"

#include <stdlib.h>

int cbd_param_add(const char * name, double val, sim_state_t * state){
    // Add name to the simulation state
    int n_index = sim_add_name(name, state);
    // Add value to the simulation state
    int v_index = sim_add_value(val, state);

    // Create param object
    cbd_param_t sig = {
        v_index,
        n_index
    };
    d_array_insert(&state->cbd_params, &sig);
    return state->cbd_params.filled_size-1;
}

void cbd_param_deinit(cbd_param_t * sig){
}
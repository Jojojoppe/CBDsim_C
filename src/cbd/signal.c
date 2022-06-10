#include "signal.h"
#include "../sim.h"

#include <stdlib.h>

int cbd_signal_add(const char * name, sim_state_t * state){
    // Add name to the simulation state
    int n_index = sim_add_name(name, state);
    // Add value to the simulation state
    int v_index = sim_add_value(0.0, state);

    // Create signal object
    cbd_signal_t sig = {
        v_index,
        n_index
    };
    d_array_insert(&state->cbd_signals, &sig);
    return state->cbd_signals.filled_size-1;
}

void cbd_signal_deinit(cbd_signal_t * sig){
}
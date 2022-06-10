#include "block.h"
#include "../sim.h"

#include <stdlib.h>

int cbd_block_add(const char * name, const int * ports_in, int ports_in_n, const int * ports_out, int ports_out_n, const int * params, int params_n, void (*eval)(cbd_block_t *, sim_state_t *), sim_state_t * state){
    // Add name to the simulation state
    int n_index = sim_add_name(name, state);
    // Add ports to the simulation state
    int pi_index = sim_add_array(ports_in, ports_in_n, state);
    int po_index = sim_add_array(ports_out, ports_out_n, state);
    // Add params
    int pa_index = sim_add_array(params, params_n, state);

    // Create block object
    cbd_block_t block = {
        pi_index,
        po_index,
        pa_index,
        n_index,
        NULL,
        eval
    };
    d_array_insert(&state->cbd_blocks, &block);
    return state->cbd_blocks.filled_size-1;
}

void cbd_block_deinit(cbd_block_t * block){
    if(block->cache){
        free(block->cache);
    }
}
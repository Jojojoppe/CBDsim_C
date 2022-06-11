#include "block.h"
#include "../sim.h"

#include <stdlib.h>

int cbd_block_add(const char * name, const int * ports_in, int ports_in_n, const int * ports_out, int ports_out_n, const int * params, int params_n, int eval_function, int depchain_break, sim_state_t * state){
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
        state->cbd_block_eval_functions[eval_function],
        eval_function,
        depchain_break,
    };
    d_array_insert(&state->cbd_blocks, &block);
    return state->cbd_blocks.filled_size-1;
}

void cbd_block_deinit(cbd_block_t * block){
    if(block->cache){
        free(block->cache);
    }
}

#define ADD_EVAL_FUNC(nr, fun) {void fun(cbd_block_t *, sim_state_t *); state->cbd_block_eval_functions[nr] = &fun;}

void cbd_block_register_eval_functions(sim_state_t * state){
    ADD_EVAL_FUNC(SRC_CONSTANT, eval_constant)
    ADD_EVAL_FUNC(SRC_STEP, eval_step)
    ADD_EVAL_FUNC(SRC_ISTEP, eval_istep)
    ADD_EVAL_FUNC(SRC_PULSE, eval_pulse)
    ADD_EVAL_FUNC(SRC_SIN, eval_sin)
    ADD_EVAL_FUNC(SRC_COS, eval_cos)

    ADD_EVAL_FUNC(STANDARD_GAIN, eval_gain)
    ADD_EVAL_FUNC(STANDARD_PLUSMIN, eval_plusmin)
    ADD_EVAL_FUNC(STANDARD_MULDIV, eval_muldiv)
    ADD_EVAL_FUNC(STANDARD_INT_EULER, eval_int_euler)
    ADD_EVAL_FUNC(STANDARD_INT_TRAP, eval_int_trap)
    ADD_EVAL_FUNC(STANDARD_INT_RK4, eval_int_rk4)
    ADD_EVAL_FUNC(STANDARD_EXP, eval_exp)

}
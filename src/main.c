#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "sim.h"
#include "cbd/blocks/src/src.h"
#include "cbd/blocks/standard/standard.h"

int main(int argc, char ** argv){

    sim_state_t state;
    sim_init(&state, 0.01);

    // Define Block Diagram
    // --------------------

    // Signals
    int signal_e1 = cbd_signal_add("e1", &state);
    int signal_e2 = cbd_signal_add("e2", &state);

    // Paramters
    int param_sin_A = cbd_param_add("sin_A", 1.0, &state);
    int param_sin_f = cbd_param_add("sin_f", 1.0, &state);
    int param_int1_init = cbd_param_add("int1_init", 0.0, &state);

    // Blocks
    int block_src_sin = cbd_blocks_src_sin(signal_e1, param_sin_A, param_sin_f, "src_sin", &state);
    int block_int1 = cbd_block_standard_int_trap(signal_e1, signal_e2, param_int1_init, "int1", &state);

    // --------------------

    sim_compile(&state);

    sim_watch_signal(signal_e1, &state);
    sim_run(10.0, &state);
    
    sim_deinit(&state);

    return 0;
}
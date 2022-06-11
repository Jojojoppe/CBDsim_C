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
    int s_in = cbd_signal_add("in", &state);
    int s_out = cbd_signal_add("out", &state);

    int p_sin_A = cbd_param_add("sin_A", 1.0, &state);
    int p_sin_f = cbd_param_add("sin_f", 2.0, &state);
    int p_K1_K = cbd_param_add("K1_K", -3.0, &state);

    int b_sin = cbd_blocks_src_sin(s_in, p_sin_A, p_sin_f, "sin", &state);
    int b_K1 = cbd_block_standard_gain(s_in, s_out, p_K1_K, "K1", &state);
    // --------------------

    sim_viz(&state);
    sim_serialize("models/stateless_path.model", &state);
    sim_compile(&state);

    sim_watch_signal(state.time, &state);
    sim_watch_signal(s_in, &state);
    sim_watch_signal(s_out, &state);
    sim_run(5.0, &state);

    sim_plot("1,1 x:time y:in y:out p", &state);

    sim_deinit(&state);
    return 0;
}
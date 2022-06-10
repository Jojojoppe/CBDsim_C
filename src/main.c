#include <stdio.h>
#include <math.h>

#include "sim.h"
#include "cbd/blocks/src/src.h"

int main(int argc, char ** argv){

    sim_state_t state;
    sim_init(&state);

    // Signals
    int signal_f = cbd_signal_add("f", &state);

    // Paramters
    int param_sin_A = cbd_param_add("sin_A", 1.0, &state);
    int param_sin_f = cbd_param_add("sin_f", 1.0, &state);

    // Simulation defaults
    int signal_time = cbd_signal_add("time", &state);
    int param_timestep = cbd_param_add("timestep", 0.01, &state);

    // Blocks
    int block_src_sin = cbd_blocks_src_sin(signal_time, signal_f, param_sin_A, param_sin_f, "src_sin", &state);

    fputs("\033c", stdout);
    dbg_sim_printall(&state);

    sim_deinit(&state);

    return 0;
}
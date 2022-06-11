#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "sim.h"
#include "cbd/blocks/src/src.h"
#include "cbd/blocks/standard/standard.h"

#define SIGNAL(name, state) int s_##name = cbd_signal_add(#name, state);
#define PARAM(name, value, state) int p_##name = cbd_param_add(#name, value, state);

int main(int argc, char ** argv){

    sim_state_t state;
    sim_init(&state, 0.01);

    SIGNAL(ein, &state)
    SIGNAL(e2, &state)
    SIGNAL(e3, &state)
    SIGNAL(e4, &state)
    SIGNAL(f, &state)
    SIGNAL(p2, &state)
    SIGNAL(q3, &state)

    PARAM(K_I, 1.0, &state)
    PARAM(K_C, 1.0, &state)
    PARAM(K_R, 0.5, &state)
    PARAM(int_I_init, 0.0, &state)
    PARAM(int_C_init, 0.0, &state)

    int b_int_I = cbd_block_standard_int_euler(s_e2, s_p2, p_int_I_init, "int_I", &state);
    int b_int_C = cbd_block_standard_int_euler(s_f, s_q3, p_int_C_init, "int_C", &state);
    int b_K_I = cbd_block_standard_gain(s_p2, s_f, p_K_I, "K_I", &state);
    int b_K_C = cbd_block_standard_gain(s_q3, s_e3, p_K_C, "K_C", &state);
    int b_K_R = cbd_block_standard_gain(s_f, s_e4, p_K_R, "K_R", &state);

    int b_pm1; {
        const int pin[1] = {s_ein};
        const int min[2] = {s_e3, s_e4};
        b_pm1 = cbd_block_standard_plusmin(pin, 1, min, 2, s_e2, "pm1", &state);
    }

    PARAM(inp_A, 1.0, &state)
    PARAM(inp_t0, 0.5, &state)
    int b_inp = cbd_blocks_src_step(s_ein, p_inp_A, p_inp_t0, "inp", &state);

    // sim_viz(&state);
    sim_serialize("models/RLC.model", &state);

    sim_compile(&state);

    sim_watch_signal(state.time, &state);
    sim_watch_signal(s_ein, &state);
    sim_watch_signal(s_e2, &state);
    sim_watch_signal(s_e3, &state);
    sim_watch_signal(s_e4, &state);
    sim_watch_signal(s_f, &state);

    sim_run(40.0, &state);

    sim_plot("2,1 x:time y:ein y:e2 y:e3 y:e4 p x:time y:f p", &state);
    // sim_csv("rlc_euler.csv time f", &state);

    sim_deinit(&state);
    return 0;
}
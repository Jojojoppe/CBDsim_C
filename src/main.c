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
    int signal_e3 = cbd_signal_add("e3", &state);
    int signal_q2 = cbd_signal_add("q2", &state);
    int signal_f = cbd_signal_add("f", &state);

    // Paramters
    int param_pulse_A = cbd_param_add("pulse_A", 1.0, &state);
    int param_pulse_t0 = cbd_param_add("pulse_t0", 1.0, &state);
    int param_pulse_t1 = cbd_param_add("pulse_t1", 11.0, &state);
    int param_C_K = cbd_param_add("C_K", 1/2.0, &state);
    int param_R_K = cbd_param_add("R_K", 1.0, &state);
    int param_intC_init = cbd_param_add("intC_init", 0.0, &state);

    // Blocks
    int block_pulse = cbd_blocks_src_pulse(signal_e1, param_pulse_A, param_pulse_t0, param_pulse_t1, "step", &state);
    int block_gainC = cbd_block_standard_gain(signal_q2, signal_e2, param_C_K, "gainC", &state);
    int block_gainR = cbd_block_standard_gain(signal_e3, signal_f, param_R_K, "gainR", &state);
    int block_intC = cbd_block_standard_int_trap(signal_f, signal_q2, param_intC_init, "intC", &state);
    int block_add1; {
        const int pinp[1] = {signal_e1};
        const int pinm[1] = {signal_e2};
        block_add1 = cbd_block_standard_plusmin(pinp, 1, pinm, 1, signal_e3, "add1", &state);
    }
    // --------------------

    sim_compile(&state);
    // sim_serialize("test.model", &state);
    sim_viz(&state);

    sim_watch_signal(state.time, &state);
    sim_watch_signal(signal_e1, &state);
    sim_watch_signal(signal_e2, &state);
    sim_watch_signal(signal_e3, &state);
    sim_watch_signal(signal_f, &state);
    
    sim_run(20.0, &state);

    sim_plot("2,1 x:time y:e1:label:input y:e2:label:U_C y:e3:label:U_R p x:time y:f:label:I p", &state);
    // sim_csv("out/test.csv time e1 e2 e3 f", &state);

    sim_deinit(&state);
    return 0;
}
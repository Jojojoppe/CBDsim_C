#include <stdio.h>

#include "model/model.h"
#include "model/blocks/blocks.h"
#include "sim/sim.h"
#include "sim/solvers/euler.h"

int main(int argc, char ** argv){
    int error = 0;

    // Generate model
    model_t * model = model_init();

    SIGNAL(e1, model);
    SIGNAL(e2, model);
    SIGNAL(e3, model);
    SIGNAL(e4, model);
    SIGNAL(f, model);
    SIGNAL(q3, model);
    SIGNAL(p2, model);

    blocks_add_src_step(1, 10, "src", s_e1, model);
    blocks_add_std_integrate(0.1, "iC", s_f, s_q3, model);
    blocks_add_std_integrate(0.0, "iI", s_e2, s_p2, model);
    blocks_add_std_attenuate(0.1, "KC", s_q3, s_e3, model);
    blocks_add_std_attenuate(0.1, "KI", s_p2, s_f, model);
    blocks_add_std_gain(0.1, "KR", s_f, s_e4, model);
    blocks_add_std_plusmin("pm", (int[]){s_e1}, 1, (int[]){s_e3, s_e4}, 2, s_e2, model);

    SIGNAL(q0, model);
    blocks_add_src_constant(0, "zero", s_q0, model);

    if((error = model_compile("TM.c", model))) goto end;
    // Load model and run simulation
    solver_euler_params_t sparams = {.timestep=0.01};
    sim_state_t * state = sim_init(&solver_euler, &sparams, "./plot");

    if((error = sim_compile_model("TM.c", "model.so", state))) goto end;
    if((error = sim_load_model("model.so", state))) goto end;

    sim_init_run(state);
    sim_plot_window("w", NULL, state);
    sim_plot("w", "pe", 221, NULL, state, "legend", 4, "time e1", "time e2", "time e3", "time e4");
    sim_plot("w", "pf", 223, NULL, state, "legend", 1, "time f");
    sim_plot("w", "pos", 222, NULL, state, "", 1, "q0 q3 style:None");

    sim_run_realtime(20.0, 30, 1, state);

end:
    model_deinit(model);
    sim_deinit(state);
    return error;
}
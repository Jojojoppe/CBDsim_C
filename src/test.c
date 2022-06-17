#include <stdio.h>

#include "sim/sim.h"
#include "sim/solvers/euler.h"
#include "sim/solvers/rk4.h"

int main(int argc, char ** argv){
    int error = 0;

    // Load model and run simulation
    sim_state_t * state = sim_init();
    if((error = sim_compile_model("TM.c", "model.so", state))) goto end;
    if((error = sim_load_model("model.so", state))) goto end;

    solver_euler_params_t sparams_euler = {.timestep=0.01};
    solver_rk4_params_t sparams_rk4 = {.timestep=0.01};
    sim_discrete_settings_t dsettings[2] = {
        (sim_discrete_settings_t){
            .sampling_time = 0.1,
        },
        (sim_discrete_settings_t){
            .sampling_time = 1.0,
        },
    };
    sim_run_settings_t runsettings = {
        .viz = "./plot",
        .solver = &solver_euler,
        .solver_settings = &sparams_euler,
        .discrete_settings = dsettings,
        .nr_discrete_settings = 2,
    };
    sim_init_run(&runsettings, state);

    sim_plot_window("w", NULL, state);
    sim_plot("w", "pe", 221, NULL, state, "legend", 4, "time e1", "time e2", "time e3", "time e4");
    sim_plot("w", "pf", 223, NULL, state, "legend", 1, "time f");
    sim_plot("w", "pos", 222, NULL, state, "", 1, "q0 q3 style:None");
    sim_plot("w", "dis", 224, NULL, state, "", 3, "time testval", "time testval2");

    // sim_run_realtime(20.0, 30, 1.5, state);
    sim_run(20, state);

end:
    sim_deinit(state);
    return error;
}
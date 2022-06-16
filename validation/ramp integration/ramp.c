#include <stdio.h>

#include "model/model.h"
#include "model/blocks/blocks.h"
#include "sim/sim.h"
#include "sim/solvers/euler.h"
#include "sim/solvers/rk4.h"

/* TODO list:
 *  - Fix RK4 method. See google sheets file with graph: somehow damping is different with RK4
 */

int main(int argc, char ** argv){
    int error = 0;

    // Generate model
    model_t * model = model_init();

    SIGNAL(s, model);
    SIGNAL(out, model);
    blocks_add_src_sine(1, 1, "src", s_s, model);
    blocks_add_std_integrate(0, "int", s_s, s_out, model);

    if((error = model_compile("TM.c", model))) goto end;
    // Load model and run simulation
    solver_euler_params_t sparams_euler = {.timestep=0.01};
    solver_rk4_params_t sparams_rk4 = {.timestep=0.01};
    // sim_state_t * state = sim_init(&solver_euler, &sparams_euler, "./plot");
    sim_state_t * state = sim_init(&solver_rk4, &sparams_rk4, "./plot");

    if((error = sim_compile_model("TM.c", "model.so", state))) goto end;
    if((error = sim_load_model("model.so", state))) goto end;

    sim_init_run(state);
    sim_plot_window("w", NULL, state);
    sim_plot("w", "p1", 111, NULL, state, "", 1, "time out");

    // sim_csv_start("validation/euler_ramp.csv", state);
    // sim_csv_start("validation/rk4_ramp.csv", state);

    // sim_run_realtime(20.0, 30, 1, state);
    sim_run(10, state);

end:
    model_deinit(model);
    sim_deinit(state);
    return error;
}
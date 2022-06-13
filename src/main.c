#include <stdio.h>
#include <stdlib.h>

#include "sim/solvers/euler.h"
#include "sim/solver.h"
#include "sim/sim.h"

int run_sim(){
    solver_euler_params_t sparams = {.timestep=0.01};
    sim_state_t * state = sim_init(&solver_euler, &sparams, "./plot");

    // Compile model
    if(sim_compile_model("TM.c", "model.so", state)){
        printf("Could not compile model\n");
        sim_deinit(state);
        return 1;
    }
    // Load model
    int e;
    if((e = sim_load_model("model.so", state))){
        printf("Could not load model: %d\n", e);
        sim_deinit(state);
        return 1;
    }

    sim_init_run(state);

    sim_plot_window("w0", "Some__Title", state);
    sim_plot("w0", "p0", 121, "Some__Plot", state,
        "xlabel:time legend",
        2,
        "time s1:src",
        "time s2:out"
    );
    sim_plot("w0", "p1", 122, "Some__other__Plot", state,
        "xlabel:src ylabel:out",
        1,
        "s1 s2"
    );
    // sim_csv_start("data.csv", state);

    // sim_run_realtime(5.0, 30, 1, state);
    sim_run(5.0, state);

    sim_deinit(state);
    return 0;
}

#include "model/model.h"
#include "model/blocks/blocks.h"
#include "dynamic_array.h"

int main(int argc, char ** argv){

    model_t * model = model_init();

    int s_1 = model_add_signal("s1", model);
    int s_2 = model_add_signal("s2", model);
    int b_src = blocks_add_src_sine(1.0, 1.0, "src", s_1, model);
    int b_tst = blocks_add_std_exp("tst", s_1, s_2, model);    

    if(model_compile("TM.c", model)) return -1;

    model_deinit(model);

    if(run_sim()) return -1;

    return 0;
}
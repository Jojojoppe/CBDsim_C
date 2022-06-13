#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

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

    for(int i=0; i<state->model_values(); i++){
        printf("%s\t= %f\n", state->model_value_name(i), state->values[i]);
    }

    sim_window("w0", "Some__Title", state);
    sim_plot("w0", "p0", 121, "Some__Plot", state,
        "xlabel:time legend",
        2,
        "time in1:sin",
        "time in2:cos"
    );
    sim_plot("w0", "p1", 122, "Some__other__Plot", state,
        "",
        1,
        "in1 in2"
    );

    sim_run_realtime(20.0, 30, 1, state);

    sim_deinit(state);
    return 0;
}

#include "model/model.h"
#include "model/blocks/blocks.h"
#include "dynamic_array.h"

int main(int argc, char ** argv){

    model_t * model = model_init();

    int s_in1 = model_add_signal("in1", model);
    int s_in2 = model_add_signal("in2", model);
    int b_sine = blocks_add_src_sine(1.0, 1.0, "sin", s_in1, model);
    int b_cosine = blocks_add_src_cosine(1.0, 1.0, "cos", s_in2, model);

    model_compile("TM.c", model);

    model_deinit(model);

    printf("Running:\n");
    int r = run_sim();
    return 0;
}
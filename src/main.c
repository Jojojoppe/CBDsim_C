#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "solvers/euler.h"
#include "solver.h"
#include "sim.h"

int main(int argc, char ** argv){
    solver_euler_params_t sparams = {.timestep=0.01};
    sim_state_t * state = sim_init(&solver_euler, &sparams);

    // Compile model
    if(sim_compile_model("TESTMODEL.c", "model.so", state)){
        printf("Could not compile model\n");
        sim_deinit(state);
        return 1;
    }
    // Load model
    if(sim_load_model("model.so", state)){
        printf("Could not load model\n");
        sim_deinit(state);
        return 1;
    }

    sim_init_run(state);

    for(int i=0; i<state->model_values(); i++){
        printf("%s\t= %f\n", state->model_value_name(i), state->values[i]);
    }

    sim_run(10.0, state);
    sim_plot("1,1 x:time y:f p", state);

    sim_run(10.0, state);
    sim_plot("1,1 x:time y:f p", state);

    sim_init_run(state);
    sim_run(10.0, state);
    sim_plot("1,1 x:time y:f p", state);

    sim_deinit(state);
    return 0;
}
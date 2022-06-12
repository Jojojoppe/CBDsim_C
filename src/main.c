#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "sim/solvers/euler.h"
#include "sim/solver.h"
#include "sim/sim.h"

int run_sim(){
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

    sim_deinit(state);
    return 0;
}

#include "model/model.h"
#include "model/blocks/blocks.h"
#include "dynamic_array.h"

int main(int argc, char ** argv){

    model_t * model = model_init();

    int s_in = model_add_signal("in", model);
    int b_sine = blocks_add_src_cosine(1.0, 2.0, "sin", s_in, model);

    printf("signals:\n");
    for(int i=0; i<D_ARRAY_LEN(model->signals); i++){
        char * n = (D_ARRAY_ATV(signal_t*, &model->signals, i))->name;
        printf("\t- %s\n", n);
    }
    printf("params:\n");
    for(int i=0; i<D_ARRAY_LEN(model->params); i++){
        char * n = (D_ARRAY_ATV(param_t*, &model->params, i))->name;
        double v = (D_ARRAY_ATV(param_t*, &model->params, i))->value;
        printf("\t- %s = %f\n", n, v);
    }
    printf("blocks:\n");
    for(int i=0; i<D_ARRAY_LEN(model->blocks); i++){
        char * n = (D_ARRAY_ATV(block_t*, &model->blocks, i))->name;
        void (*generate)(FILE * f, block_t * block) = (D_ARRAY_ATV(block_t*, &model->blocks, i))->generate;
        printf("\t- %s\n", n);
        generate(stdout, D_ARRAY_ATV(block_t*, &model->blocks, i));
    }

    model_deinit(model);

    // int r = run_sim();
    return 0;
}
#include <stdio.h>

#include "sim/sim.h"
#include "sim/solvers/euler.h"
#include "sim/solvers/rk4.h"

#include "model/model.h"

const block_definition_t src_sine = {
    .type = "src_sine",
    .genparams_size = 0,
    .gen = 
        "out = A*sin(2*M_PI*f*time);",
    .gen_init = 
        "",
    .names = {
        // Inputs
        0,
        // Outputs
        "out", 0,
        // Parameters
        "A", "f", 0,
        // Variables,
        0,
    },
};

const block_definition_t std_gain = {
    .type = "std_gain",
    .genparams_size = 0,
    .gen = 
        "out = K*in;",
    .gen_init = 
        "",
    .names = {
        // Inputs
        "in", 0,
        // Outputs
        "out", 0,
        // Parameters
        "K", 0,
        // Variables,
        0,
    },
};

int main(int argc, char ** argv){
    int error = 0;

    model_t * toplevel = model_init("toplevel");

    model_t * modelA;
    model_add_submodel("modelA", &modelA, toplevel);
    model_add_inout("O", MODEL_OUTPUT, MODEL_INOUT_NOARRAY, modelA);
    model_add_block("src", &src_sine, (double[]){1.0, 0.5}, modelA);
    model_add_block("gain", &std_gain, (double[]){2.5}, modelA);
    model_connect_signals("src/out", "gain/in", modelA);
    model_connect_signals("gain/out", "O", modelA);

    model_update(toplevel);

    model_add_block("gain", &std_gain, (double[]){2.0}, toplevel);
    model_connect_signals("modelA/O", "gain/in", toplevel);

    model_debug(toplevel);
    model_export(toplevel, "testout.model");

    model_deinit(toplevel);

    return 0;
}

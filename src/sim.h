#ifndef __H_SIM
#define __H_SIM

#include "dynamic_array.h"
#include "cbd/signal.h"
#include "cbd/param.h"
#include "cbd/block.h"

typedef struct sim_state_s {
    // Layout
    // ------
    d_array_t names;            // char*[]
    d_array_t values;           // double[]
    d_array_t arrays;           // d_array_t(int)[]

    d_array_t cbd_signals;      // cbd_signal_t[]
    d_array_t cbd_params;       // cbd_param_t[]
    d_array_t cbd_blocks;       // cbd_blocks_t[]
    // ------

    int time, timestep;

    d_array_t eval_order;

} sim_state_t;

void sim_init(sim_state_t * state, double timestep);
void sim_deinit(sim_state_t * state);

int sim_add_name(const char * name, sim_state_t * state);
int sim_add_value(const double v, sim_state_t * state);
int sim_add_array(const int * vals, int vals_n, sim_state_t * state);

void sim_compile(sim_state_t * state);

void dbg_sim_printall(sim_state_t * state);

#endif
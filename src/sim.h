#ifndef __H_SIM
#define __H_SIM

#include "dynamic_array.h"
#include "cbd/signal.h"
#include "cbd/param.h"
#include "cbd/block.h"

#include <stdio.h>

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
    d_array_t watchlist;

    FILE * plotter;

    void (**cbd_block_eval_functions)(cbd_block_t * block, struct sim_state_s * state);
    int _initialized;
} sim_state_t;

void sim_init(sim_state_t * state, double timestep);
void sim_deinit(sim_state_t * state);

int sim_add_name(const char * name, sim_state_t * state);
int sim_add_value(const double v, sim_state_t * state);
int sim_add_array(const int * vals, int vals_n, sim_state_t * state);

void sim_compile(sim_state_t * state);

void sim_watch_signal(int signal, sim_state_t * state);
void sim_plot(const char * options, sim_state_t * state);
void sim_csv(const char * options, sim_state_t * state);

void sim_run(double runtime, sim_state_t * state);

void sim_serialize(const char * fname, sim_state_t * state);
void sim_load(const char * fname, double timestep, sim_state_t * state);

int sim_get_signal(const char * name, sim_state_t * state);

void dbg_sim_printall(sim_state_t * state);

#endif
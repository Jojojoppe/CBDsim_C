#ifndef __H_SIM
#define __H_SIM

#include "solver.h"
#include <stdio.h>
#include <stdarg.h>

typedef struct sim_state_s {
    // Solver information
    const solver_t * solver;
    void * solver_state;

    // Model information
    void * model;
    int (*model_values)();
    char * (*model_value_name)(int);
    double (*model_value_init)(int);
    int (*model_disdomains)();
    double (*model_disdomain_ts)(int);
    void (*model_init)(double*);
    void (*model_step)(double*,int,int,int,double,double,solver_integrate,void*);

    // Run information
    double * values;
    double time, timestep;
    int disdomains;
    double * sample_at;
    int major, minor;

    // Logger stuff
    const char * viz_type;
    FILE * viz;
    FILE * csv;
} sim_state_t;

sim_state_t * sim_init();
void sim_deinit(sim_state_t * state);

int sim_compile_model(const char * modelcfile, const char * modelfile, sim_state_t * state);
int sim_load_model(const char * modelfile, sim_state_t * state);

typedef struct{
    // Visualizer settings
    const char * viz;
    // Solver settings
    const solver_t * solver;
    void * solver_settings;
} sim_run_settings_t;

void sim_init_run(sim_run_settings_t * settings, sim_state_t * state);

void sim_run(double runtime, sim_state_t * state);
void sim_run_realtime(double runtime, double updatef, double speed, sim_state_t * state);

void sim_plot_window(const char * name, const char * title, sim_state_t * state);
void sim_plot(const char * wname, const char * pname, int loc, const char * title, sim_state_t * state, const char * options, int lines, ...);
void sim_plot_update(sim_state_t * state);
void sim_plot_data_all(sim_state_t * state);
void sim_plot_reset(sim_state_t * state);

void sim_csv_start(const char * fname, sim_state_t * state);
void sim_csv_end(sim_state_t * state);
void sim_csv_data_all(sim_state_t * state);

#endif
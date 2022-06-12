#ifndef __H_SIM
#define __H_SIM

#include "solver.h"

typedef struct sim_state_s {
    // Solver information
    solver_t * solver;
    void * solver_state;

    // Model information
    void * model;
    int (*model_values)();
    char * (*model_value_name)(int);
    double (*model_value_init)(int);
    void (*model_init)(double*);
    double (*model_step)(double*,int,int,double,double,solver_integrate,void*);

    // Run information
    double * values;
    double time, timestep;
    int major, minor;
} sim_state_t;

sim_state_t * sim_init(solver_t * solver, void * solver_params);
void sim_deinit(sim_state_t * state);

int sim_compile_model(const char * modelcfile, const char * modelfile, sim_state_t * state);
int sim_load_model(const char * modelfile, sim_state_t * state);

void sim_init_run(sim_state_t * state);
void sim_step(sim_state_t * state);

#endif
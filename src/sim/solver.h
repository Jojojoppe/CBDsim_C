#ifndef __H_SOLVER
#define __H_SOLVER

typedef struct {
    double timestep;
    int major, minor;
} solver_step_end_retval_t;

typedef void * (*solver_init)(void * params);
typedef void (*solver_deinit)(void * state);
typedef void (*solver_reset)(void * state);
typedef void (*solver_start_step)(void * state);
typedef solver_step_end_retval_t (*solver_end_step)(void * state);
typedef double (*solver_integrate)(double input, double initial, int nr, void * state);

typedef struct solver_s{
    solver_init init;
    solver_deinit deinit;
    solver_reset reset;
    solver_start_step start_step;
    solver_end_step end_step;
    solver_integrate integrate;
} solver_t; 

#endif
#ifndef __H_SOLVERS_EULER
#define __H_SOLVERS_EULER

#include "../solver.h"
#include "../dynamic_array.h"

const extern solver_t solver_euler;

typedef struct{
    double timestep;
} solver_euler_params_t;

typedef struct{
    double timestep;
    // Internal states of integrators
    d_array_t states;
} solver_euler_state_t;

#endif
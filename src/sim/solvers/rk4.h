#ifndef __H_SOLVERS_RK4
#define __H_SOLVERS_RK4

#include "../solver.h"
#include "../../general/dynamic_array.h"

const extern solver_t solver_rk4;

typedef struct{
    double timestep;
} solver_rk4_params_t;

typedef struct{
    double timestep;
    int passthrough;
    // Step in RK4 (1 major 3 minor)
    int step;
    // Internal states of integrators
    d_array_t states;
    d_array_t prev1;
    d_array_t prev2;
    d_array_t prev3;    
    // Internal states of differentiators
    d_array_t dstates;
    d_array_t dstates2;
} solver_rk4_state_t;

#endif
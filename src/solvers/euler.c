#include "euler.h"

#include <stdlib.h>

void * solver_euler_init(void * _params){
    solver_euler_params_t * params = (solver_euler_params_t*) _params;
    solver_euler_state_t * state = (solver_euler_state_t*) calloc(1, sizeof(solver_euler_state_t));
    D_ARRAY_INIT(double, &state->states);
    state->timestep = params->timestep;
    return (void*)state;
}

void solver_euler_deinit(void * _state){
    solver_euler_state_t * state = (solver_euler_state_t*) _state;
    d_array_deinit(&state->states);
    free(state);
}

void solver_euler_start_step(void * _state){
    solver_euler_state_t * state = (solver_euler_state_t*) _state;
}

solver_step_end_retval_t solver_euler_stop_step(void * _state){
    solver_euler_state_t * state = (solver_euler_state_t*) _state;
    return (solver_step_end_retval_t){state->timestep, 1, 0};
}

double solver_euler_integrate(double input, double initial, int nr, void * _state){
    solver_euler_state_t * state = (solver_euler_state_t*) _state;
    if(D_ARRAY_LEN(state->states)<=nr) d_array_insert(&state->states, &initial);
    double * states = D_ARRAY_DP(double, state->states);
    states[nr] = states[nr] + state->timestep*input;
    return states[nr];
}

const solver_t solver_euler = {
    solver_euler_init,
    solver_euler_deinit,
    solver_euler_start_step,
    solver_euler_stop_step,
    solver_euler_integrate,
};
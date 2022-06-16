#include "euler.h"

#include <stdlib.h>

void * solver_euler_init(void * _params){
    solver_euler_params_t * params = (solver_euler_params_t*) _params;
    solver_euler_state_t * state = (solver_euler_state_t*) calloc(1, sizeof(solver_euler_state_t));
    D_ARRAY_INIT(double, &state->states);
    D_ARRAY_INIT(double, &state->dstates);
    D_ARRAY_INIT(double, &state->dstates2);
    state->timestep = params->timestep;
    return (void*)state;
}

void solver_euler_deinit(void * _state){
    solver_euler_state_t * state = (solver_euler_state_t*) _state;
    d_array_deinit(&state->states);
    d_array_deinit(&state->dstates);
    d_array_deinit(&state->dstates2);
    free(state);
}

void solver_euler_reset(void * _state){
    solver_euler_state_t * state = (solver_euler_state_t*) _state;
    for(int i=0; i<D_ARRAY_LEN(state->states); i++){
        (D_ARRAY_DP(double, state->states))[i] = 0.0;
    }
    for(int i=0; i<D_ARRAY_LEN(state->dstates); i++){
        (D_ARRAY_DP(double, state->dstates))[i] = 0.0;
    }
    for(int i=0; i<D_ARRAY_LEN(state->dstates2); i++){
        (D_ARRAY_DP(double, state->dstates2))[i] = 0.0;
    }
}

void solver_euler_start_step(void * _state, int passthrough){
    solver_euler_state_t * state = (solver_euler_state_t*) _state;
    state->passthrough = passthrough;
}

solver_step_end_retval_t solver_euler_stop_step(void * _state){
    solver_euler_state_t * state = (solver_euler_state_t*) _state;
    return (solver_step_end_retval_t){state->timestep, 1, 0};
}

double solver_euler_integrate(double input, double initial, int nr, void * _state){
    solver_euler_state_t * state = (solver_euler_state_t*) _state;
    if(D_ARRAY_LEN(state->states)<=nr) d_array_insert(&state->states, &initial);
    double * states = D_ARRAY_DP(double, state->states);
    // Check for passthrough
    if(state->passthrough) return states[nr];
    states[nr] = states[nr] + state->timestep*input;
    return states[nr];
}

double solver_euler_differentiate(double input, double initial, int nr, void * _state){    
    solver_euler_state_t * state = (solver_euler_state_t*) _state;
    if(D_ARRAY_LEN(state->dstates)<=nr) d_array_insert(&state->dstates, &input);
    double * dstates = D_ARRAY_DP(double, state->dstates);
    double * dstates2 = D_ARRAY_DP(double, state->dstates2);
    // Check for passthrough
    if(state->passthrough) return dstates2[nr];
    dstates2[nr] = (input-dstates[nr])/state->timestep;
    dstates[nr] = input;
    return dstates2[nr];
}

const solver_t solver_euler = {
    solver_euler_init,
    solver_euler_deinit,
    solver_euler_reset,
    solver_euler_start_step,
    solver_euler_stop_step,
    solver_euler_integrate,
    solver_euler_differentiate,
};
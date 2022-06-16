#include "rk4.h"

#include <stdlib.h>

void * solver_rk4_init(void * _params){
    solver_rk4_params_t * params = (solver_rk4_params_t*) _params;
    solver_rk4_state_t * state = (solver_rk4_state_t*) calloc(1, sizeof(solver_rk4_state_t));
    D_ARRAY_INIT(double, &state->states);
    D_ARRAY_INIT(double, &state->prev1);
    D_ARRAY_INIT(double, &state->prev2);
    D_ARRAY_INIT(double, &state->prev3);
    state->timestep = params->timestep/4.0;
    state->step = 3; // Begin on step 0, major step
    return (void*)state;
}

void solver_rk4_deinit(void * _state){
    solver_rk4_state_t * state = (solver_rk4_state_t*) _state;
    d_array_deinit(&state->states);
    d_array_deinit(&state->prev1);
    d_array_deinit(&state->prev2);
    d_array_deinit(&state->prev3);
    free(state);
}

void solver_rk4_reset(void * _state){
    solver_rk4_state_t * state = (solver_rk4_state_t*) _state;
    for(int i=0; i<D_ARRAY_LEN(state->states); i++) (D_ARRAY_DP(double, state->states))[i] = 0.0;
    for(int i=0; i<D_ARRAY_LEN(state->prev1); i++) (D_ARRAY_DP(double, state->prev1))[i] = 0.0;
    for(int i=0; i<D_ARRAY_LEN(state->prev2); i++) (D_ARRAY_DP(double, state->prev2))[i] = 0.0;
    for(int i=0; i<D_ARRAY_LEN(state->prev3); i++) (D_ARRAY_DP(double, state->prev2))[i] = 0.0;
    state->step = 3;
}

void solver_rk4_start_step(void * _state, int passthrough){
    solver_rk4_state_t * state = (solver_rk4_state_t*) _state;
    state->passthrough = passthrough;
}

solver_step_end_retval_t solver_rk4_stop_step(void * _state){
    solver_rk4_state_t * state = (solver_rk4_state_t*) _state;
    switch(state->step){
        // Major step
        case 0:
            if(!state->passthrough) state->step = 1;
            return (solver_step_end_retval_t){state->timestep, 0, 1};
        // Minor steps
        case 1:
            if(!state->passthrough) state->step = 2;
            return (solver_step_end_retval_t){state->timestep, 0, 1};
        case 2:
            if(!state->passthrough) state->step = 3;
            return (solver_step_end_retval_t){state->timestep, 0, 1};
        case 3:
            if(!state->passthrough) state->step = 0;
            return (solver_step_end_retval_t){state->timestep, 1, 0};
        // Error... return to Major step
        default:
            if(!state->passthrough) state->step = 0;
            return (solver_step_end_retval_t){state->timestep, 1, 0};
    }
}

double solver_rk4_integrate(double input, double initial, int nr, void * _state){
    solver_rk4_state_t * state = (solver_rk4_state_t*) _state;
    if(D_ARRAY_LEN(state->states)<=nr){
        d_array_insert(&state->states, &initial);
        double z = 0;
        d_array_insert(&state->prev1, &z);
        d_array_insert(&state->prev2, &z);
        d_array_insert(&state->prev3, &z);
    }
    double * states = D_ARRAY_DP(double, state->states);
    double * prev1 = D_ARRAY_DP(double, state->prev1);
    double * prev2 = D_ARRAY_DP(double, state->prev2);
    double * prev3 = D_ARRAY_DP(double, state->prev3);
    // Check for passthrough
    if(state->passthrough) return states[nr];
    states[nr] = states[nr] + state->timestep/3*(prev3[nr]/2 + prev2[nr] + prev1[nr] + input/2);
    prev3[nr] = prev2[nr];
    prev2[nr] = prev1[nr];
    prev1[nr] = input;
    return states[nr];
}

const solver_t solver_rk4 = {
    solver_rk4_init,
    solver_rk4_deinit,
    solver_rk4_reset,
    solver_rk4_start_step,
    solver_rk4_stop_step,
    solver_rk4_integrate,
};
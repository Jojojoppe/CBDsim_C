#include "sim.h"

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>

sim_state_t * sim_init(solver_t * solver, void * solver_params){
    sim_state_t * state = (sim_state_t*) calloc(1, sizeof(sim_state_t));
    // Set solver info
    state->solver = solver;
    state->solver_state = solver->init(solver_params);
    // Set visualizer
    state->vis = visualizer_init(); 
    return state;
}

void sim_deinit(sim_state_t * state){
    if(!state) return;
    state->solver->deinit(state->solver_state);
    if(state->model){
        dlclose(state->model);
    }
    if(state->values){
        free(state->values);
    }
    visualizer_deinit(state->vis);
    free(state);
}

int sim_compile_model(const char * modelcfile, const char * modelfile, sim_state_t * state){
    if(!state) return -1;
    int modelcfile_len = strlen(modelcfile);
    int modelfile_len = strlen(modelfile);
    char * totalcmd = malloc(modelcfile_len + modelfile_len + 64);

    sprintf(totalcmd, "cc -pie -shared -o \"%s\" \"%s\"", modelfile, modelcfile);

    int r = system(totalcmd);

    free(totalcmd);
    return r;
}

int sim_load_model(const char * modelfile, sim_state_t * state){
    if(!state) return -1;
    int modelfile_len = strlen(modelfile);
    char cwd_buf[512];
    char * cwd = getcwd(cwd_buf, 512);
    if(!cwd) return -1;
    int cwd_len = strlen(cwd);
    char * mfile = malloc(cwd_len + modelfile_len + 16);

    sprintf(mfile, "%s/%s", cwd, modelfile);

    state->model = dlopen(mfile, RTLD_NOW);
    free(mfile);

    if(!state->model) return -1;

    state->model_values = dlsym(state->model, "values");
    if(dlerror()) return -2;
    state->model_value_name = dlsym(state->model, "value_name");
    if(dlerror()) return -3;
    state->model_value_init = dlsym(state->model, "value_init");
    if(dlerror()) return -4;
    state->model_init = dlsym(state->model, "init");
    if(dlerror()) return -5;
    state->model_step = dlsym(state->model, "step");
    if(dlerror()) return -6;

    return 0;
}

void sim_init_run(sim_state_t * state){
    if(!state) return; // TODO error handling
    if(!state->model) return; // TODO error handling

    // if already done free values
    if(state->values) free(state->values);

    // Allocate values
    state->values = calloc(state->model_values(), sizeof(double));
    // Clear all other values
    state->time = state->timestep = 0;
    state->major = 1;
    state->minor = 0;

    // Load initial values
    for(int i=0; i<state->model_values(); i++){
        state->values[i] = state->model_value_init(i);
    }

    // Reset solver
    state->solver->reset(state->solver_state);

    // Initialize model
    state->model_init(state->values);

    // Setup visualizer
    visualizer_reset(state->vis);
    for(int i=0; i<state->model_values(); i++){
        visualizer_record_names(state->model_value_name(i), state->vis);
    }
}

void sim_step(sim_state_t * state){
    if(!state) return; // TODO error handling

    state->solver->start_step(state->solver_state);

    state->model_step(state->values, state->major, state->minor, 
        state->time, state->timestep, state->solver->integrate, state->solver_state
    );

    solver_step_end_retval_t r = state->solver->end_step(state->solver_state);
    state->major = r.major;
    state->minor = r.minor;
    state->timestep = r.timestep;
    state->time += r.timestep;
}

void sim_run(double runtime, sim_state_t * state){
    if(!state) return; // TODO error handling
    double starttime = state->time;
    visualizer_record_start(state->vis);
    while(state->time<=runtime+starttime){
        sim_step(state);
        visualizer_recordall(state->values, state->vis);
    }
    visualizer_record_end(state->vis);
}

void sim_plot(const char * options, sim_state_t * state){
    if(!state) return; // TODO error handling
    visualizer_plot(options, state->vis);
}

void sim_csv(const char * options, sim_state_t * state){
    if(!state) return; // TODO error handling
    visualizer_csv(options, state->vis);
}

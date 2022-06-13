#include "sim.h"

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

sim_state_t * sim_init(const solver_t * solver, void * solver_params, const char * viz){
    sim_state_t * state = (sim_state_t*) calloc(1, sizeof(sim_state_t));
    // Set solver info
    state->solver = solver;
    state->solver_state = solver->init(solver_params);
    // Set visualizer
    if(viz){
        state->viz = popen(viz, "w");
    }
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
    if(state->viz){
        fprintf(state->viz, "stop\n");
        // fprintf(state->viz, "quit\n");
        pclose(state->viz);
    }
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

    // Reset viz
    // TODO move to viz.c
    if(state->viz){
        fprintf(state->viz, "reset\n");
        fflush(state->viz);
    }

    // Set data format
    // TODO move to viz.c
    // FIXME not all values...
    if(state->viz){
        fprintf(state->viz, "format ");
        for(int i=0; i<state->model_values(); i++){
            char * name = state->model_value_name(i);
            fprintf(state->viz, "%s ", name);
        }
        fprintf(state->viz, "\n");
        fflush(state->viz);
    }
}

double sim_step(sim_state_t * state){
    if(!state) return; // TODO error handling

    state->solver->start_step(state->solver_state);

    state->model_step(state->values, state->major, state->minor, 
        state->time, state->timestep, state->solver->integrate, state->solver_state
    );

    double oldtimestep = state->timestep;

    solver_step_end_retval_t r = state->solver->end_step(state->solver_state);
    state->major = r.major;
    state->minor = r.minor;
    state->timestep = r.timestep;
    state->time += r.timestep;

    return oldtimestep;
}

void sim_run(double runtime, sim_state_t * state){
    if(!state) return; // TODO error handling
    double starttime = state->time;
    while(state->time<=runtime+starttime){
        sim_step(state);
        sim_plot_data_all(state);
    }

    // Update viz
    sim_plot_update(state);
}

void sim_run_realtime(double runtime, double updatef, double speed, sim_state_t * state){
    if(!state) return; // TODO error handling

    double starttime = state->time;

    // Get current system time
    struct timeval tv;
    double simpassed = 0.0;

    while(state->time<=runtime+starttime){
        gettimeofday(&tv, NULL);
        unsigned long tstart = 1000000ull*tv.tv_sec + tv.tv_usec;

        double delta = sim_step(state);
        sim_plot_data_all(state);

        simpassed += delta;
        if(simpassed>speed/updatef){
            simpassed = 0.0;
            sim_plot_update(state);
        }

        // Wait until now + timestep is gone
        gettimeofday(&tv, NULL);
        unsigned long tend = 1000000ull*tv.tv_sec + tv.tv_usec;
        unsigned long tused = tend-tstart;
        // Get usecods needed for timestep
        unsigned long treq = (unsigned long)(state->timestep/speed*1000000);
        if(tused<treq)
            usleep((treq-tused));
    }

    // Update viz
    sim_plot_update(state);
}
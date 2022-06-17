#include "sim.h"

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

int _sim_sighandler_INT;
void _sim_sighandler(int s){
    if(s==SIGINT) _sim_sighandler_INT = 1;
}

sim_state_t * sim_init(){
    sim_state_t * state = (sim_state_t*) calloc(1, sizeof(sim_state_t));

    // Add sighandler
    signal(SIGINT, _sim_sighandler);
    return state;
}

void sim_deinit(sim_state_t * state){
    if(!state) return;
    if(state->solver) state->solver->deinit(state->solver_state);
    if(state->model) dlclose(state->model);
    if(state->values) free(state->values);
    if(state->viz){
        // TODO move to plot.c
        fprintf(state->viz, "stop\n");
        // fprintf(state->viz, "quit\n");
        pclose(state->viz);
    }
    if(state->csv) fclose(state->csv);
    if(state->sample_times) free(state->sample_times);
    if(state->sample_at) free(state->sample_at);
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
    state->model_disdomains = dlsym(state->model, "disdomains");
    if(dlerror()) return -7;
    state->model_domain_name = dlsym(state->model, "domain_name");
    if(dlerror()) return -8;

    return 0;
}

void sim_init_run(sim_run_settings_t * settings, sim_state_t * state){
    if(!state) return; // TODO error handling
    if(!state->model) return; // TODO error handling

    // if already done free values
    if(state->values) free(state->values);
    // if already sample_at array free
    if(state->sample_at) free(state->sample_at);
    // if already sample_times array free
    if(state->sample_times) free(state->sample_times);

    // Allocate values
    state->values = calloc(state->model_values(), sizeof(double));
    // Allocate sample at array
    state->sample_at = calloc(state->model_disdomains(), sizeof(double));
    // Allocate sample times array
    state->sample_times = calloc(state->model_disdomains(), sizeof(double));
    // Clear all other values
    state->time = state->timestep = 0;
    state->major = 1;
    state->minor = 0;
    state->disdomains = state->model_disdomains();

    // Load initial values
    for(int i=0; i<state->model_values(); i++){
        state->values[i] = state->model_value_init(i);
    }

    // Load sample times
    double sampletime_last = 1.0;
    for(int i=0; i<state->disdomains; i++){
        if(i<settings->nr_discrete_settings){
            state->sample_times[i] = settings->discrete_settings[i].sampling_time;
            sampletime_last = state->sample_times[i];
        }else{
            state->sample_times[i] = sampletime_last;
        }
    }

    // Create solver if not exists or if different solver
    if(state->solver!=settings->solver){
        if(state->solver){
            // Already a solver: deinitialize it
            state->solver->deinit(state->solver_state);
        }
        state->solver = settings->solver;
        state->solver_state = settings->solver->init(settings->solver_settings);
    }
    // Reset solver
    state->solver->reset(state->solver_state);

    // Create visualizer if not exists of if different visualizer
    if(state->viz_type!=settings->viz){
        if(state->viz_type){
            // Already a visualizer: deinitialize it
            // TODO move to plot.c
            fprintf(state->viz, "stop\n");
            // fprintf(state->viz, "quit\n");
            pclose(state->viz);
        }
        state->viz_type = settings->viz;
        state->viz = popen(settings->viz, "w");
    }else{
        sim_plot_reset(state);
    }

    // Initialize model
    state->model_init(state->values);

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

void sim_run(double runtime, sim_state_t * state){
    if(!state) return; // TODO error handling
    if(!state->solver) return;

    double starttime = state->time;

    int step_type = 0; // Start with analog step
    double used_timestep = 0.0;
    double nxt_analog = 0.0;

    while(state->time<=runtime+starttime && !_sim_sighandler_INT){

        if(step_type==0){
            // Do analog step
            state->solver->start_step(state->solver_state, 0);
            used_timestep = state->timestep;
        }else{
            // Do discrete domain step
            state->solver->start_step(state->solver_state, 1);
            used_timestep = state->sample_at[step_type-1] - state->time;
            state->sample_at[step_type-1] += state->sample_times[step_type-1];
        }

        // If step was major step output values
        if(state->major*(step_type==0)){
            sim_plot_data_all(state);
            sim_csv_data_all(state);
        }

        // Run model step
        state->model_step(state->values, state->major*(step_type==0),
            state->minor||(step_type!=0), step_type, state->time, used_timestep,
            state->solver->integrate, state->solver->differentiate, state->solver_state
        );

        solver_step_end_retval_t r = state->solver->end_step(state->solver_state);
        state->major = r.major;
        state->minor = r.minor;
        state->timestep = r.timestep;

        state->time += used_timestep;

        if(step_type==0) nxt_analog = state->time + state->timestep;

        // Search for next step
        double nxt = nxt_analog; // Assume analog
        step_type = 0;
        for(int i=0; i<state->disdomains; i++){
            if(state->sample_at[i]<=nxt){
                nxt = state->sample_at[i];
                step_type = i+1;
            }
        }
        used_timestep = nxt - state->time;

    }

    // Update viz
    sim_plot_update(state);
}

void sim_run_realtime(double runtime, double updatef, double speed, sim_state_t * state){
    if(!state) return; // TODO error handling
    if(!state->solver) return;

    double starttime = state->time;

    int step_type = 0; // Start with analog step
    double used_timestep = 0.0;
    double nxt_analog = 0.0;

    // Get current system time
    struct timeval tv;
    double simpassed = 0.0;

    sim_plot_update(state);
    while(state->time<=runtime+starttime && !_sim_sighandler_INT){
        gettimeofday(&tv, NULL);
        unsigned long tstart = 1000000ull*tv.tv_sec + tv.tv_usec;

        if(step_type==0){
            // Do analog step
            state->solver->start_step(state->solver_state, 0);
            used_timestep = state->timestep;
        }else{
            // Do discrete domain step
            state->solver->start_step(state->solver_state, 1);
            used_timestep = state->sample_at[step_type-1] - state->time;
            state->sample_at[step_type-1] += state->sample_times[step_type-1];
        }

        // If step was major step output values
        if(state->major*(step_type==0)){
            sim_plot_data_all(state);
            sim_csv_data_all(state);
        }

        // Plot data if needed
        if(simpassed>speed/updatef){
            simpassed = 0.0;
            sim_plot_update(state);
        }
        simpassed += used_timestep;

        // Run model step
        state->model_step(state->values, state->major*(step_type==0),
            state->minor||(step_type!=0), step_type, state->time, used_timestep,
            state->solver->integrate, state->solver->differentiate, state->solver_state
        );

        solver_step_end_retval_t r = state->solver->end_step(state->solver_state);
        state->major = r.major;
        state->minor = r.minor;
        state->timestep = r.timestep;

        state->time += used_timestep;

        if(step_type==0) nxt_analog = state->time + state->timestep;

        // Search for next step
        double nxt = nxt_analog; // Assume analog
        step_type = 0;
        for(int i=0; i<state->disdomains; i++){
            if(state->sample_at[i]<=nxt){
                nxt = state->sample_at[i];
                step_type = i+1;
            }
        }
        used_timestep = nxt - state->time;

        // Wait for nxt
        gettimeofday(&tv, NULL);
        unsigned long tend = 1000000ull*tv.tv_sec + tv.tv_usec;
        unsigned long tused = tend-tstart;
        // Get usecods needed for timestep
        unsigned long tendreq = tstart + used_timestep*1000000ull;
        signed long treq = (signed long)tendreq-(signed long)tend;
        if(treq>0){
            usleep(treq);
        }

    }

    // Update viz
    sim_plot_update(state);
}
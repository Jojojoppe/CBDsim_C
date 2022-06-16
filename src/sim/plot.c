#include "sim.h"

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

void sim_plot_window(const char * name, const char * title, sim_state_t * state){
    if(!state) return; if(!state->viz) return;
    if(!name) return;

    fprintf(state->viz, "wind %s", name);
    if(title) fprintf(state->viz, " title:%s", title);
    fprintf(state->viz, "\n");
    fflush(state->viz);
}

void sim_plot(const char * wname, const char * pname, int loc, const char * title, sim_state_t * state, const char * options, int lines, ...){
    if(!state) return; if(!state->viz) return;
    if(!wname) return; if(!pname) return; if(loc==0) return; if(options==0) return;

    fprintf(state->viz, "plot %s %s %03d %s ", wname, pname, loc, options);
    if(title) fprintf(state->viz, "title:%s ", title);

    va_list va;
    va_start(va, lines);
    for(int i=0; i<lines; i++){
        char * line = va_arg(va, char*);
        fprintf(state->viz, "| %s ", line);
    }
    va_end(va);

    fprintf(state->viz, "\n");
    fflush(state->viz);
}

void sim_plot_update(sim_state_t * state){
    if(!state) return; if(!state->viz) return;
    fprintf(state->viz, "update\n");
    fflush(state->viz);
}

void sim_plot_data_all(sim_state_t * state){
    if(!state) return; if(!state->viz) return;
    // FIXME not all values...
    if(state->viz){
        fprintf(state->viz, "d ");
        for(int i=0; i<state->model_values(); i++){
            fprintf(state->viz, "%g ", state->values[i]);
        }
        fprintf(state->viz, "\n");
        fflush(state->viz);
    }
}

void sim_plot_reset(sim_state_t * state){
    if(!state) return; if(!state->viz) return;
    fprintf(state->viz, "reset\n");
    fflush(state->viz);
}
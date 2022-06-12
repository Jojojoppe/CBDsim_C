#include "visualizer.h"

#include <stdlib.h>

visualizer_t * visualizer_init(){
    visualizer_t * vis = calloc(1, sizeof(visualizer_t));
    vis->f = popen("./plot", "w");
    return vis;
}

void visualizer_deinit(visualizer_t * vis){
    if(!vis) return;
    fprintf(vis->f, "X\n");
    pclose(vis->f);
    free(vis);
}

void visualizer_reset(visualizer_t * vis){
    if(!vis) return;
    vis->n = 0;
}

void visualizer_record_names(const char * name, visualizer_t * vis){
    if(!vis) return;
    if(!vis->n++){
        fprintf(vis->f, "cols ");
    }
    fprintf(vis->f, "%s ", name);
}

void visualizer_record_start(visualizer_t * vis){
    if(!vis) return;
    fprintf(vis->f, "\ndata\n");
}

void visualizer_recordall(double * v, visualizer_t * vis){
    if(!vis) return;
    for(int i=0; i<vis->n; i++) fprintf(vis->f, "%e ", v[i]);
    fprintf(vis->f, "\n");
}

void visualizer_record(double v, visualizer_t * vis){
    if(!vis) return;
    fprintf(vis->f, "%e ", v);
    vis->_n++;
    if(vis->_n==vis->_n){
        vis->_n = 0;
        fprintf(vis->f, "\n");
    }
}

void visualizer_record_end(visualizer_t * vis){
    if(!vis) return;
    fprintf(vis->f, "e\n");
}

void visualizer_plot(const char * options, visualizer_t * vis){
    if(!vis) return;
    fprintf(vis->f, "plot %s\n", options);
}

void visualizer_csv(const char * options, visualizer_t * vis){
    if(!vis) return;
    fprintf(vis->f, "csv %s\n", options);
}
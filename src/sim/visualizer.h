#ifndef __H_VISUALIZER
#define __H_VISUALIZER

#include <stdio.h>

typedef struct {
    FILE * f;
    int n, _n;
} visualizer_t;

visualizer_t * visualizer_init();
void visualizer_deinit(visualizer_t * vis);

void visualizer_reset(visualizer_t * vis);
void visualizer_record_names(const char * name, visualizer_t * vis);

void visualizer_record_start(visualizer_t * vis);
void visualizer_recordall(double * v, visualizer_t * vis);
void visualizer_record(double v, visualizer_t * vis);
void visualizer_record_end(visualizer_t * vis);

void visualizer_plot(const char * options, visualizer_t * vis);
void visualizer_csv(const char * options, visualizer_t * vis);

#endif
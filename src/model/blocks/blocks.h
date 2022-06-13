#ifndef __H_BLOCKS_BLOCKS
#define __H_BLOCKS_BLOCKS

#include "../block.h"
#include "../model.h"

#define BLOCK_PARAM(nm, name, value, model) int p##nm; {char * s = calloc(1, strlen(name)+strlen(#nm)+2); sprintf(s, "%s/%s", #name #nm); p##nm = model_add_param(s, value, model); free(s);}

// Sources
// -------
int blocks_add_src_sine(double A, double f, const char * name, int out, model_t * model);
int blocks_add_src_cosine(double A, double f, const char * name, int out, model_t * model);

int blocks_add_src_step(double v, double t, const char * name, int out, model_t * model);
int blocks_add_src_istep(double v, double t, const char * name, int out, model_t * model);
int blocks_add_src_impulse(double A, double t, const char * name, int out, model_t * model);
int blocks_add_src_pulse(double v0, double v1, double t0, double t1, const char * name, int out, model_t * model);

int blocks_add_src_constant(double v, const char * name, int out, model_t * model);
int blocks_add_src_ramp(double a, double b, const char * name, int out, model_t * model);

#endif
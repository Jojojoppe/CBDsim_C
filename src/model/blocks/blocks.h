#ifndef __H_BLOCKS_BLOCKS
#define __H_BLOCKS_BLOCKS

#include "../block.h"
#include "../model.h"

int blocks_add_src_sine(double A, double f, const char * name, int out, model_t * model);
int blocks_add_src_cosine(double A, double f, const char * name, int out, model_t * model);

#endif
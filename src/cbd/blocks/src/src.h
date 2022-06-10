#ifndef __H_CBD_BLOCKS_SRC
#define __H_CBD_BLOCKS_SRC

#include "../../block.h"
#include "../../../sim.h"

int cbd_blocks_src_sin(int out, int A, int f, const char * name, sim_state_t * state);
int cbd_blocks_src_step(int out, int A, int t, const char * name, sim_state_t * state);

#endif
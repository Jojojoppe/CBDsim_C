#ifndef __H_CBD_BLOCKS_STANDARD
#define __H_CBD_BLOCKS_STANDARD

#include "../../block.h"
#include "../../../sim.h"

int cbd_block_standard_plusmin(const int * inp, int inp_n, const int * inm, int inm_n, int out, const char * name, sim_state_t * state);
int cbd_block_standard_muldiv(const int * inm, int inm_n, const int * ind, int ind_n, int out, const char * name, sim_state_t * state);

int cbd_block_standard_int_euler(int in, int out, int initial, const char * name, sim_state_t * state);
int cbd_block_standard_int_trap(int in, int out, int initial, const char * name, sim_state_t * state);
int cbd_block_standard_int_rk4(int in, int out, int initial, const char * name, sim_state_t * state);

int cbd_block_standard_gain(int in, int out, int gain, const char * name, sim_state_t * state);

#endif
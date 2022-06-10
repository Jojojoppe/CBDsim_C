#ifndef __H_CBD_PARAM
#define __H_CBD_PARAM

typedef struct sim_state_s sim_state_t;

typedef struct cbd_param_s {
    int value;      // Index to value array
    int name;       // Index to name array
} cbd_param_t;

int cbd_param_add(const char * name, double val, sim_state_t * state);
void cbd_param_deinit(cbd_param_t * sig);

#endif
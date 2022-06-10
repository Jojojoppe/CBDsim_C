#ifndef __H_CBD_SIGNAL
#define __H_CBD_SIGNAL

typedef struct sim_state_s sim_state_t;

typedef struct cbd_signal_s {
    int value;      // Index to value array
    int name;       // Index to name array
} cbd_signal_t;

int cbd_signal_add(const char * name, sim_state_t * state);
void cbd_signal_deinit(cbd_signal_t * sig);

#endif
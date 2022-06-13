#ifndef __H_MODEL_SIGNAL
#define __H_MODEL_SIGNAL

typedef struct signal_s{
    char * name;
} signal_t;

signal_t * signal_init(const char * name);
void signal_deinit(signal_t * signal);

#endif
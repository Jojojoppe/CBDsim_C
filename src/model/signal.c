#include "signal.h"

#include <string.h>
#include <stdlib.h>

signal_t * signal_init(const char * name){
    signal_t * s = calloc(1, sizeof(signal_t));

    // Copy name
    s->name = malloc(strlen(name)+1);
    strcpy(s->name, name);
    return s;
}

void signal_deinit(signal_t * signal){
    if(!signal) return;
    free(signal->name);
    free(signal);
}
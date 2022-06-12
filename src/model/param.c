#include "param.h"

#include <string.h>
#include <stdlib.h>

param_t * param_init(const char * name, double value){
    param_t * s = calloc(1, sizeof(param_t));

    // Copy name
    s->name = malloc(strlen(name)+1);
    strcpy(s->name, name);

    s->value = value;

    return s;
}

void param_deinit(param_t * param){
    if(!param) return;
    free(param->name);
    free(param);
}
#include "variable.h"

#include <string.h>
#include <stdlib.h>

variable_t * variable_init(const char * name){
    variable_t * s = calloc(1, sizeof(variable_t));

    // Copy name
    s->name = malloc(strlen(name)+1);
    strcpy(s->name, name);
    return s;
}

void variable_deinit(variable_t * variable){
    if(!variable) return;
    free(variable->name);
    free(variable);
}
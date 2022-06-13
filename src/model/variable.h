#ifndef __H_MODEL_VARIABLE
#define __H_MODEL_VARIABLE

typedef struct variable_s{
    char * name;
} variable_t;

variable_t * variable_init(const char * name);
void variable_deinit(variable_t * variable);

#endif
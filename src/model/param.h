#ifndef __H_MODEL_PARAM
#define __H_MODEL_PARAM

typedef struct param_s{
    char * name;
    double value;
} param_t;

param_t * param_init(const char * name, double value);
void param_deinit(param_t * param);

#endif
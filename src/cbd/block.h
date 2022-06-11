#ifndef __H_CBD_BLOCK
#define __H_CBD_BLOCK

typedef struct sim_state_s sim_state_t;

typedef struct cbd_block_s {
    int ports_in;       // Index to array array 
    int ports_out;      // Index to array array
    int params;         // Index to array array
    int name;           // Index to name array
    void * cache;       // Block of data used by the implemented block, is freed in deinit function (if not NULL)
    void (*eval)(struct cbd_block_s *, sim_state_t *);
    int eval_num;
    int depchain_break; // nonzero if breaks chain of dependencies
} cbd_block_t;  


int cbd_block_add(const char * name, const int * ports_in, int ports_in_n, const int * ports_out, int ports_out_n, const int * params, int params_n, int eval_func, int depchain_break, sim_state_t * state);
void cbd_block_deinit(cbd_block_t * block);

enum CBD_BLOCK_FUNCTION {
    SRC_CONSTANT = 0,
    SRC_STEP,
    SRC_ISTEP,
    SRC_PULSE,
    SRC_SIN,
    SRC_COS,

    STANDARD_GAIN,
    STANDARD_PLUSMIN,
    STANDARD_MULDIV,
    STANDARD_INT_EULER,
    STANDARD_INT_TRAP,
    STANDARD_INT_RK4,
    STANDARD_EXP,

    _CBD_BLOCK_FUNCTION_SIZE,
};

void cbd_block_register_eval_functions(sim_state_t * state);

#endif
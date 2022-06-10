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
} cbd_block_t;  


int cbd_block_add(const char * name, const int * ports_in, int ports_in_n, const int * ports_out, int ports_out_n, const int * params, int params_n, void (*eval)(cbd_block_t *, sim_state_t *), sim_state_t * state);
void cbd_block_deinit(cbd_block_t * block);

#endif
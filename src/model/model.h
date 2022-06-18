#ifndef __H_MODEL
#define __H_MODEL

#include "../general/dynamic_array.h"
#include "../general/hashmap.h"

#include <stdio.h>

typedef struct{
    // Model acts as a block-like structrure well
    char * name;
    d_array_t in, out;

    // Values
    d_array_t parameters;
    d_array_t variables;

    // Block diagram
    d_array_t signals;
    d_array_t blocks;

    // Bond graph
    d_array_t bonds; // note: each bond has one f and e
    d_array_t elements;

    // Name to index (for all) hashmap
    // Note: does not own the keys
    hashmap * hmap;
} model_t;

// Value holding types
// -------------------
typedef struct{
    double value;
    char * name;
} parameter_t;

typedef struct{
    double value;
    char * name;
} variable_t;

// Types with values attached
// --------------------------
typedef struct{
    char * name;
    int from_index;
    d_array_t to_index;
} signal_t;

typedef struct{
    char * name;
    int from_index;
    int to_index;
    int causality; // 0 means effort out (effort at to_index)
} bond_t;

// Equations generating types
// --------------------------
typedef int (*equations_gen_f)(FILE * fout, void * data);

typedef struct block_definition_s{
    char * type;        // Name of the type of the block
    size_t genparams_size;   // Size of generation parameter block
    // Output generation functions
    equations_gen_f gen;
    equations_gen_f gen_init;
    // Names of inputs, outputs, parameters and variables. One array and null
    // terminated for each type: {in, 0, out, 0, param, 0, var, 0}
    // Must match the numbers previously mentioned otherwise errors will be
    // thrown. If any number <32 is used this will array-ify the next name
    // ---> in[12]
    // TODO If -1 is used the next name will have an indefinite amount of values
    // if -2 is used the next name will be global and will have to be connected
    // manually
    const char * names[];
} block_definition_t;

typedef struct{
    char * name;
    d_array_t in, out;
    d_array_t parameters, variables;
    void * gen_params;
    const block_definition_t * block_def;
} block_t;

typedef struct{
    char * type;        // Name of the type of the block
    int inputs;         // Number of inputs for this block. -1 for unknown amount
    int outputs;        // Number of outputs for this block. -1 for unknown amount
    size_t genparams_size;   // Size of generation parameter block
    // Output generation functions
    equations_gen_f gen;
    equations_gen_f gen_init;
} element_definition_t;

typedef struct{
    char * name;
    d_array_t in, out;
    void * gen_params;
    const element_definition_t * element_def;
} element_t;

/* Initialize model
 * ----------------
 *  Allocates a model object and initializes used data structures
 *
 *  void
 *  -> model_t * : pointer to created model object. NULL if error occured
 */
model_t * model_init();

/* Deinitialze model
 * -----------------
 *  Frees all allocated memory in the model structure
 *  
 *  model_t * model : model object to initialize. May be NULL
 *  -> void
 */
void model_deinit(model_t * model);

/* Debug print
 * -----------
 *  Print list of all items in model
 * 
 *  model_t * model : model object
 *  -> void
 */
void model_debug(model_t * model);

/* Add parameter
 * -------------
 *  Add a parameter to the model
 * 
 *  char * name : name of the parameter. Must be unique name in model
 *  double value : parameter value
 *  model_t * model : model object
 *  -> int : index/ID of the parameter. -1 if error
 */
int model_add_parameter(char * name, double value, model_t * model);

/* Add variable
 * ------------
 *  Add a variable to the model
 * 
 *  char * name : name of the variable. Must be unique name in model
 *  double value : variable value
 *  model_t * model : model object
 *  -> int : index/ID of the variable. -1 if error
 */
int model_add_variable(char * name, double value, model_t * model);


/* Add signal
 * ----------
 *  Add a signal to the model
 * 
 *  char * name : name of the signal. Must be unique name in model
 *  model_t * model : model object
 *  -> int : index/ID of the signal. -1 if error
 */
int model_add_signal(char * name, model_t * model);

/* Add block
 * ---------
 *  Add a block to the model
 * 
 *  char * name : name of the block. Must be unique name in model
 *  const block_definition_t * definition : Object containing type of block
 *  double * parameters : Array of parameters (for global ones value is ignored)
 *  model_t * model : model object
 *  -> int : index/ID of the block. -1 if error
 */
int model_add_block(char * name, const block_definition_t * definition, double * parameters, model_t * model);

#endif
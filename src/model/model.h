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

    // Submodels
    // Note: No references to submodels! model owns its submodels
    d_array_t submodels;

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


// Equations generating types
// --------------------------
//typedef int (*equations_gen_f)(FILE * fout, void * data);
typedef char * equations_gen_f;

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

#define MODEL_INPUT 1
#define MODEL_OUTPUT 0
#define MODEL_INOUT_NOARRAY 0

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

/* Add model port
 * --------------
 *  Add input or output to a model
 * 
 *  char * name : name of the input/output. Must be a unique name in the model
 *  int direction : 1 for input, 0 for output
 *  int arr : number of elements in array. 0 for scalar
 *  model_t * model : model object
 *  -> int : index/ID of the signal. -1 if error. If array ID of first
 */
int model_add_inout(char * name, int direction, int arr, model_t * model);

/* Add submodel
 * ------------
 *  Add a submodel to the model
 * 
 *  Note: model owns its submodels!!
 *  char * name : name of the submodel
 *  model_t ** submodel : pointer to submodel pointer. May be NULL
 *  model_t * model : model object
 *  -> int : index/ID of the submodel. -1 if error
 */
int model_add_submodel(char * name, model_t ** submodel, model_t * model);

/* Connect signals
 * ---------------
 *  Connect two existing signals
 * 
 *  char * name_out : output side of a signal to connect to name_in
 *  char * name_in : input side of a signal to connect to name_out
 *  model_t * model : model object
 *  -> int : 0 if succeeded, anything else for errors
 */
int model_connect_signals(char * name_out, char * name_in, model_t * model);

/* Connect signals with name
 * -------------------------
 *  Connect two existing signals and add a name to the connection
 * 
 *  char * name : name of the connection
 *  char * name_out : output side of a signal to connect to name_in
 *  char * name_in : input side of a signal to connect to name_out
 *  model_t * model : model object
 *  -> int : 0 if succeeded, anything else for errors
 */
int model_connect_signals_named(char * name, char * name_out, char * name_in, model_t * model);

/* Update ports and signals
 * ------------------------
 *  Update connections, inputs and outputs and signals
 * 
 *  model_t * model
 *  -> void
 */
void model_update(model_t * model);

/* Flatten
 * -------
 *  Flatten model in order to compile
 * 
 *  input stays untouched and a full copy will be created in output
 * 
 *  model_t * in : model to flatten
 *  model_t ** out : output. Model object is created by this function
 *  -> int : 0 if succeeded, anything else for errors
 */
int model_flatten(model_t * in, model_t ** out);

/*
 * Export
 * ------
 *  Export model to file
 *
 *  model_t * model
 *  const char * fname : file to write to
 *  -> int : 0 if succeeded, anything else for errors
 */
int model_export(model_t * model, const char * fname);

/*
 * Import
 * ------
 *  Import model from file
 *
 *  const char * fname : file to read from
 *  model_t ** model : model object
 *  -> int : 0 if succeeded, anything else for errors
 */
int model_import(const char * fname, model_t ** model);

#endif

#include "model.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "../general/dynamic_array.h"
#include "../general/hashmap.h"
#include "../general/base64.h"

void _model_export_hashmap(void * key, size_t ksize, uintptr_t value, void * usr){
    FILE * f = (FILE*) usr;
    fprintf(f, "%s %d\n", (char*) key, value);
}

void _model_export_submodel(model_t * model, FILE * f){
    size_t osize;

    // model name
    fprintf(f, "================\nname %s\n", model->name);

    // model in
    fprintf(f, "in %d ", D_ARRAY_LEN(model->in));
    for(D_ARRAY_LOOP(int, i, model->in)){
        fprintf(f, "%d ", *i);
    }
    fprintf(f, "\n");

    // model out
    fprintf(f, "out %d ", D_ARRAY_LEN(model->out));
    for(D_ARRAY_LOOP(int, i, model->out)){
        fprintf(f, "%d ", *i);
    }
    fprintf(f, "\n");

    // model parameters
    fprintf(f, "params %d\n", D_ARRAY_LEN(model->parameters));
    for(D_ARRAY_LOOP(parameter_t, param, model->parameters)){
        fprintf(f, "%s %f\n", param->name, param->value);
    }

    // model variables
    fprintf(f, "variables %d\n", D_ARRAY_LEN(model->variables));
    for(D_ARRAY_LOOP(variable_t, var, model->variables)){
        fprintf(f, "%s %f\n", var->name, var->value);
    }

    // model signals
    fprintf(f, "signals %d\n", D_ARRAY_LEN(model->signals));
    for(D_ARRAY_LOOP(signal_t, sig, model->signals)){
        fprintf(f, "%s %d %d ", sig->name, sig->from_index, D_ARRAY_LEN(sig->to_index));
        for(D_ARRAY_LOOP(int, to, sig->to_index)){
            fprintf(f, "%d ", *to);
        }
        fprintf(f, "\n");
    }

    // model blocks
    fprintf(f, "blocks %d\n", D_ARRAY_LEN(model->blocks));
    for(D_ARRAY_LOOP(block_t, block, model->blocks)){
        // Basic block information
        fprintf(f, "name %s", block->name);
        fprintf(f, "\n\tin %d ", D_ARRAY_LEN(block->in));
        for(D_ARRAY_LOOP(int, i, block->in)) fprintf(f, "%d ", *i);
        fprintf(f, "\n\tout %d ", D_ARRAY_LEN(block->out));
        for(D_ARRAY_LOOP(int, i, block->out)) fprintf(f, "%d ", *i);
        fprintf(f, "\n\tparams %d ", D_ARRAY_LEN(block->parameters));
        for(D_ARRAY_LOOP(int, i, block->parameters)) fprintf(f, "%d ", *i);
        fprintf(f, "\n\tvariables %d ", D_ARRAY_LEN(block->variables));
        for(D_ARRAY_LOOP(int, i, block->variables)) fprintf(f, "%d ", *i);
        fprintf(f, "\n");

        // Block definition
        block_definition_t * def = block->block_def;
        fprintf(f, "\tname %s\n\tgenparams %d %s\n", def->type, def->genparams_size, 
                def->genparams_size>0 ? base64_encode(block->gen_params, def->genparams_size, &osize) : ".");
        osize = strlen(def->gen);
        fprintf(f, "\tgen %d %s\n", osize, osize>0 ? base64_encode(def->gen, osize, &osize) : ".");
        osize = strlen(def->gen_init);
        fprintf(f, "\tgeninit %d %s\n", osize, osize>0 ? base64_encode(def->gen_init, osize, &osize) : ".");

        // Block names
        fprintf(f, "\tnames ");
        /*int i = 0;*/
        /*int zerocounter = 0;*/
        /*while(zerocounter<4){*/
            /*if(def->names[i]){*/
                /*fprintf(f, "%s ", def->names[i]);*/
            /*}else{*/
                /*fprintf(f, ", ");*/
                /*zerocounter++;*/
            /*}*/
            /*i++;*/
        /*}*/
        fprintf(f, "\n");
    }

    // model string hashmap
    fprintf(f, "strings %d\n", hashmap_size(model->hmap));
    hashmap_iterate(model->hmap, _model_export_hashmap, f);


    // submodels
    fprintf(f, "submodules %d\n", D_ARRAY_LEN(model->submodels));
    for(D_ARRAY_LOOP(model_t*, submodel, model->submodels)){
        _model_export_submodel(*submodel, f);
    }
}

int model_export(model_t * model, const char * fname){
    FILE * f = fopen(fname, "w");
    if(f==NULL) return -1;

    // TODO check for correctness of model object
    
    build_decoding_table();

    _model_export_submodel(model, f);    

    base64_cleanup();
    fclose(f);
    return 0;
}

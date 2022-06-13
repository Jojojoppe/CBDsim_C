#include "model.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int _model_evaluate_chainbreaking(int i, model_t * model, int * block_evaluated, int * block_evaluated_round, d_array_t * eval_order, int * sig_from, d_array_t * sig_to){
    block_t * block = D_ARRAY_ATV(block_t*, &model->blocks, i);
    d_array_t * ina = &block->ports_in;
    for(int j=0; j<ina->filled_size; j++){
        int in = *(int*)d_array_at(ina, j);
        int from = sig_from[in];
        if(from<0) continue;
        block_t * from_block = D_ARRAY_ATV(block_t*, &model->blocks, from);

        if(!from_block->statefull){
            if(!block_evaluated[from]){
                _model_evaluate_chainbreaking(from, model, block_evaluated, block_evaluated_round, eval_order, sig_from, sig_to);
                block_evaluated[from] = 1;
                if(block_evaluated_round[from]){
                    printf("ERROR: Algebraic loop detected between blocks %s and %s\n", block->name, from_block->name);
                    return -1;   
                }
                block_evaluated_round[from] = 1;
                d_array_insert(eval_order, &from);
            }
        }
    }
    return 0;
}

int _model_findend_nonchainbreaking(int i, model_t * model, int * visited, int * sig_from, d_array_t * sig_to){
    block_t * block = D_ARRAY_ATV(block_t*, &model->blocks, i);
    if(visited[i]){
        printf("ERROR: Algebraic loop detected at block %s\n", block->name);
        return -2;
    }
    visited[i] = 1;
    for(int j=0; j<D_ARRAY_LEN(block->ports_out); j++){
        int out = D_ARRAY_ATV(int, &block->ports_out, j);
        d_array_t * toa = &sig_to[out];
        if(toa->filled_size==0){
            // Found end of path
            return i;
        }
        for(int k=0; k<toa->filled_size; k++){
            int toblock = *(int*)d_array_at(toa, k);
            int r = _model_findend_nonchainbreaking(toblock, model, visited, sig_from, sig_to);
            if(r!=-1) return r;
        }
    }
    return -1;
}

int model_evaluate(FILE * f, d_array_t * eval_order, model_t * model){
    fprintf(f, "#include <math.h>\n\ntypedef double (*integral_f)(double,double,int,void*);\n\n");

    fprintf(f, "#define INT(v, i) integral(v, i, integral_nr++, solver_state)\n\n");

    // Create values() function
    fprintf(f, "int values(){\n\treturn %d;\n}\n\n", D_ARRAY_LEN(model->signals)+D_ARRAY_LEN(model->params));

    // Create value_name(int i) function
    fprintf(f, "char * value_name(int i){\n\tchar * names[] = {\n\t\t");
    for(int i=0; i<D_ARRAY_LEN(model->signals); i++){
        fprintf(f, "\"%s\", ", (D_ARRAY_ATV(signal_t*, &model->signals, i))->name);
    }
    for(int i=0; i<D_ARRAY_LEN(model->params); i++){
        fprintf(f, "\"%s\", ", (D_ARRAY_ATV(param_t*, &model->params, i))->name);
    }
    fprintf(f, "\n\t};\n\treturn names[i];\n}\n\n");

    // Create value_init(int i) function
    fprintf(f, "double value_init(int i){\n\tdouble initvals[] = {\n\t\t");
    for(int i=0; i<D_ARRAY_LEN(model->signals); i++){
        fprintf(f, "0.0, ");
    }
    for(int i=0; i<D_ARRAY_LEN(model->params); i++){
        fprintf(f, "%f, ", (D_ARRAY_ATV(param_t*, &model->params, i))->value);
    }
    fprintf(f, "\n\t};\n\treturn initvals[i];\n}\n\n");
    
    // Create init function
    fprintf(f, "void init(double * values){\n");
    fprintf(f, "int integral_nr = 0;\ndouble * signals = values;\ndouble * params = values+%d;\n", D_ARRAY_LEN(model->signals));
    // Evaluate all blocks in order
    for(int i=0; i<D_ARRAY_PLEN(eval_order); i++){
        int block_i = D_ARRAY_ATV(int, eval_order, i);
        block_t * block = D_ARRAY_ATV(block_t*, &model->blocks, block_i);
        block->generate_init(f, block);
    }
    fprintf(f, "}\n\n");

    // Create step function
    fprintf(f, "void step(double * values, int major, int minor, double time, double timestep, integral_f integral, void * solver_state){\n");
    fprintf(f, "int integral_nr = 0;\ndouble * signals = values;\ndouble * params = values+%d;\n", D_ARRAY_LEN(model->signals));
    fprintf(f, "values[0] = time;\nvalues[1] = timestep;\n"); // FIXME not hardcoded?
    // Evaluate all blocks in order
    for(int i=0; i<D_ARRAY_PLEN(eval_order); i++){
        int block_i = D_ARRAY_ATV(int, eval_order, i);
        block_t * block = D_ARRAY_ATV(block_t*, &model->blocks, block_i);
        block->generate(f, block);
    }
    fprintf(f, "}\n\n");
}

int model_compile(const char * outfile, model_t * model){
    if(!model) return -1;
    FILE * f = fopen(outfile, "w");

    int err = 0;

    int * sig_from = calloc(D_ARRAY_LEN(model->signals), sizeof(int));
    d_array_t * sig_to = calloc(D_ARRAY_LEN(model->signals), sizeof(d_array_t));
    for(int i=0; i<D_ARRAY_LEN(model->signals); i++) D_ARRAY_INIT(int, sig_to+i);
    for(int i=0; i<D_ARRAY_LEN(model->signals); i++) sig_from[i] = -1;
    // Set time and timestep to be set from -2
    sig_from[0] = sig_from[1] = -2;
    d_array_t eval_order;
    D_ARRAY_INIT(int, &eval_order);
    int * block_evaluated = calloc(D_ARRAY_LEN(model->blocks), sizeof(int));

    // Map signals to blocks
    // Loop over blocks to create mapping
    for(int i=0; i<D_ARRAY_LEN(model->blocks); i++){
        block_t * block = D_ARRAY_ATV(block_t*, &model->blocks, i);
        // Set from values
        for(int j=0; j<D_ARRAY_LEN(block->ports_out); j++){
            int sf = D_ARRAY_ATV(int, &block->ports_out, j);
            if(sig_from[sf]!=-1){
                err = 1;
                printf("ERROR: block %s sets output to already connected signal %s\n", block->name, (D_ARRAY_ATV(signal_t*, &model->signals, sf))->name);
                goto compileclean;
            }
            sig_from[sf] = i;
        }
        // set to values
        for(int j=0; j<D_ARRAY_LEN(block->ports_in); j++){
            int si = D_ARRAY_ATV(int, &block->ports_in, j);
            d_array_insert(sig_to+si, &i);
        }
    }

    // Find block order
    // Start from all chain breaking blocks and traverse down
    for(int i=0; i<D_ARRAY_LEN(model->blocks); i++){
        block_t * block = D_ARRAY_ATV(block_t*, &model->blocks, i);
        if(!block->statefull) continue;

        int * block_evaluated_round = calloc(D_ARRAY_LEN(model->blocks), sizeof(int));
        int r = _model_evaluate_chainbreaking(i, model, block_evaluated, block_evaluated_round, &eval_order, sig_from, sig_to);
        free(block_evaluated_round);
        if(r){
            err = 1;
            goto compileclean;
        }
        d_array_insert(&eval_order, &i);
    }

    // Loop over all non-evaluated blocks (non-chain breaking / algebraic paths), traverse to top and evaluate
    for(int i=0; i<D_ARRAY_LEN(model->blocks); i++){
        block_t * block = D_ARRAY_ATV(block_t*, &model->blocks, i);
        if(block->statefull || block_evaluated[i]) continue;

        int * visited = calloc(D_ARRAY_LEN(model->blocks), sizeof(int));
        int end = _model_findend_nonchainbreaking(i, model, visited, sig_from, sig_to);
        if(end>-1){
            int * block_evaluated_round = calloc(D_ARRAY_LEN(model->blocks), sizeof(int));
            _model_evaluate_chainbreaking(end, model, block_evaluated, block_evaluated_round, &eval_order, sig_from, sig_to);
            free(block_evaluated_round);
            d_array_insert(&eval_order, &end);
            block_evaluated[end] = 1;
        }
        free(visited);
        if(end==-2){
            err = 1;
            goto compileclean;
        }
    }

    // DONE COMPILING
    // EVALUATE
    model_evaluate(f, &eval_order, model);
    

compileclean:
    free(block_evaluated);
    d_array_deinit(&eval_order);
    free(sig_from);
    for(int i=0; i<D_ARRAY_LEN(model->signals); i++) d_array_deinit(sig_to +i);
    free(sig_to); 

    fclose(f);

    return err;
}

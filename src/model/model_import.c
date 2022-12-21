#include "model.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "../general/dynamic_array.h"
#include "../general/hashmap.h"
#include "../general/base64.h"

int _model_import_model(model_t * model, FILE * f){
    char * buf = malloc(1024);
    int num, val;

    // Should expect 16 ='s then name
    fscanf(f, "================\nname %s\n", buf);
    free(model->name);
    model->name = malloc(strlen(buf)+1);
    strcpy(model->name, buf);

    // in
    fscanf(f, "in %d", &num);
    if(num){
        for(int i=0; i<num; i++){
            fscanf(f, "%d", &val);
            d_array_insert(&model->in, &val);
        };
    }
    fscanf(f, "\n");

    // out
    fscanf(f, "out %d", &num);
    if(num){
        for(int i=0; i<num; i++){
            fscanf(f, "%d", &val);
            d_array_insert(&model->out, &val);
        };
    }
    fscanf(f, "\n");

    // params
    fscanf(f, "params %d\n", &num);
    if(num){
        for(int i=0; i<num; i++){
            parameter_t p;
            fscanf(f, "%s %lf\n", buf, &p.value);
            p.name = malloc(strlen(buf)+1);
            strcpy(p.name, buf);
            d_array_insert(&model->parameters, &p);
        };
    }

    // variables
    fscanf(f, "variables %d\n", &num);
    if(num){
        for(int i=0; i<num; i++){
            variable_t p;
            fscanf(f, "%s %lf\n", buf, &p.value);
            p.name = malloc(strlen(buf)+1);
            strcpy(p.name, buf);
            d_array_insert(&model->variables, &p);
        };
    }

    // signals
    fscanf(f, "signals %d\n", &num);
    if(num){
        for(int i=0; i<num; i++){
            signal_t s;
            fscanf(f, "%s %d %d\n", buf, &s.from_index, &val);
            s.name = malloc(strlen(buf)+1);
            strcpy(s.name, buf);
            D_ARRAY_INIT(int, &s.to_index);
            for(int j=0; j<val; j++){
                int to;
                fscanf(f, "%d", &to);
                d_array_insert(&s.to_index, &to);
            }
            d_array_insert(&model->signals, &s);
            fscanf(f, "\n");
        };
    }

    // blocks
    fscanf(f, "blocks %d\n", &num);
    if(num){
        for(int i=0; i<num; i++){
            block_t b;
            fscanf(f, "name %s\n", buf);
            b.name = malloc(strlen(buf)+1);
            strcpy(b.name, buf);

            fscanf(f, "\tin %d ", &val);
            D_ARRAY_INIT(int, &b.in);
            for(int j=0; j<val; j++){
                int v;
                fscanf(f, "%d", &v);
                d_array_insert(&b.in, &v);
            }



            fscanf(f, "\n\tout %d ", &val);
            D_ARRAY_INIT(int, &b.out);
            for(int j=0; j<val; j++){
                int v;
                fscanf(f, "%d", &v);
                d_array_insert(&b.out, &v);
            }

            fscanf(f, "\n\tparams %d ", &val);
            D_ARRAY_INIT(int, &b.parameters);
            for(int j=0; j<val; j++){
                int v;
                fscanf(f, "%d", &v);
                d_array_insert(&b.parameters, &v);
            }

            fscanf(f, "\n\tvariables %d ", &val);
            D_ARRAY_INIT(int, &b.variables);
            for(int j=0; j<val; j++){
                int v;
                fscanf(f, "%d", &v);
                d_array_insert(&b.variables, &v);
            }

            // block definition
            block_definition_t * def = calloc(sizeof(block_definition_t), 1);

            fscanf(f, "\tname %s\n", buf);
            def->type = malloc(strlen(buf)+1);
            strcpy(def->type, buf);

            fscanf(f, "\tgenparams %d %s\n", &val, buf);
            def->genparams_size = val;
            if(val){
                b.gen_params = base64_decode(buf, strlen(buf), &val);
            }else{
                b.gen_params = NULL;
            }

            fscanf(f, "\tgen %d %s\n", &val, buf);
            if(val){
                def->gen = base64_decode(buf, strlen(buf), &val);
            }else{
                def->gen = "";
            }

            fscanf(f, "\tgeninit %d %s\n", &val, buf);
            if(val){
                def->gen_init = base64_decode(buf, strlen(buf), &val);
            }else{
                def->gen_init = "";
            }

            // TODO names
            fscanf(f, "names \n");

            b.block_def = def;

            d_array_insert(&model->blocks, &b);
        };
    }

    // strings
    fscanf(f, "strings %d\n", &num);
    if(num){
        for(int i=0; i<num; i++){
            uintptr_t d;
            fscanf(f, "%s %d\n", buf, &d);
            char * key = malloc(strlen(buf)+1);
            strcpy(key, buf);
            key[strlen(buf)] = 0;
            hashmap_set(model->hmap, key, strlen(buf), d);
        }
    }

    // submodules
    fscanf(f, "submodules %d\n", &num);
    if(num){
        for(int i=0; i<num; i++){
            model_t * submodel;
            model_add_submodel("__submodel__", &submodel, model);
            _model_import_model(submodel, f);
            model_update(model);
        }
    }

    free(buf);
    return 0;
}

int model_import(const char * fname, model_t ** model){
    FILE * f = fopen(fname, "r");
    if(f==NULL) return -1;

    build_decoding_table();

    model_t * toplevel = model_init("__toplevel__");

    if(_model_import_model(toplevel, f)){
        model_deinit(toplevel);
        return -1;
    }

    *model = toplevel;
    base64_cleanup();
    fclose(f);
    return 0;
}

#include <math.h>

typedef double (*integral_f)(double,double,int,void*);

// GENERATED BY MODELING ENGINE
// ----------------------------

int values(){
    return 14;
}

char * value_name(int i){
    char * names[] = {
        "e1", "e2", "e3", "e4", "f", "q3", "p2",
        "sA", "sf", "iIi", "iCi", "IK", "CK", "RK",
    };
    return names[i];
}

double value_init(int i){
    double initvals[] = {
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.5,
    };
    return initvals[i];
}

void init(double * values){
    // block 6: sin
    // block 3: gain
    // block 2: gain
    // block 4: gain
    // block 5: pm
    // block 0: int
    // block 1: int
}

void step(double * values, int major, int minor, double time, double timestep, integral_f integral, void * solver_state){

    int integral_nr = 0;

    // block 6: step
    if(time<=values[8]) values[0] = 0.0;
    else values[0] = values[7];

    // block 3: gain
    values[2] = values[12] * values[5];

    // block 2: gain
    values[4] = values[11] * values[6];

    // block 4: gain
    values[3] = values[13] * values[4];

    // block 5: pm
    values[1] = values[0] - values[2] - values[3];

    // block 0: int
    values[6] = integral(values[1], values[9], integral_nr++, solver_state);

    // block 1: int
    values[5] = integral(values[4], values[10], integral_nr++, solver_state);

}
// ----------------------------
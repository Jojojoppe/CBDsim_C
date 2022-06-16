#include <math.h>

typedef double (*integral_f)(double,double,int,void*);
typedef double (*differentiate_f)(double,double,int,void*);

#define INT(v, i) integral(v, i, integral_nr++, solver_state)
#define DDT(v, i) differentiate(v, i, differential_nr++, solver_state)

int values(){
	return 21;
}

char * value_name(int i){
	char * names[] = {
		"time", "timestep", "e1", "e2", "e3", "e4", "f", "q3", "p2", "q0", "src/v", "src/t", "iC/initial", "iI/initial", "KC/k", "KI/k", "KR/k", "zero/v", "testval", "testval2",
		"ddt",
	};
	return names[i];
}

double value_init(int i){
	double initvals[] = {
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.000000, 10.000000, 1.000000, 0.000000, 1.000000, 1.000000, 1.000000, 0.000000, 0.0, 0.0,
		0.0,
	};
	return initvals[i];
}

int disdomains(){
	return 2;
}

double disdomain_ts(int i){
	double tsses[] = {
		0.3, 1.0,
	};
	return tsses[i];
}

void init(double * values){
int integral_nr = 0;
double * signals = values;
double * vars = signals+10;
double * params = vars+0;
}

void step(double * values, int major, int minor, int disdomain, double time, double timestep, integral_f integral, differentiate_f differentiate, void * solver_state){
int integral_nr = 0;
int differential_nr = 0;
double * signals = values;
double * vars = signals+10;
double * params = vars+0;
values[0] = time;
values[1] = timestep;
signals[6] = params[5]*signals[8];
signals[7] = INT(signals[6], params[2]);
signals[2] = time<params[1] ? 0.0 : params[0];
signals[4] = params[4]*signals[7];
signals[5] = params[6]*signals[6];
signals[3] = 0.0 + signals[2] - signals[4] - signals[5] ;
signals[8] = INT(signals[3], params[3]);
signals[9] = params[7];

if(disdomain==1) values[18] = values[4];
if(disdomain==2) values[19] = values[4];
values[20] = DDT(values[4], 0.0);
}



#include <dc/fmath.h>
#include <dc/vector.h>

#include "helpers.h"

void setVector(vector_t* v, float x, float y, float z, float w){
	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
}

void normalize3(vector_t* v){
	float n2 = v->x * v->x + v->y * v->y + v->z * v->z;
	float n = fsqrt(n2);
	v->x /= n; 
	v->y /= n;
	v->z /= n;
}

float clamp(float x, float a, float b){
	return (x <= a ? a : (x >= b ? b : x));
}

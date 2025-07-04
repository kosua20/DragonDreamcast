#ifndef HELPERS_H
#define HELPERS_H

#define MAX(A, B) ((A) < (B) ? (B) : (A))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

void setVector(vector_t* v, float x, float y, float z, float w);

void normalize3(vector_t* v);

float clamp(float x, float a, float b);

#endif

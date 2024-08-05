#pragma once

#include "idmfc.h"


inline void vm_SubVectors (vector *result,const vector *a, const vector *b)
{
	// Subtracts second vector from first.  Either source can equal dest

	result->x=a->x-b->x;
	result->y=a->y-b->y;
	result->z=a->z-b->z;
}

inline float vm_GetMagnitude (vector *a)
{
	float f;

	f=(a->x * a->x) + (a->y * a->y) + (a->z * a->z); 

	return (sqrt(f)); 
}

inline float vm_VectorDistance (const vector *a,const vector *b)
{
	// Given two vectors, returns the distance between them

	vector dest;
	float dist;

	vm_SubVectors (&dest,a,b);
	dist=vm_GetMagnitude (&dest);
	return dist;
}

inline float vm_GetMagnitudeFast(vector *v)
{
   float a,b,c,bc;

    a = fabs(v->x);
    b = fabs(v->y);
	c = fabs(v->z);

	if (a < b) {
		float t=a; a=b; b=t;
	}

	if (b < c) {
		float t=b; b=c; c=t;

		if (a < b) {
			float t=a; a=b; b=t;
		}
	}

	bc = (b/4) + (c/8);

	return a + bc + (bc/2);
}

inline float vm_VectorDistanceQuick (vector *a,vector *b)
{
	// Given two vectors, returns the distance between them

	vector dest;
	float dist;

	vm_SubVectors (&dest,a,b);
	dist=vm_GetMagnitudeFast (&dest);
	return dist;
}


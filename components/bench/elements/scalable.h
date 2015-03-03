
#ifndef SCALABLE_H
#define SCALABLE_H

#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
using namespace std;

typedef double				DOUBLE;
typedef unsigned			UINT32;

#define USE_WRAP /* wrapping through the buffer eradicates apparent architecture-dependent fine-scale timing variations */
#define WRAP 32

#define USE_FLOAT /* work the float processor if defined, integer processor if not */

#ifdef USE_FLOAT
typedef DOUBLE STATE_TYPE;
#else
typedef UINT32 STATE_TYPE;
#endif

struct PARS
{
	//	scaling
	UINT32					P;	//	number of processes
	UINT32					E;	//	number of elements per process
	UINT32					O;	//	number of operations per element
	UINT32					N;	//	number of computation steps
};

struct STATE
{
	void init(PARS& pars, UINT32 processIndex);
	STATE_TYPE* step(STATE_TYPE in);

	PARS pars;
	vector<STATE_TYPE> V;
};

#endif


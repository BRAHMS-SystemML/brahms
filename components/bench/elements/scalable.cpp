

#include "scalable.h"


void STATE::init(PARS& pars, UINT32 processIndex)
{
	//	store pars
	this->pars = pars;

	//	resize storage
	V.resize(pars.E);

	//	initial conditions THESE CURRENTLY ARE OVERWRITTEN IN STEP! but no matter...
	for (UINT32 e = 0; e < pars.E; e++)
		V[e] = e + processIndex;
}

STATE_TYPE* STATE::step(STATE_TYPE in)
{
	//	extract some stuff
	STATE_TYPE* V = &this->V[0];
	UINT32 E = pars.E;
	UINT32 O = pars.O;

	//	loop over operations
	for (UINT32 o=0; o<O; o++)
	{

#ifdef USE_WRAP

		//	loop over elements
		for (UINT32 s=0; (s<WRAP && s<E); s++)
		{

			//	loop over elements
			for (UINT32 e=s; e<E; e+=WRAP)
			{
				//	float op
				in = (in + 1.0) / 1.0001;
				V[e] = in;

/*
				//	alt. (float) op
				a = (a + 1.0) / 1.0001;
				V[e] = a;
*/
			}

		}

		//V[0] += V[E-1];

#else

		//	loop over elements
		for (UINT32 e=0; e<E; e++)
		{
			/*
			//	operation
			in += V[e];
			V[e] = in % 0xFF;
			*/

			//	float op
			in = (in + 1.0) / 1.0001;
			V[e] = in;
		}

//		V[0] += V[E-1];

#endif

	}

	//	return state
	return &V[0];
}

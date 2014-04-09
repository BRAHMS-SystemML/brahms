/*
________________________________________________________________

	This file is part of BRAHMS
	Copyright (C) 2007 Ben Mitchinson
	URL: http://brahms.sourceforge.net

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
________________________________________________________________

	Subversion Repository Information (automatically updated on commit)

	$Id:: util_rng.cpp 2437 2009-12-13 19:06:12Z benjmitch     $
	$Rev:: 2437                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-13 19:06:12 +0000 (Sun, 13 Dec 2009)       $
________________________________________________________________

*/






////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "std/2009/util/rng"
#define COMPONENT_CLASS_CPP class__std_2009_util_rng_0
#define COMPONENT_FLAGS (0)

//	include
#include "../utility.h"

//	include header
#include "std/2009/util/rng/brahms/0/utility.h"
namespace rng = std_2009_util_rng_0;

//	include STL
#include <cstdlib> // abs(INT32) used in MT2000 on linux


#include <iostream>
using namespace std;


#define ENSURE_EXISTS { if (!generator) berr << "generator not yet selected"; }





////////////////	RNGs

//	base class
class RNG
{

public:

	virtual ~RNG() {};
	virtual void select(const char* distribution) = 0;
	virtual void seed(const UINT32* seed, UINT32 count) = 0;
	virtual void fill(DOUBLE* dst, UINT32 cnt, DOUBLE gain = 1.0, DOUBLE offset = 0.0) = 0;
	virtual void fill(SINGLE* dst, UINT32 cnt, SINGLE gain = 1.0, SINGLE offset = 0.0) = 0;
	virtual DOUBLE get() = 0;

};

//	include RNGs
#include "util_rng.MT2000.h"








////////////////	COMPONENT CLASS

class COMPONENT_CLASS_CPP : public Utility
{

public:

	COMPONENT_CLASS_CPP();
	~COMPONENT_CLASS_CPP();

	void select(const char* select);
	void seed(const UINT32* seed, UINT32 count);

	Symbol event(Event* event);

	DOUBLE double_ret;
	INT64 cachedims;
	VBYTE cache;

	RNG* generator;

};









////////////////	OPERATION METHODS

const Symbol GENERIC_FUNCTION_SELECT = 1;
const Symbol GENERIC_FUNCTION_SEED = 2;
const Symbol GENERIC_FUNCTION_FILL = 3;
const Symbol GENERIC_FUNCTION_GET = 4;

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP()
{
	double_ret = 0;
	cachedims = 0;
	generator = NULL;
}

COMPONENT_CLASS_CPP::~COMPONENT_CLASS_CPP()
{
	//	BUG FOUND BY valgrind! (not deleting this object on dtor)
	//	delete if exists
	if (generator) delete generator;
	generator = NULL;
}

UINT64 getNumberOfElements(Dimensions& dims)
{
	UINT64 numEls = 1;
	for (UINT32 d=0; d<dims.count; d++)
		numEls *= dims.dims[d];
	return numEls;
}

bool isScalar(Dimensions& dims)
{
	if (dims.count < 1) return false;
	UINT64 numEls = 1;
	for (UINT32 d=0; d<dims.count; d++)
		numEls *= dims.dims[d];
	return numEls == 1;
}

void COMPONENT_CLASS_CPP::select(const char* p_select)
{
	//	string
	string select = (const char*) p_select;

	//	get generator part
	size_t pos = select.find(".");
	if (pos == string::npos) berr << "invalid generator string \"" << select << "\"";
	string gen = select.substr(0, pos);
	string dist = select.substr(pos + 1);

	//	delete if exists
	if (generator) delete generator;
	generator = NULL;

	//	create
	if (gen == "MT2000")
		generator = new MT2000;

	//	check
	if (!generator)
		berr << "unrecognised generator \"" << gen << "\"";

	//	set distribution
	try
	{
		generator->select(dist.c_str());
	}
	catch(const char* e)
	{
		berr << e;
	}
}

void COMPONENT_CLASS_CPP::seed(const UINT32* seed, UINT32 count)
{
	if (!count) berr << "empty seed is invalid";
	for (UINT32 c=0; c<count; c++)
		if (!seed[c]) berr << "zero element in seed is invalid";
	generator->seed(seed, count);
}

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{

	switch(event->type)
	{

		case rng::EVENT_UTILITY_SELECT:
		{
			//	validate
			if (!event->data) berr << E_NULL_ARG;

			//	select
			select((const char*) event->data);

			//	ok
			return C_OK;
		}



		case rng::EVENT_UTILITY_SEED:
		{

			rng::Seed* seed = (rng::Seed*) event->data;

			ENSURE_EXISTS;

			//	seed
			this->seed(seed->seed, seed->count);

			//	ok
			return C_OK;

		}



		case rng::EVENT_UTILITY_GET:
		{
			ENSURE_EXISTS;

			//	get value
			double_ret = generator->get();
			event->data = &double_ret;

			//	ok
			return C_OK;
		}

		

		case rng::EVENT_UTILITY_FILL:
		{
			rng::Fill* efill = (rng::Fill*) event->data;

			ENSURE_EXISTS;

			//	switch
			switch(efill->type)
			{
				case TYPE_DOUBLE:
				{
					DOUBLE* dst = (DOUBLE*) efill->dst;
					generator->fill(dst, efill->count, efill->gain, efill->offset);
					break;
				}

				case TYPE_SINGLE:
				{
					SINGLE* dst = (SINGLE*) efill->dst;
					generator->fill(dst, efill->count, (SINGLE)efill->gain, (SINGLE)efill->offset);
					break;
				}

				default:
					berr << "invalid call to EVENT_UTILITY_FILL (wrong numeric type)";
			}

			//	ok
			return C_OK;

		}



		case EVENT_STATE_GET:
		case EVENT_STATE_SET:
		{
			//	here, we get/set the complete state of the generator as a string, to
			//	make it possible for it to be serialized by the framework
			return E_NOT_IMPLEMENTED;
		}




		case EVENT_FUNCTION_GET:
		{
			EventFunctionGet* egf = (EventFunctionGet*) event->data;

			//	resolve function name to function event (handle)
			string func = egf->name;

			//	handle case
			if (func == "select")
			{
				//	ok
				egf->handle = GENERIC_FUNCTION_SELECT;
				egf->argumentModifyCount = 0;
				return C_OK;
			}

			//	handle case
			if (func == "seed")
			{
				//	ok
				egf->handle = GENERIC_FUNCTION_SEED;
				egf->argumentModifyCount = 0;
				return C_OK;
			}

			//	handle case
			if (func == "fill")
			{
				//	ok
				egf->handle = GENERIC_FUNCTION_FILL;
				egf->argumentModifyCount = 1;
				return C_OK;
			}

			//	handle case
			if (func == "get")
			{
				//	ok
				egf->handle = GENERIC_FUNCTION_GET;
				egf->argumentModifyCount = 1;
				return C_OK;
			}

			//	ok
			return E_FUNC_NOT_FOUND;
		}



#define TYPE_EL_MASK (TYPE_ELEMENT_MASK | TYPE_COMPLEX_MASK)

#define ASSERT_ARG_COUNT(count) { if (ecf->argumentCount != count) { ecf->argumentCount = count; berr << E_BAD_ARG_COUNT; } }

//	real vector args work with any value of TYPE_CPXFMT_MASK and TYPE_ORDER_MASK, transparently
#define ASSERT_ARG_REAL_VECTOR(index, ptype) { \
	if ((ecf->arguments[index]->type & TYPE_EL_MASK) != (ptype | TYPE_REAL)) { ecf->argumentCount = index; berr << E_BAD_ARG_TYPE; } \
	if (!((ecf->arguments[index]->dims.count == 1) || (ecf->arguments[index]->dims.count == 2 && ecf->arguments[index]->dims.dims[1] == 1))) { ecf->argumentCount = index; berr << E_BAD_ARG_SIZE; } \
	}

//	real scalar args work with any value of TYPE_CPXFMT_MASK and TYPE_ORDER_MASK, transparently
#define ASSERT_ARG_REAL_SCALAR(index, ptype) { \
	if ((ecf->arguments[index]->type & TYPE_EL_MASK) != (ptype | TYPE_REAL)) { ecf->argumentCount = index; berr << E_BAD_ARG_TYPE; } \
	if (!isScalar(ecf->arguments[index]->dims)) { ecf->argumentCount = index; berr << E_BAD_ARG_SIZE; } \
	}




		case EVENT_FUNCTION_CALL:
		{
			EventFunctionCall* ecf = (EventFunctionCall*) event->data;

			//	switch on function handle
			switch(ecf->handle)
			{

				case GENERIC_FUNCTION_SELECT:
				{
					//	arg count
					ASSERT_ARG_COUNT(1);

					//	argument 1: select type
					ASSERT_ARG_REAL_VECTOR(0, TYPE_CHAR8);

					//	select
					select((const char*) ecf->arguments[0]->real);

					//	ok
					return C_OK;
				}

				case GENERIC_FUNCTION_SEED:
				{
					ENSURE_EXISTS;

					//	arg count
					ASSERT_ARG_COUNT(1);

					//	argument 1: select type
					ASSERT_ARG_REAL_VECTOR(0, TYPE_UINT32);

					//	seed
					UINT32* seed = (UINT32*)ecf->arguments[0]->real;
					UINT32 count = getNumberOfElements(ecf->arguments[0]->dims);
					this->seed(seed, count);

					//	ok
					return C_OK;
				}

				case GENERIC_FUNCTION_FILL:
				{
					ENSURE_EXISTS;

					//	arg count
					ASSERT_ARG_COUNT(4);

					//	argument 1: count
					ASSERT_ARG_REAL_SCALAR(1, TYPE_UINT32);
					UINT32 count = ((UINT32*)ecf->arguments[1]->real)[0];

					//	argument 2: gain
					ASSERT_ARG_REAL_SCALAR(2, TYPE_DOUBLE);
					DOUBLE gain = ((DOUBLE*)ecf->arguments[2]->real)[0];

					//	argument 3: offset
					ASSERT_ARG_REAL_SCALAR(3, TYPE_DOUBLE);
					DOUBLE offset = ((DOUBLE*)ecf->arguments[3]->real)[0];

					//	argument 0: type is implied by its type
					//	we insist on a real (column or row) vector so that we can accept any values of TYPE_ORDER_MASK and TYPE_CPXFMT_MASK
					TYPE type = ecf->arguments[0]->type;
					UINT32 count2 = getNumberOfElements(ecf->arguments[0]->dims);
					if ((type & TYPE_EL_MASK) == (TYPE_DOUBLE | TYPE_REAL))
					{
						//	if passed argument was empty, create response locally and repoint argument
						if (!count2)
						{
							//	create locally and redirect arg
							cache.resize(count * sizeof(DOUBLE));
							generator->fill((DOUBLE*)&cache[0], count, gain, offset);
							ecf->arguments[0]->real = &cache[0];
							ecf->arguments[0]->imag = NULL;
							ecf->arguments[0]->dims.count = 1;
							cachedims = count;
							ecf->arguments[0]->dims.dims = &cachedims;
						}

						//	if the right number of elements, no need to recreate locally
						else if (count2 == count)
						{
							ASSERT_ARG_REAL_VECTOR(0, TYPE_DOUBLE);

							//	just overwrite
							DOUBLE* dst = (DOUBLE*)ecf->arguments[0]->real;
							generator->fill(dst, count, gain, offset);
						}

						//	otherwise, this won't work
						else
						{
							ecf->argumentCount = 0;
							berr << E_BAD_ARG_SIZE;
						}
					}
					else if ((type & TYPE_EL_MASK) == (TYPE_SINGLE | TYPE_REAL))
					{
						//	if passed argument was empty, create response locally and repoint argument
						if (!count2)
						{
							//	create locally and redirect arg
							cache.resize(count * sizeof(SINGLE));
							generator->fill((SINGLE*)&cache[0], count, gain, offset);
							ecf->arguments[0]->real = &cache[0];
							ecf->arguments[0]->imag = NULL;
							ecf->arguments[0]->dims.count = 1;
							cachedims = count;
							ecf->arguments[0]->dims.dims = &cachedims;
						}

						//	if the right number of elements in passed argument, just fill it
						else if (count2 == count)
						{
							ASSERT_ARG_REAL_VECTOR(0, TYPE_DOUBLE);

							//	just overwrite
							SINGLE* dst = (SINGLE*)ecf->arguments[0]->real;
							generator->fill(dst, count, gain, offset);
						}

						//	otherwise, this won't work
						else
						{
							ecf->argumentCount = 0;
							berr << E_BAD_ARG_SIZE;
						}
					}
					else
					{
						ecf->argumentCount = 0;
						return E_BAD_ARG_TYPE;
					}

					//	mark modified arguments
					ecf->arguments[0]->flags |= F_MODIFIED;

					//	ok
					return C_OK;
				}

				case GENERIC_FUNCTION_GET:
				{
					ENSURE_EXISTS;

					//	arg count
					ASSERT_ARG_COUNT(1);

					//	argument 0: must be real double
					TYPE type = ecf->arguments[0]->type;
					if ((type & TYPE_EL_MASK) != (TYPE_DOUBLE | TYPE_REAL))
					{
						ecf->argumentCount = 0;
						return E_BAD_ARG_TYPE;
					}

					//	argument 0: must be scalar or empty (for recreate)
					Dims dims(ecf->arguments[0]->dims);
					UINT32 count = dims.getNumberOfElements();

					//	if passed argument was empty, create response locally and repoint argument
					if (!count)
					{
						//	create locally and redirect arg
						double_ret = generator->get();
						ecf->arguments[0]->real = &double_ret;
						ecf->arguments[0]->dims.count = 1;
						cachedims = 1;
						ecf->arguments[0]->dims.dims = &cachedims;
					}

					//	if the right number of elements in passed argument, just fill it
					else if (count == 1)
					{
						//	just overwrite
						DOUBLE* dst = (DOUBLE*)ecf->arguments[0]->real;
						*dst = generator->get();
					}

					//	otherwise, this won't work
					else
					{
						ecf->argumentCount = 0;
						berr << E_BAD_ARG_SIZE;
					}

					//	mark modified arguments
					ecf->arguments[0]->flags |= F_MODIFIED;

					//	ok
					return C_OK;
				}

				default:
				{
					return E_FUNC_NOT_FOUND;
				}

			}

		}


	}


	return S_NULL;

}




#include "brahms-1199.h"

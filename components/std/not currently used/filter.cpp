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

	$Id:: filter.cpp 1751 2009-04-01 23:36:40Z benjmitch       $
	$Rev:: 1751                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-04-02 00:36:40 +0100 (Thu, 02 Apr 2009)       $
________________________________________________________________

*/



#include "group.h"




////////////////	COMPONENT INFORMATION

//	process information
#define COMPONENT_FLAGS		( F_NOT_RATE_CHANGER | F_NEEDS_ALL_INPUTS )





////////////////	COMPONENT CLASS

class ____CLASS_CPP____ : public Process
{



////////////////	REQUIRED PUBLIC OVERRIDES

public:


	
////////////////	OVERRIDES

	

	void event(Event& event);
	bool			step();



////////////////	MEMBERS

private:

	struct FILTER
	{
		VDOUBLE		coeffs;			//	coefficients (by column)
		UINT32				L;				//	number of lags
		UINT32				queuePointer;	// current position of first lag measured in samples (rolling queue in filter storage)
		DOUBLE*				laggedInputs;	// pointers to regions in workspace that belong to each filter
		DOUBLE*				laggedOutputs;	// pointers to regions in workspace that belong to each filter
	};

	//	filters
	vector<FILTER>			filters;

	//	input information
	UINT32					input_numels;

	//	workspace
	VDOUBLE			workspace;

	//	handle to input
	Symbol					hInput;
	vector<Symbol>			hOutputs;
};







////////////////	OPERATION METHODS

void ____CLASS_CPP____::event(Event& event)
{
	

	switch(event.type)
	{
	case EVENT_RUN_SERVICE:

		step();
	event.response = RESPONSE_OK;
	return;





		case EVENT_STATE_SET:
		{
			//	expect our init data as DataML
			DataMLNode nodePars(event.xmlNode);

			DataMLNode nodeFilters = nodePars.getField("filters");
			UINT32 numFilters = nodeFilters.getNumberOfElementsReal();

			//	extract individual filters
			for (UINT32 f=0; f<numFilters; f++)
			{
				//	extract filter
				FILTER filter;
				DataMLNode nodeFilter = nodeFilters.getCell(f);
				filter.coeffs = nodeFilter.validate(Dims(2, DIM_NONZERO)).getArrayDOUBLE();

				//	check is valid and get length
				if (filter.coeffs[1] != 1.0) berr << "all filters must have unity a0 element";
				filter.L = filter.coeffs.size() / 2 - 1;
				filter.queuePointer = 0;

				//	store
				filters.push_back(filter);
			}

			//	ok
			event.response = RESPONSE_OK;
			return;
		}

		case EVENT_INIT_PRECONNECT:
		{
			//	make sure we got no inputs
			if (iif.getNumberOfPorts() != 1)
				berr << "expects one input";

			//	ok
			event.response = RESPONSE_OK;
			return;
		}

		case EVENT_INIT_CONNECT:
		{
			//	validate input - must be double-type, but number of dimensions and size don't matter
			hInput = iif.getPort(0);
			Symbol hData = iif.getData(hInput);
			assertClass(hData, "std/2009/data/numeric");

			Structure* structure = numeric_get_structure(hData);

			UINT32 dataType = structure->typeElement;
			bool input_complex = structure->complex;
			Dims input_dims = structure->dims;

			if (dataType != TYPE_DOUBLE) berr << "filter only accepts double type input";
			input_numels = input_complex ? 2 : 1;
			for(UINT32 i=0; i<input_dims.size(); i++) input_numels *= input_dims[i];
			if (!input_dims.size()) input_numels = 0;

			//	create workspace for filters all in one vector
			UINT32 size = 0;
			for(UINT32 f=0; f<filters.size(); f++)
				size += filters[f].L * 2 * input_numels;
			workspace.resize(size);

			//	get pointers into it
			size = 0;
			for(UINT32 f=0; f<filters.size(); f++)
			{
				/*

				post R477

				with new compiler on windows, CL v14, it now causes an abort
				if we try to obtain the address of the first element of an
				empty vector, and workspace can be empty if no filter has a
				non-zero L. we fix this by loading NULLs in this case. actually,
				i don't know if it will abort if i try to obtain the address
				of the first absent element, so i'll cover this case too just in
				case.

				OLD LINES WERE LIKE THIS:
				filters[f].laggedInputs = &workspace[size];

				*/

				filters[f].laggedInputs = size < workspace.size() ? &workspace[size] : NULL;
				size += filters[f].L * input_numels;
				filters[f].laggedOutputs = size < workspace.size() ? &workspace[size] : NULL;
				size += filters[f].L * input_numels;
			}

			//	instantiate outputs
			for(UINT32 f=0; f<filters.size(); f++)
			{
				//	output is exactly the same as input
				Symbol hOutput = oif.addPort(hData);
				oif.setPortName(hOutput, getComponentData(hComponent, hData).name);
				hOutputs.push_back(hOutput);
			}

			//	ok
			event.response = RESPONSE_OK;
			return;
		}

	}

}



////////////////	STEP()

bool ____CLASS_CPP____::step()
{
	

	Symbol input = iif.getData(hInput);

	//	get pointer to input
	double* p_input = (double*) numeric_get_content(input);

	//	foreach filter
	for (UINT32 f=0; f<filters.size(); f++)
	{
		//	get pointers etc
		VDOUBLE output(input_numels);
		double* coeffs = &filters[f].coeffs[0];
		UINT32 L = filters[f].L;

		//	add b0 term
		for (UINT32 i=0; i<input_numels; i++) output[i] += coeffs[0] * p_input[i];

		//	add other b terms
		UINT32 rolling_queue_pointer = filters[f].queuePointer;
		double* p_lagged_inputs = filters[f].laggedInputs;
		double* p;
		for (UINT32 b=1; b<=L; b++)
		{
			p = p_lagged_inputs + rolling_queue_pointer * input_numels;
			for (UINT32 i=0; i<input_numels; i++) output[i] += coeffs[b*2] * p[i];
			if (rolling_queue_pointer) rolling_queue_pointer--;
			else rolling_queue_pointer = L-1;
		}

		//	subtract other a terms
		rolling_queue_pointer = filters[f].queuePointer;
		double* p_lagged_outputs = filters[f].laggedOutputs;
		for (UINT32 b=1; b<=L; b++)
		{
			p = p_lagged_outputs + rolling_queue_pointer * input_numels;
			for (UINT32 i=0; i<input_numels; i++) output[i] -= coeffs[b*2+1] * p[i];
			if (rolling_queue_pointer) rolling_queue_pointer--;
			else rolling_queue_pointer = L-1;
		}

		//	if any lags at all, roll the queues
		if (L)
		{
			//	advance rolling queue pointer
			if (++filters[f].queuePointer == L) filters[f].queuePointer = 0;

			//	roll lagged inputs
			p = p_lagged_inputs + filters[f].queuePointer * input_numels;
			for (UINT32 i=0; i<input_numels; i++) p[i] = p_input[i];

			//	roll lagged outputs
			p = p_lagged_outputs + filters[f].queuePointer * input_numels;
			for (UINT32 i=0; i<input_numels; i++) p[i] = output[i];
		}

		//	store output
		Symbol outputobject = oif.getData(hOutputs[f]);
		numeric_set_content(outputobject, &output[0]);
	}

	
	//ok
	return true;

	
}



#include "brahms-1065.h"



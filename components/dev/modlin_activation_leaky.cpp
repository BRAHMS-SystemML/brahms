/*
________________________________________________________________

	This file is part of BRAHMS
	Copyright (C) 2007 Ben Mitch(inson)
	URL: http://sourceforge.net/projects/abrg-brahms

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

	$Id:: modlin_activation_leaky.cpp 2419 2009-11-30 18:33:48#$
	$Rev:: 2419                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)       $
________________________________________________________________

*/



/*

	I keep this just as an example of a 1065 process :)

*/




//	group data (template group)
//	common data
#define __GROUP__ dev/jon/modlin/activation
#define __RELEASE__ 0
#define __REVISION__ 42
#define COMPONENT_CLASS_CPP dev_modlin_leaky_0
#define COMPONENT_CLASS_STRING "dev/modlin/leaky"

//	common groups
#define __clear(what) memset(&what, 0, sizeof(what))

//	define CORE_EXPORT for import
#ifdef _MSC_VER
#define CORE_EXPORT __declspec(dllimport)
#endif

//	use MATML
#define MATML

//	overlay (1)
#include "brahms-1065.h"
using namespace std;
using namespace brahms;

//	set additional info
#define MODULE_COMPONENT_ADDITIONAL "Author=Alex Cope & Jon Chambers\n"

//	process includes
#ifdef __PROC__
#include "dev/std/data/numeric/brahms/0/data.h"
#include "dev/std/data/spikes/brahms/0/data.h"
#include "dev/std/util/rng/brahms/0/utility.h"
#endif

//	STL includes
#include <ctime>
#include <cmath>
#include <sstream>
#include <iostream>

//	per-module version information
const ComponentVersion MODULE_VERSION_COMPONENT = {__RELEASE__, __REVISION__};

//	process information
#define MODULE_COMPONENT_FLAGS			( FLAG_NOT_RATE_CHANGER ) 

#define SINGLE_INF			(1.0e38)
#define SINGLE_ISINF(v)		(v >= 1.0e37)

/* ################################################################## */
//	DEFINE OPERATION TYPES AND TOTAL OPS
#define ADD			0
#define SHUNT			1
#define SHUNT_DEV		2

/* ################################################################## */

class COMPONENT_CLASS_CPP : public Process
{



////////////////	'STRUCTORS

public:

	COMPONENT_CLASS_CPP();
	~COMPONENT_CLASS_CPP();




////////////////	EVENT
	
	void event(Event& event);




////////////////	MEMBERS, THE PRIVATE DATA STORE OF THE CLASS

public:

	//	my state variables
	
	Dims			dims;
	
	UINT32			count;
	VSINGLE			tau_membrane;
	VSINGLE			sigma_membrane;
	VSINGLE			lambda_membrane;
	VSINGLE			lambda_membrane_reciprocal;
	VSINGLE			p_v;
	VSINGLE			pos_reversal_potential;
	VSINGLE			neg_reversal_potential;
	
	VDOUBLE			membrane;
	VUINT32			numPorts;
	
	Symbol			rng;
		
	Symbol				hOutput;
	vector<vector <Symbol> >	hInputs;
	
	//	INPUT SET HANDLES
	vector<Symbol>		hSets;



};


////////////////	'STRUCTORS

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP()
{
	//rng = NULL;
}

COMPONENT_CLASS_CPP::~COMPONENT_CLASS_CPP()
{
}





////////////////	OPERATION METHODS

void COMPONENT_CLASS_CPP::event(Event& event)
{
	switch(event.type)
	{
		// Stepping event comes first
		case EVENT_RUN_SERVICE:
		{
			// create data structures
			DOUBLE* idata;
			vector<VDOUBLE> totals;
			VDOUBLE neg_total;
			Symbol hInput;
			VDOUBLE odata;
			odata.resize(count, 0);
			totals.resize(hSets.size());
			for (UINT32 i=0; i<totals.size(); i++) {
				totals[i].resize(count,0);
			}
			neg_total.resize(count,0);
			
			// get noise
			VDOUBLE noise_membrane;
			noise_membrane.resize(count);
			for (UINT32 i=0; i < count; i++) {
				// decay membrane
				membrane[i] *= lambda_membrane[i];
			}
			
			dev_std_util_rng::fill(hComponent,rng, &noise_membrane[0], count, 1.0, 0.0);
			
			
			// get data and assign based on operation
			for (UINT32 i=0; i < hInputs.size(); i++) {
				for (UINT32 j=0; j < numPorts[i]; j++) {
					Symbol input = iif.getData(hInputs[i][j]);
					// make up the totals according to the operation types
					idata = (DOUBLE*)numeric_get_content(input);
					for (UINT32 k=0; k < count; k++) {
						if (i == ADD && idata[k] < 0) {neg_total[k] += idata[k];}
						else {totals[i][k] += idata[k];}
						// don't let shunt be negative!
						if (i == SHUNT && j == (hInputs[i].size() - 1) && totals[i][k] < -p_v[k]) {
							totals[i][k] = -p_v[k];
						}
					}
				}
			}
			
			// do it
			for (UINT32 i=0; i < count; i++) {
				if (!SINGLE_ISINF(pos_reversal_potential[i])) {
					totals[ADD][i] *= (pos_reversal_potential[i] - membrane[i]);
				}
				if (!SINGLE_ISINF(neg_reversal_potential[i])) {
					neg_total[i] *= (membrane[i] - neg_reversal_potential[i]);
				}
				totals[ADD][i] += neg_total[i];
				
				/* ####################################################### */
				// combine operation types

				odata[i] = totals[ADD][i];
				
				// Only do other operations if we have the ports
				if (hInputs[SHUNT].size() > 0) odata[i] *= (p_v[i] + totals[SHUNT][i]);

				if (hInputs[SHUNT_DEV].size() > 0) odata[i] /= (p_v[i] + totals[SHUNT_DEV][i]);

				//
				/* ####################################################### */
				
				odata[i] *=  lambda_membrane_reciprocal[i];
				membrane[i] += odata[i];
				membrane[i] += noise_membrane[i] * sigma_membrane[i];
			}
	

			numeric_set_content(oif.getData(hOutput), &membrane[0]);
			
			event.response = C_OK;
			return;
		}

		case EVENT_INIT_CONNECT:
		{
			// open output port with dimensions given by the state data
			if (event.flags & F_FIRST_CALL)
			{
			
				hOutput = oif.addPort("dev/std/data/numeric", 0);
				oif.setPortName(hOutput, "out");
				Symbol out = oif.getData(hOutput);
				numeric_set_structure(out, TYPE_DOUBLE | TYPE_REAL, dims);
				
			}
			
			// check inputs for veracity...
			if (event.flags & F_LAST_CALL)
			{
				Symbol hInput;
				for (UINT32 i=0; i<iif.getNumberOfPorts(); i++) {
					hInput = (iif.getPort(i));
					/*Symbol data = iif.getData(hInput);
					assertClass(data, "dev/std/data/numeric");
   					numeric_validate(data, TYPE_DOUBLE);*/
				}
			}

			
			// check input dimensions against state data
				/*Dims srcDims = cppdims(structure);
			if (srcDims[0] != src.dims[0] || srcDims[1] != src.dims[1]) {berr << "Source dimensions of input do not match dims";}
			}*/

			event.response = C_OK;
			return;
		}
		
		
		case EVENT_INIT_POSTCONNECT:
		{	
			// get the ports for the different sets
			hInputs.resize(hSets.size());
			for (UINT32 i=0; i < hSets.size(); i++) {
				numPorts.push_back(iif.getNumberOfPorts(hSets[i]));
				for (UINT32 j=0; j < numPorts[i]; j++) {
					hInputs[i].push_back(iif.getPort(hSets[i], j));
				}
			}
			// assume ports on the default set are addition and add them to that list
			numPorts[0] += iif.getNumberOfPorts();
			for (UINT32 i=0; i < iif.getNumberOfPorts(); i++) {
				hInputs[0].push_back(iif.getPort(i));
			}
			
			event.response = C_OK;
			return;
			
		}
		
		
		case EVENT_STATE_SET:
		{
			/* ######################################################### */
			// create operation sets
			hSets.push_back(iif.getSet("add"));
			hSets.push_back(iif.getSet("shunt"));
			hSets.push_back(iif.getSet("shunt_dev"));
				
			/* ######################################################### */
			
			//	create and seed RNG
			rng = coreCreateUtility(hComponent, "dev/std/util/rng", 0);
			dev_std_util_rng::select(hComponent, rng, "MT2000.normal");
			dev_std_util_rng::seed(hComponent, rng, &(*event.state.seed)[0], event.state.seed->size());
			
			/*
				Extracting the data from the state MatML file - reality checking as we go...
			*/
			
			
			MatMLNode nodePars(event.xmlNode);
			
			// get dimensions
			VUINT64 dims_temp = nodePars.getField("dims").getArrayUINT64();
			for (UINT32 i=0; i < dims_temp.size(); i++) {
				dims.push_back(dims_temp[i]);
			}
			
			count = dims.getNumberOfElements();
			
			membrane.resize(count, 0);
			
			// get the args
			if (nodePars.hasField("tau_membrane")) {
				tau_membrane = nodePars.getField("tau_membrane").getArraySINGLE();
			}
			UINT32 N = tau_membrane.size();
			
			if (nodePars.hasField("sigma_membrane")) {
				sigma_membrane = nodePars.getField("sigma_membrane").validate(Dims(N)).getArraySINGLE();
			}
			
			if (nodePars.hasField("p")) {
				p_v = nodePars.getField("p").validate(Dims(N)).getArraySINGLE();
			}
			
			if (nodePars.hasField("pos_reversal_potential")) {
				pos_reversal_potential = nodePars.getField("pos_reversal_potential").validate(Dims(N)).getArraySINGLE();
			}
			
			if (nodePars.hasField("neg_reversal_potential")) {
				neg_reversal_potential = nodePars.getField("neg_reversal_potential").validate(Dims(N)).getArraySINGLE();
			}
			
			lambda_membrane.resize(count);
			lambda_membrane_reciprocal.resize(count);
			if (N == 1) {
				tau_membrane.resize(count, tau_membrane[0]);
				sigma_membrane.resize(count, sigma_membrane[0]);
				p_v.resize(count, p_v[0]);
				pos_reversal_potential.resize(count, pos_reversal_potential[0]);
				neg_reversal_potential.resize(count, neg_reversal_potential[0]);
			}
			else if (N != count) {
				berr << "pars are of incorrect size!";
			}
			
			for (UINT32 i=0; i<count; i++) {
				lambda_membrane[i] = exp(-1.0 / (tau_membrane[i] * sampleRateToRate(time.sampleRate)));
				lambda_membrane_reciprocal[i] = 1.0 - lambda_membrane[i];
			}
			
			
			//	ok
			event.response = C_OK;
			return;
		}


//#include "events.cpp"


	}

	//	raise an exception if you run into trouble during event(). the return value
	//	indicates whether you processed the event.


	
}

#include "brahms-1065.h"



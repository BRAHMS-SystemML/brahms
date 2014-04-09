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

	$Id:: component.cpp 2406 2009-11-19 20:53:19Z benjmitch    $
	$Rev:: 2406                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-19 20:53:19 +0000 (Thu, 19 Nov 2009)       $
________________________________________________________________

*/

/* CUT HERE */

/*

	This is a newly created BRAHMS Process. It is a native
	process, and needs to be built before BRAHMS can run it.
	In Matlab, run "brahms_manager", select this component,
	and click on "Build". After doing this, you can build it
	in future using "brahms_utils".

*/



////////////////	COMPONENT INFO

//	define your component information here. if you use the BRAHMS
//	Manager to create your process, it will insert sensible defaults.
#define COMPONENT_CLASS_STRING __TEMPLATE_CLASS_STRING_C__
#define COMPONENT_CLASS_CPP __TEMPLATE__CLASS_CPP__
#define COMPONENT_RELEASE __TEMPLATE__RELEASE_INT__
#define COMPONENT_REVISION __TEMPLATE__REVISION_INT__
#define COMPONENT_ADDITIONAL __TEMPLATE__ADDITIONAL_C__
#define COMPONENT_FLAGS __TEMPLATE__FLAGS_C__

//	we define this symbol to ask the template to include the basics
//	we usually need to build a process. it will import the "brahms"
//	namespace and include the header files (SDK) for the data and util
//	classes from the Standard Library.
#define OVERLAY_QUICKSTART_PROCESS

//	include the component interface overlay (component bindings 1199)
#include "brahms-1199.h"

/* EXAMPLE

//	alias data and util namespaces to something briefer
namespace numeric = std_2009_data_numeric_0;
namespace spikes = std_2009_data_spikes_0;
namespace rng = std_2009_util_rng_0;

*/



////////////////	COMPONENT CLASS (DERIVES FROM Process)

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	use ctor/dtor only if required
	COMPONENT_CLASS_CPP() {}
	~COMPONENT_CLASS_CPP() {}

	//	the framework event function
	Symbol event(Event* event);

private:

	/*

		Private member data of the process goes here.
		You can use this to store your parameters
		and process state between calls to event().

	*/

/* EXAMPLE

	//	an input and an output port
	numeric::Input input;
	numeric::Output output;

	//	RNG utility
	rng::Utility random;

*/

};



////////////////	EVENT

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	switch(event->type)
	{
		case EVENT_STATE_SET:
		{
			//	extract DataML
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodeState(&xmlNode);

/* EXAMPLE

			//	extract a scalar and a matrix of real doubles of size 8x3
			UINT32 myParameter = nodeState.getField("myParameter").getUINT32();
			VDOUBLE myInitData = nodeState.getField("myInitData").validate(TYPE_DOUBLE | TYPE_REAL, Dims(8, 3)).getArrayDOUBLE();

			//	obtain and initialise a RNG from std/2009/util/rng
			VUINT32 seed = getRandomSeed(1);
			random.create(hComponent);
			random.select("MT2000.normal");
			random.seed(&seed[0], 1);

*/

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	on first call
			if (event->flags & F_FIRST_CALL)
			{

/* EXAMPLE
				
				//	validate that exactly one input was provided
				if (iif.getNumberOfPorts() != 1)
					berr << "expected 1 input";

				//	create one output, real scalar DOUBLE
				output.setName("myOutput");
				output.create(hComponent);
				output.setStructure(TYPE_REAL | TYPE_DOUBLE, Dims(1).cdims());

*/

			}

			//	on last call
			if (event->flags & F_LAST_CALL)
			{

/* EXAMPLE

				//	validate that the sole input is called "myInput", is data/numeric
				//	class, and is real 4x2 UINT32.
				input.attach(hComponent, "myInput");
				input.validateStructure(TYPE_REAL | TYPE_UINT32, Dims(4, 2).cdims());

*/

			}

			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{

/* EXAMPLE
			
			//	get data object on input port and access its content
			UINT32* in = (UINT32*) input.getContent();

			//	get a single value from RNG (note that the call for multiple
			//	samples is more efficient - google "std_util_rng")
			DOUBLE rand = random.get();

			//	get data object on output port and write its content
			DOUBLE* out = (DOUBLE*) output.getContent();
			*out = rand * in[0];

*/

			//	ok
			return C_OK;
		}

	}

	//	if we service the event, we return C_OK
	//	if we don't, we should return S_NULL to indicate that we didn't
	return S_NULL;
}







//	include the second part of the overlay (it knows you've included it once already)
#include "brahms-1199.h"


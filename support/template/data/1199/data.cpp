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

	$Id:: data.cpp 2435 2009-12-12 16:35:44Z benjmitch         $
	$Rev:: 2435                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-12 16:35:44 +0000 (Sat, 12 Dec 2009)       $
________________________________________________________________

*/


/*

	THIS TEMPLATE IS SORELY OUT OF DATE 12/12/09

*/



/*

	NOTES

	* You should rename the source file to <CLASS_US>.cpp
	before you compile it. This way, when an error occurs,
	the filename reported in the error message will be
	meaningful. This template filename, for instance, would
	not help you find the problem.

	* We name the CPP class as the namespace (i.e. the
	SystemML class with slashes replaced by underscores),
	but suffixed with the release number. This ensures all
	SystemML component releases have unique class names,
	which facilitates linking on systems that use the class
	name as part of the link process. Here, we do this by
	defining COMPONENT_CLASS_CPP in the makefile.

	* Note that we include the overlay twice - once at the
	top of the file and again at the bottom. Different
	content is included each time, and both are necessary.

*/



////////////////	PER-MODULE INFO (USED BY OVERLAY)

//	COMPONENT_CLASS_CPP is defined in the makefile
//	COMPONENT_CLASS_STRING is defined in the makefile

//	component type
#define COMPONENT_DATA

//	component additional info
#define COMPONENT_ADDITIONAL "Author=Ben Mitch\nURL=http://brahms.sourceforge.net\n"

//	component flags
#define COMPONENT_FLAGS (0)



////////////////	INCLUDES

//	include the overlay
#include "brahms-1199.h"

//	additional STL includes
#include <ctime>
#include <cmath>
#include <sstream>
#include <iostream>
#include <fstream>
#include <limits>

//	import namespaces
using namespace std;
using namespace brahms;

//	include our own header
#include "data.h"

//	and import its namespace
using namespace ____CLASS_US____;



////////////////	PER-MODULE DATA

//	component version
const struct ComponentVersion COMPONENT_VERSION =
{
	,
	__RELEASE__,
	__REVISION__
};



////////////////	HELPER FUNCTIONS

//	handy clear macro
#define ____CLEAR(what) memset(&what, 0, sizeof(what))

//	number to string conversion
inline string n2s(UINT32 n)
{
	ostringstream s;
	s << n;
	return s.str();
}



////////////////	CLASS DECLARATION

class COMPONENT_CLASS_CPP : public Data
{

public:
	
	//	You may or may not need ctor & dtor for your data class - if you
	//	don't need them, you can leave them out completely.

	COMPONENT_CLASS_CPP();
	~COMPONENT_CLASS_CPP();

	//	You *will* generally need the copy ctor, since you will use an
	//	"ofstream" object to maintain the output file (used if this object
	//	is logging to file), and this cannot be copied implicitly.

	COMPONENT_CLASS_CPP(const COMPONENT_CLASS_CPP& src);



////////////////	EVENT

	//	You must define the event member function - you need not define
	//	any additional member functions: all interaction with the framework
	//	will come through this member, and all interaction with processes
	//	*can* come through this member (though you are not prevented from
	//	creating other member functions for private use by your client
	//	Processes).

	Symbol event(Event* event);




////////////////	COMPONENT INTERFACE

public:

	//	If you want to provide additional member functions for private use
	//	by your client processes, declare them here. Don't do this unless
	//	you have good reason (i.e. optimisation).



////////////////	MEMBERS

private:

	//	Internal state of the data object, equals "structure" (that which
	//	does not change during an execution) and "content" (that which does
	//	change)

	VehicleStructure structure;
	VehicleContent content;
	VDOUBLE vcontent;

	//	Logging data

	vector<VehicleContent> log;
	ofstream outfile;

	//	Generic Interface return data

	string						eas_str;
	EventGenericStructure		eas;
	Dims						eaf_dims;
	EventGenericForm				eaf;
	EventGenericContent			eac;

};









////////////////	CTOR & DTOR

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP()
{
	____CLEAR(structure);
	____CLEAR(content);
}

COMPONENT_CLASS_CPP::~COMPONENT_CLASS_CPP()
{
	//	Close output file if open
	if (outfile.is_open()) outfile.close();
}

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP(const COMPONENT_CLASS_CPP& src)
{
	//	Copy state
	vcontent = src.vcontent;
	structure = src.structure;
	content.wheelSpeed = vcontent.size() ? &vcontent[0] : NULL;

	//	Don't copy outfile, because if it is active, *only* the 0th data object needs access to it
}






////////////////	EVENT

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	

	switch(event->type)
	{



////////////////	PRIVATE DATA INTERFACE (see documentation page "Events")

		case EVENT_GET_STRUCTURE:
		{
			//	return structure to caller
			event->data = &structure;

			//	ok
			return C_OK;
		}

		case EVENT_SET_STRUCTURE:
		{
			//	set structure from caller
			structure = *((VehicleStructure*)event->data);

			//	update content storage
			vcontent.resize(structure.numberOfWheels);
			content.wheelSpeed = vcontent.size() ? &vcontent[0] : NULL;

			//	ok
			return C_OK;
		}

		case EVENT_GET_CONTENT:
		{
			//	return content to caller
			event->data = &content;

			//	ok
			return C_OK;
		}

		case EVENT_SET_CONTENT:
		{
			//	set content from caller
			VehicleContent* newcontent = (VehicleContent*)event->data;
			memcpy(content.wheelSpeed, newcontent->wheelSpeed, structure.numberOfWheels * sizeof(DOUBLE));

			//	ok
			return C_OK;
		}

		case EVENT_GET_CONTENT_DIRECT:
		{
			//	return content to caller
			event->data = content.wheelSpeed;

			//	ok
			return C_OK;
		}

		case EVENT_SET_CONTENT_DIRECT:
		{
			//	set content from caller
			DOUBLE* newcontent = (DOUBLE*)event->data;
			memcpy(content.wheelSpeed, newcontent, structure.numberOfWheels * sizeof(DOUBLE));

			//	ok
			return C_OK;
		}



////////////////	GENERIC DATA INTERFACE (see documentation page "Events")

		case EVENT_GENERIC_STRUCTURE_GET:
		{
			//	return structure as a string
			ostringstream ss;
			ss << structure.numberOfWheels;
			eas_str = ss.str();
			eas.structure = eas_str.c_str();
			event->data = &eas;

			//	ok
			return C_OK;
		}

		case EVENT_GENERIC_STRUCTURE_SET:
		{
			//	set structure from a string
			string struc((const char*)event->data);
			istringstream ss;
			ss.str() = struc;
			ss >> structure.numberOfWheels;

			//	update content storage
			vcontent.resize(structure.numberOfWheels);
			content.wheelSpeed = vcontent.size() ? &vcontent[0] : NULL;

			//	ok
			return C_OK;
		}

		case EVENT_GENERIC_FORM_ADVANCE:
		{
			//	return structure of equivalent numeric form
			eaf.type = TYPE_DOUBLE | TYPE_REAL;
			eaf_dims = Dims(structure.numberOfWheels);
			eaf.dims = eaf_dims.cdims();
			eaf.form = C_FORM_FIXED;
			event->data = &eaf;

			//	ok
			return C_OK;
		}

		//case EVENT_GENERIC_FORM_CURRENT:
		//{
			//	There is no need to service this event if you return C_FORM_FIXED
			//	from EVENT_GENERIC_FORM_ADVANCE. That is, if the structure of your generic
			//	numeric form doesn't change as the state content changes, there is no
			//	need to update the structure using EVENT_GENERIC_FORM_CURRENT (this event
			//	will never be called).
		//}

		case EVENT_GENERIC_CONTENT_GET:
		{
			//	access.bytes should return how many bytes per real/imag
			//	access.real should return the real data (can be NULL if access.bytes is zero)
			//	access.imag should return the imag data if there is any
			eac.bytes = structure.numberOfWheels * sizeof(DOUBLE);
			eac.real = vcontent.size() ? &vcontent[0] : NULL;
			event->data = &eac;

			//	ok
			return C_OK;
		}

		case EVENT_GENERIC_CONTENT_SET:
		{
			//	access.bytes will be how many bytes per real/imag
			//	access.real will be the real data (can be NULL if udata is zero)
			//	access.imag will be the imag data if there is any
			EventGenericContent* eac = (EventGenericContent*) event->data;
			if (eac->bytes != (structure.numberOfWheels * sizeof(DOUBLE)))
				berr << "invalid set state (" << eac->bytes << " bytes supplied, expected " << (structure.numberOfWheels * sizeof(DOUBLE)) << ")";
			if (!eac->real && eac->bytes) berr << "invalid set state (no real data)";
			if (eac->imag) berr << "invalid set state (did not expect imag data)";
			if (eac->bytes) memcpy(content.wheelSpeed, eac->real, eac->bytes);

			//	ok
			return C_OK;
		}



////////////////	NATIVE COMPONENT INTERFACE (see documentation page "Events")

		case EVENT_STATE_SET:
		{
			EventStateSet* ess = (EventStateSet*) event->data;

			//	You may be called to set your state from a passed XMLNode, or
			//	just to set your state to one of a small set of generic states.
			//	If you are asked to set to a generic state, you should assume
			//	that structure does not change.

			//	Generic, "undefined" (interpretation up to you)

			if (ess->flags & F_UNDEFINED)
			{
				//	set everything to NaNs, perhaps
				DOUBLE undef = numeric_limits<DOUBLE>::quiet_NaN();
				for (UINT32 w=0; w<structure.numberOfWheels; w++)
					content.wheelSpeed[w] = undef;
			}

			//	Generic, "zero" (interpretation up to you)

			else if (ess->flags & F_ZERO)
			{
				//	set everything to zero
				for (UINT32 w=0; w<structure.numberOfWheels; w++)
					content.wheelSpeed[w] = 0.0;
			}

			//	Specific

			else
			{
				//	not implemented
				berr << "not implemented";
			}

			//	ok
			return C_OK;
		}

		case EVENT_LOG_INIT:
		{
			EventLog* el = (EventLog*) event->data;

			//	If storing encapsulated, you must output all your logged data into
			//	one XML node when requested during EVENT_LOG_TERM

			if (el->flags & F_ENCAPSULATED)
			{
				//	reserve memory for logging data
				UINT32 numSamples = time->executionStop / time->samplePeriod;
				log.reserve(numSamples);
			}

			//	If not logging encapsulated, you *may* log the bulk of the data
			//	to a file (for example) and only link it in the XML node you return.
			//	You are not *required* to offer non-encapsulated log - you can
			//	just log encapsulated and ignore the state of this flag, but it may
			//	be much faster to log non-encapsulated.

			else
			{
				//	open file
				outfile.open(el->filename, ios::out | ios::trunc | ios::binary);
				if (!outfile.is_open()) berr << "failed to open file \"" << el->filename << "\"";
			}

			//	ok
			return C_OK;
		}

		case EVENT_LOG_SERVICE:
		{
			EventLog* el = (EventLog*) event->data;

			//	When asked to service the log, you are passed a pointer to the
			//	object that was just written - that is, the object you should get
			//	content from into the log. The same object (the 0th) is always
			//	asked to actually *perform* the log operation.
			
			//	extract source object
			COMPONENT_CLASS_CPP* src = (COMPONENT_CLASS_CPP*)el->source;

			//	Perform log service operation

			if (el->flags & F_ENCAPSULATED)
			{
				//	to memory
				log.push_back(src->content);
			}

			else
			{
				//	to file
				for (UINT32 w=0; w<structure.numberOfWheels; w++)
					outfile.write((char*)&src->content.wheelSpeed[w], sizeof(DOUBLE));
			}

			//	ok
			return C_OK;
		}

		case EVENT_LOG_TERM:
		{
			EventLog* el = (EventLog*) event->data;

			//	create output node
			XMLNode xmlNode;
			el->result = xmlNode.element;

			//	When asked to terminate the log, you should return a single XML node
			//	that constitutes the complete log. If you were storing encapsulated, it
			//	should also encapsulate the complete log. If not, it may contain a filename
			//	as a pointer to a file containing data.

			if (el->flags & F_ENCAPSULATED)
			{
				//	construct return value
				stringstream ss;
				for (UINT32 i=0; i<log.size(); i++)
				{
					for (UINT32 w=0; w<structure.numberOfWheels; w++)
					{
						ss << log[i].wheelSpeed[w] << " ";
					}
				}

				//	write XML node
				xmlNode.setAttribute("numberOfWheels", n2s(structure.numberOfWheels).c_str());
				xmlNode.nodeText(ss.str().c_str());
			}

			else
			{
				//	already finished writing file
				outfile.close();

				//	write XML node
				string ss = n2s(structure.numberOfWheels);
				xmlNode.setAttribute("numberOfWheels", ss.c_str());
				xmlNode.nodeText(el->filename);
			}

			//	ok
			return C_OK;
		}

	}

	return S_NULL;
	
}






////////////////	INCLUDE OVERLAY

#include "brahms-1199.h"


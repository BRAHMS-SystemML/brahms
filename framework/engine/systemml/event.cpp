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

	$Id:: event.cpp 2376 2009-11-15 23:34:44Z benjmitch        $
	$Rev:: 2376                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-15 23:34:44 +0000 (Sun, 15 Nov 2009)       $
________________________________________________________________

*/



#include "systemml.h"


////////////////////////////////////////////////////////////////
//
//	START NAMESPACE
//
namespace brahms {
//
////////////////////////////////////////////////////////////////



EventEx::EventEx(
)
{
	//	base class
	type = EVENT_NULL;
	flags = 0;
	object = NULL;
	data = NULL;

	//	derived class
	component = NULL;
	requireResponse = true;
	tout = NULL;
}

EventEx::EventEx(
	Symbol p_type,
	UINT32 p_flags,
	brahms::systemml::Component* p_component,
	void* p_data
)
{
	//	base class
	type = p_type;
	flags = p_flags;
	object = p_component->getObject();
	data = p_data;

	//	derived class
	component = p_component;
	requireResponse = true;
	tout = NULL;
}

EventEx::EventEx(
	Symbol p_type,
	UINT32 p_flags,
	brahms::systemml::Component* p_component,
	void* p_data,
	bool p_requireResponse,
	brahms::output::Source* p_tout
)
{
	//	base class
	type = p_type;
	flags = p_flags;
	object = p_component->getObject();
	data = p_data;

	//	derived class
	component = p_component;
	requireResponse = p_requireResponse;
	tout = p_tout;
}



Symbol EventEx::fire()
{
	try
	{
		//	report
		if (tout)
		{
			string text = brahms::base::symbol2string(type) + string(" on ") + component->getObjectName();
			(*tout) << text << brahms::output::D_FULL;
		}

		//	fire event
		Symbol response = component->module->getHandler()(this);
		if (S_ERROR(response)) throw brahms::error::globalErrorRegister.pull(response);

		//	check response
		if (requireResponse && response == S_NULL)
		{
			string msg = "no response to event \"" + string(brahms::base::symbol2string(type)) + "\"";
			ferr << E_NOT_COMPLIANT << msg;
		}

		//	report
		if (tout)
		{
			(*tout) << "...OK" << brahms::output::D_FULL;
		}

		//	ok
		return response;
	}

	catch(brahms::error::Error& e)
	{
		e.trace(string("whilst firing ") + brahms::base::symbol2string(type) + string(" on ") + component->getObjectName());
		throw;
	}

	catch(std::exception ee)
	{
		brahms::error::Error e(E_STD, ee.what());
		e.trace(string("whilst firing ") + brahms::base::symbol2string(type) + string(" on ") + component->getObjectName());
		throw e;
	}

	catch(...)
	{
		brahms::error::Error e(E_UNRECOGNISED_EXCEPTION);
		e.trace(string("whilst firing ") + brahms::base::symbol2string(type) + string(" on ") + component->getObjectName());
		throw e;
	}
}



////////////////////////////////////////////////////////////////
//
//	END NAMESPACE
//
}
//
////////////////////////////////////////////////////////////////



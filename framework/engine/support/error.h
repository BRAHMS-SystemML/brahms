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

	$Id:: error.h 2328 2009-11-08 13:26:45Z benjmitch          $
	$Rev:: 2328                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-08 13:26:45 +0000 (Sun, 08 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_SUPPORT_ERROR
#define INCLUDED_BRAHMS_SUPPORT_ERROR



////////////////	NAMESPACE

namespace brahms
{
	namespace error
	{



////////////////	EXCEPTION REGISTER, FOR PROPAGATING EXCEPTIONS THROUGH C INTERFACES

		struct ErrorRegister
		{
			//	error register stores error objects whilst they
			//	are propagated through component modules being
			//	returned from API calls, and also whilst they are
			//	temporarily held by the engine client.
			list<brahms::error::Error> errors;
			brahms::os::Mutex mutex;

			//	exchange error object for a Symbol representing it (check in)
			Symbol push(brahms::error::Error& error);

			//	exchange returned Symbol for original error object (check out)
			brahms::error::Error pull(Symbol error);

			//	get temporary reference checked-in error object so that it can be modified
			brahms::error::Error& borrow(Symbol error);
		};

/*

	component interface functions are only ever called from the engine, nested as:

	client ->
		engine (client interface) ->
			component (event function) ->
				engine (component interface)
	
	when an error object occurs in this API, we store it in the globalErrorRegister, and
	return a Symbol to the component that represents it. in turn, the component
	passes it back through the return value of the event() function. at this point,
	we retrieve it from the globalErrorRegister, and throw it. thus, the register
	serves as temporary storage so that errors need not be thrown through the C
	interface. through this register, throwing an error in an API function results in
	the error being thrown "out of" the event() function, in effect, so it's fairly
	neat.

	NB: errors returned to the client are not treated in the same way. we simply return
	a generic failure code to the client, and add the error to the engine's error stack.
	the client can retrieve all of the stack after terminating the engine.

*/

		extern ErrorRegister globalErrorRegister;

		#define CATCH_TRACE_RETHROW(trace_msg) catch(brahms::error::Error& e) { e.trace(string(trace_msg).c_str()); throw; }



	}
}



////////////////	INCLUSION GUARD

#endif



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

	$Id:: constants.cpp 2329 2009-11-09 00:13:25Z benjmitch    $
	$Rev:: 2329                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-09 00:13:25 +0000 (Mon, 09 Nov 2009)       $
________________________________________________________________

*/






////////////////	NAMESPACE

namespace brahms
{
	namespace base
	{

		

////////////////	SYMBOL 2 STRING

		#define CASE(s) case s: return #s;

		const char* symbol2string(Symbol symbol)
		{
			//	if it's an error, zero the GUID
			if (S_ERROR(symbol))
				symbol = symbol & (~S_GUID_MASK);

			switch(symbol)
			{
				CASE(S_NULL)

				CASE(E_ERROR)
				CASE(E_USER)

				CASE(E_INTERNAL)
				CASE(E_MEMORY)
				CASE(E_NOT_FOUND)
				CASE(E_NOT_IMPLEMENTED)
				CASE(E_UNRECOGNISED_EXCEPTION)
				CASE(E_STD)
				CASE(E_REALITY_INVERSION)
				CASE(E_DATAML)

				CASE(E_NULL_ARG)
				CASE(E_INVALID_ARG)
				CASE(E_INVALID_HANDLE)
				CASE(E_WRONG_HANDLE_TYPE)
				CASE(E_WRONG_NEW_HANDLE_TYPE)
				CASE(E_NOT_AVAILABLE)
				CASE(E_OVERFLOW)
				CASE(E_INTERFACE_MISUSE)

				CASE(E_NO_INSTANCE)
				CASE(E_NOT_SERVICED)

				CASE(E_PORT_EMPTY)
				CASE(E_INVALID_INPUT)

				CASE(E_FUNC_NOT_FOUND)
				CASE(E_BAD_ARG_COUNT)
				CASE(E_BAD_ARG_TYPE)
				CASE(E_BAD_ARG_SIZE)

				CASE(C_OK)
				CASE(C_YES)
				CASE(C_NO)
				CASE(C_CANCEL)
				CASE(C_STOP_USER)
				CASE(C_STOP_EXTERNAL)
				CASE(C_STOP_CONDITION)
				CASE(C_STOP_THEREFOREIAM)
				CASE(C_FORM_FIXED)
				CASE(C_FORM_BOUNDED)
				CASE(C_FORM_FIXED_BUT_LAST)
				CASE(C_FORM_UNBOUNDED)

				CASE(E_THREAD_ERROR)
				CASE(E_COMMS)
				CASE(E_COMMS_TIMEOUT)
				CASE(E_INVOCATION)
				CASE(E_THREAD_HUNG)
				CASE(E_NOT_COMPLIANT)
				CASE(E_SYSTEM_FILE)
				CASE(E_EXECUTION_FILE)
				CASE(E_UNREPRESENTABLE)
				CASE(E_XML_PARSE)
				CASE(E_XML)
				CASE(E_OS)
				CASE(E_EXECUTION_PARAMETERS)
				CASE(E_NAMESPACE)
				CASE(E_SYSTEMML)
				CASE(E_INSTALLATION)
				CASE(E_ZLIB)
				CASE(E_DEADLOCK)
				CASE(E_INPUT_RATE_MISMATCH)
				CASE(E_OUTPUT_RATE_MISMATCH)
				CASE(E_MPI)
				CASE(E_PEER_ERROR)
				CASE(E_FAILED_LOAD_MODULE)
				CASE(E_CIRCULAR_REFERENCE)
				CASE(E_MATLAB)
				CASE(E_FAILED_START_MATLAB_ENGINE)
				CASE(E_UNRECOGNISED_INFO_FIELD)
				CASE(E_PYTHON)
				CASE(E_MODULE_NOT_FOUND)
				CASE(E_SYSTEM)
				CASE(E_SYNC_TIMEOUT)
				CASE(E_SEED_OVERFLOW)
				CASE(E_SEED_INVALID)
				CASE(E_VOICE_UNDEFINED)
				CASE(E_WRONG_COMPONENT_TYPE)
				CASE(E_DATA_TYPE_UNSUPPORTED)
				CASE(E_OUT_OF_ORDER_DELIVERY)

				CASE(EVENT_MODULE_INIT)
				CASE(EVENT_MODULE_CREATE)
				CASE(EVENT_MODULE_DESTROY)
				CASE(EVENT_MODULE_DUPLICATE)
				CASE(EVENT_MODULE_TERM)

				CASE(EVENT_BEGIN_RUNPHASE)
				CASE(EVENT_BEGIN_TERMPHASE)

				CASE(EVENT_INIT_PRECONNECT)
				CASE(EVENT_INIT_CONNECT)
				CASE(EVENT_INIT_POSTCONNECT)
				CASE(EVENT_INIT_COMPLETE)

				CASE(EVENT_RUN_PLAY)
				CASE(EVENT_RUN_RESUME)
				CASE(EVENT_RUN_SERVICE)
				CASE(EVENT_RUN_PAUSE)
				CASE(EVENT_RUN_STOP)

				CASE(EVENT_LOG_INIT)
				CASE(EVENT_LOG_SERVICE)
				CASE(EVENT_LOG_TERM)

				CASE(EVENT_STATE_SET)
				CASE(EVENT_STATE_GET)

				CASE(EVENT_CONTENT_SET)
				CASE(EVENT_CONTENT_GET)

				CASE(EVENT_GENERIC_STRUCTURE_GET)
				CASE(EVENT_GENERIC_STRUCTURE_SET)
				CASE(EVENT_GENERIC_FORM_ADVANCE)
				CASE(EVENT_GENERIC_FORM_CURRENT)
				CASE(EVENT_GENERIC_CONTENT_GET)
				CASE(EVENT_GENERIC_CONTENT_SET)

				CASE(EVENT_FUNCTION_GET)
				CASE(EVENT_FUNCTION_CALL)

				CASE(ENGINE_EVENT_SET_PORT_NAME)
				CASE(ENGINE_EVENT_SET_PORT_SAMPLE_RATE)
				CASE(ENGINE_EVENT_DATA_FROM_PORT)
				CASE(ENGINE_EVENT_ASSERT_COMPONENT_SPEC)
				CASE(ENGINE_EVENT_FIRE_EVENT_ON_DATA)
				CASE(ENGINE_EVENT_GET_PORT_ON_SET)
				CASE(ENGINE_EVENT_HANDLE_UTILITY_EVENT)

				default:
				{
					cerr << "symbol2string(0x" << hex << symbol << ") was called (unrecognised)\n";
					return "[SYMBOL_UNRECOGNISED]";
				}
			}
		}



////////////////	NAMESPACE

	}
}



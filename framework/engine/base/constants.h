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

	$Id:: constants.h 2361 2009-11-12 18:39:39Z benjmitch      $
	$Rev:: 2361                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-12 18:39:39 +0000 (Thu, 12 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_BASE_CONSTANTS
#define INCLUDED_BRAHMS_BASE_CONSTANTS



const INT32 PRECISION_DO_NOT_LOG = 0x40000001;

//	12 bits of symbol are used in error symbols to attach messages
//	note this is the same in 32-bit and 64-bit builds, currently,
//	though 64-bit builds have space for way more errors if necessary
const Symbol S_GUID_MASK = 0x0FFF0000;
const UINT32 S_GUID_OFFSET_BITS = 16;
const UINT32 S_GUID_MAX = 4095;

//	bindings-only events
const Symbol EVENT_BEGIN_RUNPHASE = C_BASE_ENGINE_EVENT + 0x0001;
const Symbol EVENT_BEGIN_TERMPHASE = C_BASE_ENGINE_EVENT + 0x0002;

//	large number to avoid overflowing double-precision
const DOUBLE TEN_TO_THE_FIFTEEN				= 1000000000000000.0;

//  errors
const Symbol E_THREAD_ERROR					= E_BASE_ENGINE + 0x0001;
const Symbol E_COMMS						= E_BASE_ENGINE + 0x0002;
const Symbol E_COMMS_TIMEOUT				= E_BASE_ENGINE + 0x0003;
const Symbol E_INVOCATION					= E_BASE_ENGINE + 0x0004;
const Symbol E_THREAD_HUNG					= E_BASE_ENGINE + 0x0005;
const Symbol E_NOT_COMPLIANT				= E_BASE_ENGINE + 0x0006;
const Symbol E_SYSTEM_FILE					= E_BASE_ENGINE + 0x0007;
const Symbol E_EXECUTION_FILE				= E_BASE_ENGINE + 0x0008;
const Symbol E_UNREPRESENTABLE				= E_BASE_ENGINE + 0x0009;
const Symbol E_XML_PARSE					= E_BASE_ENGINE + 0x000A;
const Symbol E_XML							= E_BASE_ENGINE + 0x000B;
const Symbol E_OS							= E_BASE_ENGINE + 0x000C;
const Symbol E_EXECUTION_PARAMETERS			= E_BASE_ENGINE + 0x000D;
const Symbol E_NAMESPACE					= E_BASE_ENGINE + 0x000E;
const Symbol E_SYSTEMML						= E_BASE_ENGINE + 0x000F;
const Symbol E_INSTALLATION					= E_BASE_ENGINE + 0x0010;
const Symbol E_ZLIB							= E_BASE_ENGINE + 0x0011;
const Symbol E_DEADLOCK						= E_BASE_ENGINE + 0x0012;
const Symbol E_INPUT_RATE_MISMATCH			= E_BASE_ENGINE + 0x0013;
const Symbol E_OUTPUT_RATE_MISMATCH			= E_BASE_ENGINE + 0x0014;
const Symbol E_MPI							= E_BASE_ENGINE + 0x0015;
const Symbol E_PEER_ERROR					= E_BASE_ENGINE + 0x0016;
const Symbol E_FAILED_LOAD_MODULE			= E_BASE_ENGINE + 0x0017;
const Symbol E_CIRCULAR_REFERENCE			= E_BASE_ENGINE + 0x0018;
const Symbol E_MATLAB						= E_BASE_ENGINE + 0x0019;
const Symbol E_FAILED_START_MATLAB_ENGINE	= E_BASE_ENGINE + 0x001A;
const Symbol E_UNRECOGNISED_INFO_FIELD		= E_BASE_ENGINE + 0x001B;
const Symbol E_PYTHON						= E_BASE_ENGINE + 0x001C;
const Symbol E_MODULE_NOT_FOUND				= E_BASE_ENGINE + 0x001D;
const Symbol E_SYSTEM						= E_BASE_ENGINE + 0x001E;
const Symbol E_SYNC_TIMEOUT					= E_BASE_ENGINE + 0x001F;
const Symbol E_SEED_OVERFLOW				= E_BASE_ENGINE + 0x0020;
const Symbol E_SEED_INVALID					= E_BASE_ENGINE + 0x0021;
const Symbol E_VOICE_UNDEFINED				= E_BASE_ENGINE + 0x0022;
const Symbol E_WRONG_COMPONENT_TYPE			= E_BASE_ENGINE + 0x0023;
const Symbol E_DATA_TYPE_UNSUPPORTED		= E_BASE_ENGINE + 0x0024;
const Symbol E_OUT_OF_ORDER_DELIVERY		= E_BASE_ENGINE + 0x0025;

namespace brahms
{
	namespace base
	{
		const char* symbol2string(Symbol symbol);
	}
}


#endif



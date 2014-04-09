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

	$Id:: common.h 2419 2009-11-30 18:33:48Z benjmitch         $
	$Rev:: 2419                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)       $
________________________________________________________________

*/



/*

	Code that is common between 1258 and 1262 (and more...?)

*/

#ifdef __CHECK_TIMING__
#include "timer.h"
#endif


#ifdef __NIX__

#include "sys/types.h"
#include "sys/stat.h"
#include "unistd.h"

#endif


//	STL includes
#include <ctime>
#include <cmath>
#include <sstream>
#include <iostream>
#include <algorithm>

//	operations
const UINT32 OPERATION_NULL						= 0x00000000;
/*
const UINT32 OPERATION_IIF_CREATE_SET			= 0x00000001;
const UINT32 OPERATION_OIF_CREATE_SET			= 0x00000002;
*/
const UINT32 OPERATION_ADD_PORT					= 0x00000003;
const UINT32 OPERATION_SET_CONTENT				= 0x00000004;
const UINT32 OPERATION_BOUT						= 0x00000005;
const UINT32 OPERATION_GET_UTILITY_OBJECT		= 0x00000006;
const UINT32 OPERATION_GET_UTILITY_FUNCTION		= 0x00000007;
const UINT32 OPERATION_CALL_UTILITY_FUNCTION	= 0x00000008;
//const UINT32 OPERATION_GET_PROCESS_STATE		= 0x00000009;
const UINT32 OPERATION_GET_SYMBOL_STRING		= 0x0000000A;
const UINT32 OPERATION_GET_RANDOM_SEED			= 0x0000000B;



////////////////	SYSTEMML

struct Set : public vector<Port*>
{
	Set()
	{
		hSet = S_NULL;

		mxSet = NULL;
		mxIndex = NULL;
		mxPorts = NULL;
	}

	~Set()
	{
		for (UINT32 i=0; i<size(); i++)
			delete at(i);
	}

	string name;
	Symbol hSet;

	ENGINE_OBJECT mxSet;
	ENGINE_OBJECT mxIndex;
	ENGINE_OBJECT mxPorts;
};

struct Utility
{
	Utility()
	{
		____CLEAR(hev);
	}

	Symbol fireNoCheck(Symbol type, UINT32 flags, void* data)
	{
		hev.event.type = type;
		hev.event.flags = flags;
		hev.event.data = data;
		return hev.handler(&hev.event);
	}

	Symbol fireCheck(Symbol type, UINT32 flags, void* data)
	{
		hev.event.type = type;
		hev.event.flags = flags;
		hev.event.data = data;
		Symbol err = hev.handler(&hev.event);
		if (S_ERROR(err)) throw err;
		return err;
	}

	UINT32 handle;
	string identifier;
	Symbol hUtility;
	HandledEvent hev;
	ENGINE_OBJECT mxUtility;
};

struct UtilityFunction
{
	UINT32 clientHandle;
	string identifier;
	Utility* utility;
	UINT32 moduleHandle;
	ENGINE_OBJECT mxFunction;
};


Symbol quick_errorMessage(Symbol error, const char* msg, UINT32 flags)
{
	EventErrorMessage data;
	data.error = error;
	data.msg = msg;
	data.flags = flags;

	EngineEvent event;
	event.hCaller = 0;
	event.flags = 0;
	event.type = ENGINE_EVENT_ERROR_MESSAGE;
	event.data = &data;
	throw brahms::brahms_engineEvent(&event);
}

bool fileexists(string path)
{

#ifdef __WIN__

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path.c_str(), &FindFileData);
	FindClose(hFind);
	return hFind != INVALID_HANDLE_VALUE;

#endif

#ifdef __NIX__

	struct stat buf;
	return !stat(path.c_str(), &buf);

#endif

}

#define iswhitespace(c) (c == 10 || c == 13 || c == 32 || c == 9)

const char* trimws(char* buf)
{
	int L = strlen(buf);
	while(L && iswhitespace(buf[L-1]))
	{
		buf[L-1] = 0;
		L--;
	}
	while(L && iswhitespace(buf[0]))
	{
		buf++;
		L--;
	}
	return buf;
}

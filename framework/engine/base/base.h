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

	$Id:: base.h 2361 2009-11-12 18:39:39Z benjmitch           $
	$Rev:: 2361                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-12 18:39:39 +0000 (Thu, 12 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_BASE
#define INCLUDED_BRAHMS_BASE

#include "config.h"

////////////////	HEADERS

//	include STL
#include <string>
#include <vector>
#include <sstream>
#include <list>
#include <stack>
#include <iostream>
#include <deque>
#include <fstream>
using namespace std;

//	include C headers (memset, memcpy, maths, etc.)
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <cctype>



////////////////	BUILDING THE ENGINE
#define BRAHMS_BUILDING_ENGINE
////////////////	COMPONENT INTERFACE

//	must define this
#ifndef BRAHMS_BUILD_TARGET
#define BRAHMS_BUILD_TARGET ENGINE_PART
#endif

//	public interface
#include "brahms-client.h"
using namespace brahms;



////////////////	MACROS DEFINED FOR EASE OR MSG CONSISTENCY

#define ____NOT_COMPLIANT(ev, msg) berr << E_NOT_COMPLIANT << "(" << ev << ") " << msg;
#define ____CLEAR(what) memset(&what, 0, sizeof(what))



////////////////	BASE

//	includes
#include "constants.h"
#include "types.h"
#include "os.h"
#include "brahms_error.h"
#include "brahms_time.h"
#include "output.h"
#include "thread.h"
#include "core.h"
#include "ipm.h"
#include "text.h"
#include "brahms_math.h"



#endif

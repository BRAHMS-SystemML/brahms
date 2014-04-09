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

	$Id:: systemml.h 2283 2009-11-02 00:22:31Z benjmitch       $
	$Rev:: 2283                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-02 00:22:31 +0000 (Mon, 02 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_SYSTEMML
#define INCLUDED_SYSTEMML



////////////////	HEADERS

//	include
#include "support/support.h"
#include "main/enginedata.h"



//	Expose
namespace brahms
{
	namespace systemml
	{
		struct Expose
		{
			Expose(string p_what, string p_as)
			{
				what = p_what;
				as = p_as;
			}

			string what;
			string as;
		};
	}
}



//	headers
#include "event.h"
#include "loggable.h"
#include "identifier.h"
#include "link.h"
#include "port.h"
#include "set.h"
#include "interface.h"
#include "component.h"
#include "data.h"
#include "process.h"
#include "utility.h"
#include "thread.h"
#include "system.h"



#endif




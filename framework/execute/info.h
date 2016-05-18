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

	$Id:: info.cpp 2389 2009-11-18 11:40:24Z benjmitch         $
	$Rev:: 2389                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-18 11:40:24 +0000 (Wed, 18 Nov 2009)       $
________________________________________________________________

*/

#ifndef _EXECUTE_INFO_H_
#define _EXECUTE_INFO_H_

#ifndef BRAHMS_BUILDING_ENGINE
#define BRAHMS_BUILDING_ENGINE
#endif
#include "brahms-client.h"
using brahms::FrameworkVersion;

extern const FrameworkVersion VERSION_ENGINE;

// INFORMATION OPERATIONS
namespace brahms
{
	namespace info
	{
            void version(bool simple = false);
            void usage();
            void license();
            void credits();
            void audit();
            void brahmsIncludePath();
            void brahmsLibPath();
	}
}

#endif // _EXECUTE_INFO_H_

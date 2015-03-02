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

	$Id:: version.cpp 2251 2009-10-31 01:42:16Z benjmitch      $
	$Rev:: 2251                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-10-31 01:42:16 +0000 (Sat, 31 Oct 2009)       $
________________________________________________________________

*/

// Ensure __NIX__ and __WIN__ etc are set up and also get FrameworkVersion
#ifndef BRAHMS_BUILDING_ENGINE
#define BRAHMS_BUILDING_ENGINE
#endif
#include "brahms-client.h"
using brahms::FrameworkVersion;

#include <string>
using std::string;
#include <sstream>
using std::ostringstream;

const FrameworkVersion VERSION_ENGINE = {
	VERSION_ENGINE_MAJ,
	VERSION_ENGINE_MIN,
	VERSION_ENGINE_REL,
	VERSION_ENGINE_REV
};

inline void v2s(UINT16 version, ostringstream &ret)
{
	if (version == 0xFFFF) ret << "?";
	else ret << version;
}

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

	$Id:: process.h 2406 2009-11-19 20:53:19Z benjmitch        $
	$Rev:: 2406                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-19 20:53:19 +0000 (Thu, 19 Nov 2009)       $
________________________________________________________________

*/

// Configure-time configuration:
#include "BrahmsConfig.h"

//	component information
#define COMPONENT_RELEASE VERSION_BRAHMS_REL
#define COMPONENT_REVISION VERSION_BRAHMS_REV
#define COMPONENT_ADDITIONAL "Author=Ben Mitch\n" "URL=http://brahms.sourceforge.net\n"

//	include the component interface overlay, quick process mode
//	(you must have defined the above to use this)
#define OVERLAY_QUICKSTART_PROCESS
#include "brahms-1199.h"

//	STL
#include <string>
#include <vector>
using namespace std;

//	add data/utility
#include "data_numeric.h"
#include "data_spikes.h"
#include "util_rng.h"

//	import namespaces
namespace numeric = std_2009_data_numeric_0;
namespace spikes = std_2009_data_spikes_0;
namespace rng = std_2009_util_rng_0;

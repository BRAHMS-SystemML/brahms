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

	$Id:: utility.h 2317 2009-11-07 22:10:20Z benjmitch        $
	$Rev:: 2317                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-07 22:10:20 +0000 (Sat, 07 Nov 2009)       $
________________________________________________________________

*/

#include "BrahmsConfig.h"

//	component information
#define COMPONENT_RELEASE VERSION_BRAHMS_REL
#define COMPONENT_REVISION VERSION_BRAHMS_REV
#define COMPONENT_ADDITIONAL "Author=Ben Mitch\n" "URL=http://brahms.sourceforge.net\n"

//	include the component interface overlay
#define COMPONENT_UTILITY
#include "brahms-1199.h"
using namespace brahms;

//	data
const struct ComponentVersion COMPONENT_VERSION = {COMPONENT_RELEASE, COMPONENT_REVISION};

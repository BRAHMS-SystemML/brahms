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

	$Id:: support.cpp 2389 2009-11-18 11:40:24Z benjmitch      $
	$Rev:: 2389                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-18 11:40:24 +0000 (Wed, 18 Nov 2009)       $
________________________________________________________________

*/



#ifdef _MSC_VER
#define BRAHMS_STDOUT_EXPORT __declspec(dllexport)
#else
#define BRAHMS_STDOUT_EXPORT
#endif



#include "support.h"

#include "helpers.cpp"
#include "os.cpp"
#include "environment.cpp"
#include "error.cpp"
#include "loader.cpp"
#include "register.cpp"
#include "xml.cpp"
#include "execution.cpp"
#include "module.cpp"


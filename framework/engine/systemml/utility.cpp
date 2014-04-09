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

	$Id:: utility.cpp 1936 2009-05-23 20:33:39Z benjmitch      $
	$Rev:: 1936                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-05-23 21:33:39 +0100 (Sat, 23 May 2009)       $
________________________________________________________________

*/


#include "systemml.h"

namespace brahms
{
	namespace systemml
	{



		Utility::Utility(
			string name,
			EngineData& engineData,
			brahms::output::Source* tout,
			string className,
			Process* p_process
			)
			: Component(name, engineData, tout, className, 0, CT_UTILITY), parentProcess(p_process)
		{
		}

		Utility::~Utility()
		{
		}



	}
}


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

	$Id:: loggable.cpp 2447 2010-01-13 01:10:52Z benjmitch     $
	$Rev:: 2447                                                $
	$Author:: benjmitch                                        $
	$Date:: 2010-01-13 01:10:52 +0000 (Wed, 13 Jan 2010)       $
________________________________________________________________

*/



//#define DEBUG_RING_BUFFERS

#include "systemml.h"


namespace brahms
{
	namespace systemml
	{




	////////////////	LOGGABLE

		Loggable::Loggable(Component* component)
			: logEvent(S_NULL, 0, component, &logEventData)
		{
			____CLEAR(logEventData);
			logEventData.precision = PRECISION_DO_NOT_LOG; // not logging by default
		}

		void Loggable::setSuggestedOutputFilename(string filename)
		{
			suggestedOutputFilename = filename;
			logEventData.filename = suggestedOutputFilename.c_str();
		}






	}
}


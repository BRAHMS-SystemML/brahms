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

	$Id:: loggable.h 2447 2010-01-13 01:10:52Z benjmitch       $
	$Rev:: 2447                                                $
	$Author:: benjmitch                                        $
	$Date:: 2010-01-13 01:10:52 +0000 (Wed, 13 Jan 2010)       $
________________________________________________________________

*/

#ifndef _ENGINE_SYSTEMML_LOGGABLE_H_
#define _ENGINE_SYSTEMML_LOGGABLE_H_

namespace brahms
{
	namespace systemml
	{



	////////////////	LOGGABLE

		class Loggable
		{

		public:

			Loggable(Component* component);

			//	loggable data
			EventEx logEvent;					//	cached log event
			EventLog logEventData;				//	cached log event data
			string suggestedOutputFilename;		//	filename suggested for logging (pre-calculated and pointed to by log event data)

			vector<LogOriginBaseSamples> origins;	//	list of recording windows (if empty, we always record)

			//	loggable interface
			void setSuggestedOutputFilename(string filename);

		};




	}
}

#endif // _ENGINE_SYSTEMML_LOGGABLE_H_

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

	$Id:: data.h 1984 2009-05-31 21:04:33Z benjmitch           $
	$Rev:: 1984                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-05-31 22:04:33 +0100 (Sun, 31 May 2009)       $
________________________________________________________________

*/




namespace brahms
{
	namespace systemml
	{



	////////////////	DATA

		class Data : public Component
		{

			friend class System;

		public:

			Data(
				string name,
				EngineData& engineData,
				brahms::output::Source* tout,
				string className,
				UINT16 release,
				SampleRate sampleRate
				);

			~Data();

			//	duplicate completely (another component instance)
			Data* duplicate(brahms::output::Source* tout);

			//	parent port
			Port* parentPort;

		};




	}
}

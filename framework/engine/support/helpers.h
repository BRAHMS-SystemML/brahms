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

	$Id:: helpers.h 2278 2009-11-01 21:23:08Z benjmitch        $
	$Rev:: 2278                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-01 21:23:08 +0000 (Sun, 01 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_SUPPORT_HELPERS
#define INCLUDED_BRAHMS_SUPPORT_HELPERS



////////////////	NAMESPACE

namespace brahms
{



////////////////	LOCAL (BRAHMS-SPECIFIC) FUNCTIONS

	namespace local
	{
		class Seed
		{
		
		public:
			void set(const VUINT32& value);
			VUINT32 get(UINT32 count);
			UINT32 available();

		private:
			VUINT32 value;

		};

		class BaseRate
		{

		public:
			SampleRate		findBaseRate(vector<SampleRate>& sampleRates);
			void			setSystemTime(ComponentTime* stime, DOUBLE executionStop);

		public:
			SampleRate		baseSampleRate;		//	valid after findBaseRate()

		};
	}
}




////////////////	INCLUSION GUARD

#endif



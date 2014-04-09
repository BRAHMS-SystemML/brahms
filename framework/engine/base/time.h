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

	$Id:: time.h 2283 2009-11-02 00:22:31Z benjmitch           $
	$Rev:: 2283                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-02 00:22:31 +0000 (Mon, 02 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_FRAMEWORK_BASE_TIME
#define INCLUDED_BRAHMS_FRAMEWORK_BASE_TIME



////////////////	NAMESPACE

namespace brahms
{
	namespace time
	{



////////////////	CPU TIME

		struct TIME_CPU
		{
			TIME_CPU()
			{
				user = 0;
				kernel = 0;
			}

			double					user;
			double					kernel;
		};



////////////////	INIT/RUN/TERM TIME

		struct TIME_IRT
		{
			TIME_IRT()
			{
				init = 0;
				run = 0;
				term = 0;
			}

			void finalize()
			{
				//	if timer is "relative", that is if the t's are recorded from
				//	a clock that is not reset between phases, finalize() sorts out
				//	the contributions of each individual phase
				term -= run;
				run -= init;
			}

			double					init;
			double					run;
			double					term;
		};



////////////////	INIT/RUN/TERM CPU TIME

		struct TIME_IRT_CPU
		{
			void finalize()
			{
				term.user -= run.user;
				run.user -= init.user;
				init.user -= prev.user;

				term.kernel -= run.kernel;
				run.kernel -= init.kernel;
				init.kernel -= prev.kernel;
			}

			TIME_CPU				prev;		//	time spent previous to the below phases
			TIME_CPU				init;
			TIME_CPU				run;
			TIME_CPU				term;
		};



	}
}



////////////////	INCLUSION GUARD

#endif



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

	$Id:: core.cpp 2309 2009-11-06 00:49:32Z benjmitch         $
	$Rev:: 2309                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-06 00:49:32 +0000 (Fri, 06 Nov 2009)       $
________________________________________________________________

*/



namespace brahms
{
	namespace base
	{



////////////////	ERROR STACK

		void ErrorStack::push(brahms::error::Error& error)
		{
			brahms::os::MutexLocker locker(mutex);
			errors.push_back(error);
		}

		vector<brahms::error::Error>& ErrorStack::getErrors()
		{
			return errors;
		}

		void ErrorStack::clearSpurious()
		{
			//	if there is one E_THREAD_ERROR, and at least one other error, we can remove that
			//	one, because it carries no additional information (it's just the way we use to
			//	bring the caller thread down hard when something goes wrong elsewhere)
			brahms::os::MutexLocker locker(mutex);
			for (unsigned int e=0; e<errors.size(); e++)
			{
				if (errors[e].code == E_THREAD_ERROR && errors.size() > 1)
				{
					errors.erase(errors.begin() + e);
					break;
				}
			}
		}



////////////////	NAMESPACE

	}
}





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

	$Id:: error.cpp 2278 2009-11-01 21:23:08Z benjmitch        $
	$Rev:: 2278                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-01 21:23:08 +0000 (Sun, 01 Nov 2009)       $
________________________________________________________________

*/



#include "support.h"




////////////////	NAMESPACE

namespace brahms
{
	namespace error
	{

		

////////////////	EXCEPTION REGISTER

		/*

			TODO: this needs some work. with this arrangement, the
			error register will tend to fill over time with "used"
			errors that have been copied and rethrown but never
			deleted.

		*/

		Symbol ErrorRegister::push(brahms::error::Error& error)
		{
			brahms::os::MutexLocker locker(mutex);

			//	find first GUID not currently in use
			error.guid = 1;
			list<brahms::error::Error>::iterator it;
			for (it=errors.begin(); it!=errors.end(); ++it)
			{
				if ((*it).guid >= error.guid)
				{
					//	cheap and cheerful - a better way is to find GUIDs that
					//	are interleaved between existing GUIDs, but the algorithm
					//	is more expensive, and i doubt this one will ever fail.
					//	result of this algorithm is to have a GUID that is either
					//	one, or one higher than the highest GUID in use in the list.
					error.guid = (*it).guid + 1;

					//	very unlikely, must be caused by some other condition
					if (error.guid > S_GUID_MAX)
						ferr << E_INTERNAL << "error register overflow";
				}
			}

			//	store it
			errors.push_back(error);

			//	error.code shouldn't have a guid, but we just make sure
			//	because if anyone is storing something here, this information
			//	is going to be lost
			if (error.code & S_GUID_MASK)
				ferr << E_INTERNAL << "error code of error passed for registration already has GUID bits set";

			//	ok
			return (error.code & (~S_GUID_MASK)) | (error.guid << S_GUID_OFFSET_BITS);
		}

		brahms::error::Error ErrorRegister::pull(Symbol error)
		{
			brahms::os::MutexLocker locker(mutex);

			//	extract GUID from GUID bits of error Symbol
			UINT32 guid = (error & S_GUID_MASK) >> S_GUID_OFFSET_BITS;

			//	may be an unregistered error symbol, in which case there
			//	is no further information, so we can just create an error
			//	and return it directly without reference to the register
			if (guid == 0)
			{
				brahms::error::Error e(error);
				return e;
			}

			//	search amongst the registered errors for one with the correct GUID
			list<brahms::error::Error>::iterator it;
			for (it=errors.begin(); it!=errors.end(); ++it)
			{
				if ((*it).guid == guid)
				{
					brahms::error::Error e = *it;
					errors.erase(it);
					return e;
				}
			}

			//	if not found, that's an error in itself
			brahms::error::Error e(E_INTERNAL, "error not found in register (0x" + brahms::text::h2s(error) + ")");
			return e;
		}

		brahms::error::Error& ErrorRegister::borrow(Symbol error)
		{
			//	extract GUID from GUID bits of error Symbol
			UINT32 guid = (error & S_GUID_MASK) >> S_GUID_OFFSET_BITS;

			//	may be an unregistered error symbol, in which case there
			//	is no further information, so we can just create an error
			//	and add it to the register
			if (guid == 0)
			{
				brahms::error::Error e(error);
				error = push(e);
				return borrow(error);
			}

			{
				brahms::os::MutexLocker locker(mutex);

				//	search amongst the registered errors for one with the correct GUID
				list<brahms::error::Error>::iterator it;
				for (it=errors.begin(); it!=errors.end(); ++it)
				{
					if ((*it).guid == guid)
					{
						return *it;
					}
				}
			}

			//	if not found, that's an error in itself
			brahms::error::Error e(E_INTERNAL, "error not found in register (0x" + brahms::text::h2s(error) + ")");
			error = push(e);
			return borrow(error);
		}

		ErrorRegister globalErrorRegister;



////////////////	NAMESPACE

	}
}



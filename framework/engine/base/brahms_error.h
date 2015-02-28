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

	$Id:: error.h 2376 2009-11-15 23:34:44Z benjmitch          $
	$Rev:: 2376                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-15 23:34:44 +0000 (Sun, 15 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_FRAMEWORK_BASE_ERROR
#define INCLUDED_BRAHMS_FRAMEWORK_BASE_ERROR

#include <string>
#include <vector>
#include <sstream>
using std::string;
using std::vector;
using std::stringstream;

#include "brahms-client.h"

////////////////	NAMESPACE

namespace brahms
{
	namespace error
	{

////////////////	EXCEPTION CLASS

		struct BRAHMS_ENGINE_VIS_CPP Error
		{
			Error()
			{
				code = S_NULL;
			}

			Error(Symbol code)
			{
				this->code = code;
			}

			Error(Symbol code, const string& msg)
			{
				this->code = code;
				this->msg = msg;
			}

			~Error() throw()
			{
			}

			Error& at(const char* file, int line);
			Error& trace(const string& msg);
			Error& debugtrace(const string& msg);
			string format(TextFormat ef, bool debugtrace);

			Symbol code;			//	e.g. E_USER
			string msg;				//	additional info on top of "code"
			vector<string> traces;	//	one or more trace messages
			UINT32 guid;			//	used when errors are stored in the error register
		};



////////////////	EXCEPTION BUILDER CLASS

		struct ErrBuilder
		{
			ErrBuilder()
			{
			}

			//	copy ctor (copy is used to invoke throwing)
			ErrBuilder(const ErrBuilder& src)
			{
				error.code = src.error.code;
				error.msg = src.msg.str();
				error.traces = src.error.traces;
				throw error;
			}

			//	handle symbol
			ErrBuilder& operator<<(Symbol data)
			{
				if (error.code == S_NULL) error.code = data;
				else msg << data;
				return (*this);
			}

			//	handle trace
			ErrBuilder& at(const char* file, int line)
			{
				error.at(file, line);
				return (*this);
			}

			//	handle data
			template<class T> ErrBuilder& operator<<(const T& data)
			{
				if (error.code == S_NULL) error.code = E_ERROR;
				msg << data;
				return (*this);
			}

			//	handle function ("hex", etc.)
			template<class T> ErrBuilder& operator<<(T& (*data)(T&))
			{
				if (error.code == S_NULL) error.code = E_ERROR;
				msg << data;
				return (*this);
			}

			stringstream msg;
			Error error;
		};

		#define ferr brahms::error::ErrBuilder unusedInstanceInvokesCopyCtor = brahms::error::ErrBuilder().at(__FILE__, __LINE__)



	}
}



////////////////	INCLUSION GUARD

#endif

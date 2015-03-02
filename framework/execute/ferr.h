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

	$Id:: ferr.h 2376 2009-11-15 23:34:44Z benjmitch           $
	$Rev:: 2376                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-15 23:34:44 +0000 (Sun, 15 Nov 2009)       $
________________________________________________________________

*/

#ifndef _EXECUTE_FERR_H_
#define _EXECUTE_FERR_H_

#include <string>
using std::string;
#include <sstream>
using std::stringstream;

////////////////	ERROR INTERFACE
class ErrorBuilder
{

public:

    //	ctor
    ErrorBuilder()
        {
        }

    //	copy ctor (copy is used to invoke throwing)
    ErrorBuilder(const ErrorBuilder& src)
        {
            string err = src.msg.str();
            err += "\n\tat " + src.trace.str();
            throw err;
        }

    //	handle data
    template<class T> ErrorBuilder& operator<<(const T& data)
        {
            msg << data;
            return (*this);
        }

    //	handle function
    template<class T> ErrorBuilder& operator<<(T& (*data)(T&))
        {
            msg << data;
            return (*this);
        }

    //	handle trace
    ErrorBuilder& at(const char* file, int line)
        {
            trace << file << ":" << line;
            return (*this);
        }

    //	members
    stringstream msg, trace;
};

#define client_err ErrorBuilder unusedInstanceInvokesCopyCtor = ErrorBuilder().at(__FILE__, __LINE__)

#endif // _EXECUTE_FERR_H_

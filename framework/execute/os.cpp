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

	$Id:: os.cpp 2376 2009-11-15 23:34:44Z benjmitch           $
	$Rev:: 2376                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-15 23:34:44 +0000 (Sun, 15 Nov 2009)       $
________________________________________________________________

*/

// Ensure __NIX__ is set up
#ifndef BRAHMS_BUILDING_ENGINE
#define BRAHMS_BUILDING_ENGINE
#endif
#include "brahms-client.h"

#include "os.h"
#include "ferr.h"

#ifdef __NIX__
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
string lasterr;
#endif

namespace os
{

    // GET THE VALUE OF AN ENVIRONMENT VARIABLE

    string getenv(string key, bool exceptionIfAbsent)
    {
        //	return the value of the environment variable named "key"
        //	if absent, throw an exception or return the empty string

#ifdef __WIN__
        char value[16384];
        UINT32 L = GetEnvironmentVariable(key.c_str(), value, 16384);

        if (L)
        {
            //	check for overflow
            if (L > 16384) client_err << "E_INVOCATION: Environment variable too big \"" + key + "\"";

            //	found, and ok
            return value;
        }

        if (!L)
        {
            //	the variable might just have been zero length!
            if (GetLastError() != ERROR_ENVVAR_NOT_FOUND)  return value;
        }
#endif

#ifdef __NIX__
        char *value = ::getenv(key.c_str());
        if (value) return value;
#endif

        //	otherwise, not found
        if (exceptionIfAbsent) client_err << "E_INVOCATION: Environment variable not found \"" + key + "\"";
        return "";
    }

    // EXPAND TOKENS IN PATH
    string expandpath(string path)
    {

        if (path.length())
        {
            if (path.substr(0,1) == "~")
            {
                //	string to take HOME directory
                string SHOME;

#ifdef __WIN__
                //	can get from environment
                SHOME = os::getenv("USERPROFILE");
#endif

#ifdef __NIX__
                //	can get from environment
                SHOME = os::getenv("HOME");

                //	but if not there, can get from system
                if (!SHOME.length())
                {
                    struct passwd* pw = getpwuid(getuid());
                    if (pw) SHOME = pw->pw_dir;
                }
#endif

                //	replace, or don't bother if empty
                if (SHOME.length())
                {
                    path = SHOME + path.substr(1);
                }
            }
        }

        //	ok
        return path;
    }

    // SEE IF A FILE EXISTS
    bool fileexists(string path)
    {
#ifdef __WIN__
        WIN32_FIND_DATA FindFileData;
        HANDLE hFind = FindFirstFile(path.c_str(), &FindFileData);
        FindClose(hFind);
        return hFind != INVALID_HANDLE_VALUE;
#endif

#ifdef __NIX__
        struct stat buf;
        return !stat(path.c_str(), &buf);
#endif
    }
}

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

	$Id:: os.h 2251 2009-10-31 01:42:16Z benjmitch             $
	$Rev:: 2251                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-10-31 01:42:16 +0000 (Sat, 31 Oct 2009)       $
________________________________________________________________

*/




#ifdef __NIX__

//	includes
#include "errno.h"
#include "pthread.h"

#include <dlfcn.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <pwd.h>

#ifdef __GUI__
#include <X11/Intrinsic.h>
#endif

//	store last error here
string lasterr;

#endif

#ifdef __WIN__

//	includes
#define _WIN32_IE 0x0500
#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "commctrl.h"

#include "shlobj.h"

#endif




namespace os
{



	string expandpath(string path);
	string getenv(string key, bool exceptionIfAbsent = true);
	bool fileexists(string path);



////	END NAMESPACE

}



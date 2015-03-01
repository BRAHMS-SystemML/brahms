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

	$Id:: os.h 2409 2009-11-19 23:58:09Z benjmitch             $
	$Rev:: 2409                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-19 23:58:09 +0000 (Thu, 19 Nov 2009)       $
________________________________________________________________

*/




#ifndef INCLUDED_BRAHMS_SUPPORT_OS
#define INCLUDED_BRAHMS_SUPPORT_OS

#include <string>
using std::string;

#ifndef BRAHMS_BUILDING_ENGINE
#define BRAHMS_BUILDING_ENGINE
#endif
#include "brahms-client.h" // for UINT32

////////////////	NAMESPACE

namespace brahms
{
	namespace os
	{

	////////////////	CONSTANTS

	const char COMPONENT_RELEASE_FOLDER_FORM[] = "brahms/((RELEASE))";

	#ifdef __WIN__

		const char PATH_SEPARATOR[] = "\\";
		const char ENV_SEPARATOR[] = ";";
		const char COMPONENT_DLL_EXT[] = "dll";
		const char COMPONENT_MODULE_FORM_OLD[] = "libbrahms-((CLASS_US)).0.dll";
		const char M_BINDING_MODULE_FORM[] = "/BRAHMS/bindings/component/1258/libbrahms-1258.0.dll";
		const char PY_BINDING_MODULE_FORM[] = "/BRAHMS/bindings/component/1262/libbrahms-1262.0.dll";
		const char CHANNEL_MODULE_FORM[] = "/BRAHMS/bin/libbrahms-channel-((PROTOCOL)).dll";
		const char COMPRESS_MODULE_FORM[] = "/BRAHMS/bin/libbrahms-compress.dll";

	#endif

	#ifdef __GLN__

		const char PATH_SEPARATOR[] = "/";
		const char ENV_SEPARATOR[] = ":";
		const char COMPONENT_DLL_EXT[] = "so";
		const char COMPONENT_MODULE_FORM_OLD[] = "libbrahms-((CLASS_US)).so.0";
		const char M_BINDING_MODULE_FORM[] = "/BRAHMS/bindings/component/1258/libbrahms-1258.so.0";
		const char PY_BINDING_MODULE_FORM[] = "/BRAHMS/bindings/component/1262/libbrahms-1262.so.0";
		const char CHANNEL_MODULE_FORM[] = "/BRAHMS/bin/libbrahms-channel-((PROTOCOL)).so";
		const char COMPRESS_MODULE_FORM[] = "/BRAHMS/bin/libbrahms-compress.so";

	#endif

	#ifdef __OSX__

		const char PATH_SEPARATOR[] = "/";
		const char ENV_SEPARATOR[] = ":";
		const char COMPONENT_DLL_EXT[] = "dylib";
		const char COMPONENT_MODULE_FORM_OLD[] = "libbrahms-((CLASS_US)).0.dylib";
		const char M_BINDING_MODULE_FORM[] = "/BRAHMS/bindings/component/1258/libbrahms-1258.0.dylib";
		const char PY_BINDING_MODULE_FORM[] = "/BRAHMS/bindings/component/1262/libbrahms-1262.0.dylib";
		const char CHANNEL_MODULE_FORM[] = "/BRAHMS/bin/libbrahms-channel-((PROTOCOL)).0.dylib";
		const char COMPRESS_MODULE_FORM[] = "/BRAHMS/bin/libbrahms-compress.so";

	#endif



	////////////////	DIRECTORY

		struct Directory
		{
			Directory(const string& path);

			string getNextFile();
			void close();

			bool wasDirectory();
			bool wasSymLink();

			//	state
			string path;

	#ifdef __WIN__

			WIN32_FIND_DATA FindData;
			HANDLE hFind;

	#endif

	#ifdef __NIX__

			void* dir;
			void* ent;

	#endif

		};



	////////////////	STATIC FUNCTIONS

		string getlasterror(UINT32* code = 0);
		string getenv(string key, bool exceptionIfAbsent = true);
		bool fileexists(string path);
		string expandpath(string path);
		string gethostname();
		string getcurdir();
		string filenamepath(string filename);
		void setprocesspriority(INT32 p);
		UINT32 getnumprocessors();
		string getspecialfolder(string id);
		bool mkdir(string path);
		void msgbox(const char* msg);
		void msleep(UINT32 msec);
		string getuctime();



////////////////	NAMESPACE

	}
}



////////////////	INCLUSION GUARD

#endif

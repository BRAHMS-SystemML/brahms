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

	$Id:: os.cpp 2382 2009-11-16 18:12:18Z benjmitch           $
	$Rev:: 2382                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-16 18:12:18 +0000 (Mon, 16 Nov 2009)       $
________________________________________________________________

*/





#include "support.h"



#ifdef __WIN__

#include "shlobj.h"

#endif

#ifdef __NIX__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <pthread.h>
#include <errno.h>

#endif

#ifdef __OSX__

#include <sys/types.h>
#include <sys/sysctl.h>

#endif

using namespace brahms::text;


////////////////	NAMESPACE

namespace brahms
{
	namespace os
	{



	////////////////	DIRECTORY

		Directory::Directory(const string& path)
		{

	#ifdef __WIN__

			hFind = INVALID_HANDLE_VALUE;
			this->path = path + "\\*";

	#endif

	#ifdef __NIX__

			dir = NULL;
			this->path = path;

	#endif

		}

		string Directory::getNextFile()
		{

	#ifdef __WIN__

			if (hFind == INVALID_HANDLE_VALUE)
			{
				//	find first
				hFind = FindFirstFile(path.c_str(), &FindData);
				if (hFind == INVALID_HANDLE_VALUE) return "";
			}

			else
			{
				//	find next
				if (!FindNextFile(hFind, &FindData)) return "";
			}

			//	ignore . and ..
			if (FindData.cFileName[0] == '.') return getNextFile();

			//	ok
			return FindData.cFileName;

	#endif

	#ifdef __NIX__

			if (!dir)
			{
				//	find first
				dir = opendir(path.c_str());
				if (!dir) return "";
			}

			//	first or next, same call
			ent = readdir((DIR*)dir);
			if (!ent) return "";

			//	ignore . and ..
			if (((dirent*)ent)->d_name[0] == '.') return getNextFile();

			//	ok
			return ((dirent*)ent)->d_name;

	#endif

		}

		bool Directory::wasDirectory()
		{

	#ifdef __WIN__

			return FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

	#endif

	#ifdef __NIX__

			if (!ent) ferr << E_INTERNAL << "ent == NULL";
			struct stat statbuf;
			ostringstream buffer;
			buffer << path << "/" << ((dirent*)ent)->d_name;
			if (stat(buffer.str().c_str(), &statbuf) < 0) return false; //  In the expectation that opening as a file will fail
			return (statbuf.st_mode & S_IFMT) == S_IFDIR;

	#endif

		}

		bool Directory::wasSymLink()
		{

	#ifdef __WIN__

			return false;

	#endif

	#ifdef __NIX__

			if (!ent) ferr << E_INTERNAL << "ent == NULL";
			struct stat statbuf;
			ostringstream buffer;
			buffer << path << ((dirent*)ent)->d_name;
			if (lstat(buffer.str().c_str(), &statbuf) < 0) return false; //  In the expectation that opening as a file will fail
			return (S_ISLNK(statbuf.st_mode) == 1);

	#endif

		}

		void Directory::close()
		{

	#ifdef __WIN__

			FindClose(hFind);

	#endif

	#ifdef __NIX__

			closedir((DIR*)dir);

	#endif

		}







	#ifdef __NIX__
	string lasterr;
	#endif



	////////////////	STATIC FUNCTIONS

		#ifdef __NIX__

		string getlasterror(UINT32* code)
		{
			if (code) *code = 0;
			if (lasterr.length()) return lasterr;
			return "no further information";
		}

		#endif

		#ifdef __WIN__

		string getlasterror(UINT32* code)
		{
			//	format last error string
			DWORD dw = GetLastError();
			if (code) *code = dw;

			char buffer[1024] = "<no additional information>";
			if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, 1024, NULL )) return buffer;

			//	trim trailing whitespace
			int L = strlen(buffer);
			while(L)
			{
				if (!(buffer[L-1] == 10 || buffer[L-1] == 13 || buffer[L-1] == 32 || buffer[L-1] == 9)) break;
				buffer[--L] = 0;
			}

			//	ok
			return buffer;
		}

		#endif

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
				if (L > 16384) ferr << E_OS << "Environment variable too big \"" + key + "\"";

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
			if (exceptionIfAbsent) ferr << E_OS << "Environment variable not found \"" + key + "\"";
			return "";

		}

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

		string expandpath(string path)
		{
			if (path.length())
			{
				if (path.substr(0,1) == "~")
				{

					//	string to take HOME directory
					string HOME;

		#ifdef __WIN__

					//	can get from environment
					HOME = brahms::os::getenv("USERPROFILE");

		#endif

		#ifdef __NIX__

					//	can get from environment
					HOME = brahms::os::getenv("HOME", false);

					//	but if not there, can get from system
					if (!HOME.length())
					{
						struct passwd* pw = getpwuid(getuid());
						if (pw) HOME = pw->pw_dir;
					}

		#endif

					//	replace, or don't bother if empty
					if (HOME.length())
						path = HOME + path.substr(1);

				}
			}

			//	ok
			return path;
		}

		string gethostname()
		{
			char buffer[1024];

		#ifdef __WIN__
			DWORD L = 1023;
			if (GetComputerName(buffer, &L))
				return buffer;
		#endif

		#ifdef __NIX__
			if (::gethostname(buffer, 1023) == 0)
				return buffer;
		#endif

			return "(unable to get host name)";
		}

		string getcurdir()
		{

		#ifdef __WIN__

			char value[16384];
			UINT32 L = GetCurrentDirectory(16384, value);
			if (!L || L > 16000) ferr << E_OS << "failed get current directory";
			return value;

		#endif

		#ifdef __NIX__

			char value[16384];
			const char* buf = getcwd(value, 16384);
			if (!buf) ferr << E_OS << "failed get current directory";
			return buf;

		#endif

		}

		string filenamepath(string filename)
		{
			//	path is the bit that comes before the last file path separator
			string::size_type last1 = filename.rfind("/");
			last1 = last1 == string::npos ? 0 : last1;
			string::size_type last2 = filename.rfind("\\");
			last2 = last2 == string::npos ? 0 : last2;
			string::size_type last = last1 > last2 ? last1 : last2;
			if (!last) return "";
			return filename.substr(0, last);
		}

		void setprocesspriority(INT32 p)
		{

		#ifdef __WIN__

			//	set process priority
			//	3 - very high
			//	2 - high
			//	1 - raised
			//	0 - normal
			//	-1 - reduced
			//	-2 - low
			//	-3 - very low
			DWORD dwError, dwPriClass;

			switch(p)
			{
				case 3: dwPriClass = REALTIME_PRIORITY_CLASS; break;
				case 2: dwPriClass = HIGH_PRIORITY_CLASS; break;
				case 1: dwPriClass = ABOVE_NORMAL_PRIORITY_CLASS; break;
				case 0: dwPriClass = NORMAL_PRIORITY_CLASS; break;
				case -1: dwPriClass = BELOW_NORMAL_PRIORITY_CLASS; break;
				case -2: dwPriClass = IDLE_PRIORITY_CLASS; break;
				case -3: dwPriClass = IDLE_PRIORITY_CLASS; break; // on windows, this is as low as it gets!
				default: ferr << E_EXECUTION_PARAMETERS << "invalid priority setting (" << p << ")";
			}

			if(!SetPriorityClass(GetCurrentProcess(), dwPriClass))
			{
			  dwError = GetLastError();
			  ferr << E_OS << "failed to set priority (" << dwError << ")";
			}

			// Display priority class
			//dwPriClass = GetPriorityClass(GetCurrentProcess());

		#endif

		#ifdef __NIX__

			//	not implemented

		#endif

		}

		UINT32 getnumprocessors()
		{

			//	if unknown, return 0
			UINT32 ret = 0;

		#ifdef __WIN__

			HANDLE hProcess = GetCurrentProcess();
			DWORD dwProcessAffinityMask, dwSystemAffinityMask;
			if (GetProcessAffinityMask( hProcess, &dwProcessAffinityMask, &dwSystemAffinityMask ))
			{
				for (int b=0; b<32; b++)
				{
					ret += dwProcessAffinityMask & 1;
					dwProcessAffinityMask >>= 1;
				}
			}

		#endif

		#ifdef __GLN__

			cpu_set_t mask;
			unsigned int len = sizeof(cpu_set_t);
			if (!sched_getaffinity(0, len, &mask))
			{
				for (int b=0; b<32; b++)
					if (CPU_ISSET(b, &mask)) ret++;
			}

		#endif

		#ifdef __OSX__

			//	Proposal...
			//			check if this works

			//	http://alienryderflex.com/processor_count.html
			//	http://developer.apple.com/mac/library/documentation/Darwin/Reference/ManPages/man3/sysctlbyname.3.html
			size_t size = sizeof(UINT32) ;
			if (sysctlbyname("hw.ncpu", &ret, &size, NULL, 0))
				ret = 0; // unknown

		#endif

			return ret;

		}

		string getspecialfolder(string id)
		{
			if (id == "appdata.user")
			{

			#ifdef __WIN__

				char cpath[MAX_PATH];
				HRESULT result = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, cpath);
				if (result != S_OK) return "";
				string path = cpath;
				path += "/BRAHMS";

				//	create if it doesn't exist
				if (!brahms::os::fileexists(path))
				{
					if (!brahms::os::mkdir(path))
					{
						____WARN("failed to make dir \"" << path << "\"");
						return "";
					}
				}

				return path;

			#endif

			#ifdef __NIX__

				string path = brahms::os::getenv("HOME");
				path += "/.BRAHMS";

				//	create if it doesn't exist
				if (!brahms::os::fileexists(path))
				{
					if (!brahms::os::mkdir(path))
					{
						____WARN("failed to make dir \"" << path << "\"");
						return "";
					}
				}

				return path;

			#endif

			}

			if (id == "appdata.machine")
			{

			#ifdef __WIN__

				char cpath[MAX_PATH];
				HRESULT result = SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, cpath);
				if (result != S_OK) return "";
				string path = cpath;
				path += "/BRAHMS";

				//	create if it doesn't exist
				if (!brahms::os::fileexists(path))
				{
					if (!brahms::os::mkdir(path))
					{
						____WARN("failed to make dir \"" << path << "\"");
						return "";
					}
				}

				return path;

			#endif

			#ifdef __NIX__

				string path = "/etc/.BRAHMS";

				//	create if it doesn't exist
				if (!brahms::os::fileexists(path))
				{
					//	NO! on linux, root will need to do this (generally) so don't even try...
					//if (!brahms::os::mkdir(path))
					//{
					//	____WARN("failed to make dir \"" << path << "\"");
					//	return "";
					//}

					//____WARN("dir does not exist (ask root to create it) \"" << path << "\"");
					return "";
				}

				return path;

			#endif

			}

			//	not recognised
			ferr << E_INTERNAL << "unrecognised special folder \"" << id << "\"";
			return "";
		}

		bool mkdir(string path)
		{

		#ifdef __WIN__

			return CreateDirectory(path.c_str(), NULL);

		#endif

		#ifdef __NIX__

			return 0 == ::mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

		#endif

		}

		void msgbox(const char* msg)
		{
			//	display a message box
		#ifdef __WIN__
			MessageBox(0, msg, "BRAHMS message", MB_OK | MB_ICONINFORMATION);
		#endif

		#ifdef __NIX__
			//	not implemented
			____INFO("msgbox(" << msg << ")");
		#endif

		}

		string getuctime()
		{
			stringstream ss;
		#ifdef __WIN__
			SYSTEMTIME time;
			GetSystemTime(&time);
			ss << time.wYear << "-" << n2s(time.wMonth, 2) << "-" << n2s(time.wDay, 2) << " " << n2s(time.wHour, 2) << ":" << n2s(time.wMinute, 2) << ":" << n2s(time.wSecond, 2) << ":" << n2s(time.wMilliseconds, 3);
		#endif

		#ifdef __NIX__
		#endif
			return ss.str();
		}



////////////////	NAMESPACE

	}
}

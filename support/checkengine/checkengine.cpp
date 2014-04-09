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

	$Id:: checkengine.cpp 2386 2009-11-17 19:15:06Z benjmitch  $
	$Rev:: 2386                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-17 19:15:06 +0000 (Tue, 17 Nov 2009)       $
________________________________________________________________

*/



#include "engine.h"
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

#include "os.cpp"


const int OUTPUT_BUFFER_SIZE = 16384;
char outputBuffer[OUTPUT_BUFFER_SIZE];
bool flag_close = true;
int seed = 0;



void execute(int* result)
{
	cout << "opening engine...\n";
	int retstatus;
	Engine* engine = engOpenSingleUse(NULL, NULL, &retstatus);
//	Engine* engine = engOpen(NULL);

	if (!engine)
	{
		cout << "FAILED\n";
		*result = 1;
		return;
	}
	else cout << "OK\n";

	//	clear error status
	cout << "clearing error/warning status...\n";
	if (engEvalString(engine, "lasterr(''); lastwarn('');"))
	{
		cout << "FAILED\n";
		*result = 1;
		return;
	}
	else cout << "OK\n";

	//	attach output buffer
	engOutputBuffer(engine, outputBuffer, OUTPUT_BUFFER_SIZE - 1);	

	//	eval something
	cout << "running \"disp('Hello from Matlab Engine (OK)')\"\n";
	if (engEvalString(engine, "disp('Hello from Matlab Engine (OK)')"))
	{
		cout << "FAILED\n";
		*result = 1;
		return;
	}
	else cout << outputBuffer;
	
	//	eval something
	cout << "running \"a = uint64(now*1e6)\"\n";
	if (engEvalString(engine, "a = uint64(now*1e6)"))
	{
		cout << "FAILED\n";
		*result = 1;
		return;
	}
	else cout << outputBuffer;

	//	workout
	if (seed)
	{
		stringstream ss;

		ss.str("");
		ss << "randn('state', " << seed << ");";
		engEvalString(engine, ss.str().c_str());
		cout << outputBuffer;

		ss.str("");
		ss << "A = randn(100,100);";
		engEvalString(engine, ss.str().c_str());
		cout << outputBuffer;

		ss.str("");
		ss << "A = randn";
		engEvalString(engine, ss.str().c_str());
		cout << outputBuffer;
	}
	
	//	close
	if (flag_close) 
	{
		cout << "closing engine...\n";
		engClose(engine);
		cout << "OK\n";
	}

	//	ok
	*result = 0;
}





int main(int argc, char* argv[])
{
	bool thread = false;

	for (int a=1; a<argc; a++)
	{
		string arg = argv[a];

		//	pass "noclose", then engClose() will not be called
		if (arg == "noclose") { flag_close = false; continue; }

		//	pass "thread", then all engine calls will be made from a separate thread
		if (arg == "thread") { thread = true; continue; }

		//	pass "workoutN", then some decent amount of processing will be done to exercise the engine
		if (arg == "workout1") { seed = 1; continue; }
		if (arg == "workout2") { seed = 2; continue; }

		cout << "unrecognised \"" << arg << "\"\n";
	}

	int ret;
	if (thread)
	{
		os::Thread thread((os::ThreadProc)execute, &ret);
		while(!thread.is_terminated())
		{
			cout << ".";
			os::msleep(100);
		}
		cout << "\n";

		cout << "thread is now terminated (pausing 5 seconds)..." << endl;
		os::msleep(5000);
	}
	else execute(&ret);
	return ret;
}


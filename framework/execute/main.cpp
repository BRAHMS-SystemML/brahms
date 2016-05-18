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

	$Id:: main.cpp 2379 2009-11-16 01:58:12Z benjmitch         $
	$Rev:: 2379                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-16 01:58:12 +0000 (Mon, 16 Nov 2009)       $
________________________________________________________________

*/

/*
	BRAHMS "solo" consists of a single Voice running on its own, which
	will be asked to process the whole system.

	BRAHMS "concerto" consists of one or more Voices running in synchrony,
	and this is one of them.
*/

#include <iostream>
#include <vector>
#include <sstream>
#include <ctime>
#include <fstream>
using namespace std;

#ifdef __GLN__
#include <string.h>
#include <stdlib.h> // atof
#endif

#include "brahms-client.h"
using namespace brahms;

#include "ferr.h"
#include "version.h"
#include "os.h"
#include "tfs.h"
#include "info.h"
#ifndef __NOX11__
	#include "gui/gui.h"
#endif

#ifdef __NIX__
#include <string.h>
#endif

////////////////	GLOBALS
enum Operation
{
	OP_NULL = 0,
	OP_EXECUTE,
	OP_LICENSE,
	OP_CREDITS,
	OP_VERSION,
	OP_WALK,
	OP_AUDIT,
        OP_SHOWINCLUDE,  // Show the API include directory
        OP_SHOWLIB,      // Show the lib directory for any dynamically linked libraries
        OP_SHOWNAMESPACE // Show the set-at-compile-time primary namespace
};

struct Instance
{
	Instance()
	{
		operation = OP_NULL;
		walkLevel = WL_NULL;
		segfault = false;
	}

	void setOperation(Operation o, string arg)
	{
		if (operation != OP_NULL)
			client_err << "E_INVOCATION: argument \"" << arg << "\" unexpected (operation already set)";
		operation = o;
	}

	Operation operation;
	string executionFilename;
	bool segfault;
	brahms::WalkLevel walkLevel;
	string execPars;
	string logLevels;

	string logFilename;
	string exitFilename;

        bool nogui;
}
instance;

CreateEngine createEngine = {0};



////////////////	INTERPRET ARGUMENTS

void grep(string& str, const string what, const string with)
{
	//	don't bother if they are the same
	if (what == with) return;

	//	global replace, but with offset so we can't get an infinite loop
	INT32 offset = 0;
	while(true)
	{
		size_t pos = str.find(what, offset);
		if (pos == string::npos) return;
		str.replace(pos, what.length(), with);
		offset = pos + with.length();
	}
}


void interpretArgs(int argc, char *argv[])
{
        // By default, we ask for a gui progress window:
        instance.nogui = false;

	//	interpret each argument in turn
	for (int a=1; a<argc; a++)
	{
		//	extract arg
		string arg = argv[a];

		//	ignore empty (script "brahms" may pass some of these)
		if (!arg.length()) continue;

		//	convert shortcuts
		if (arg == "--nothreads") arg = "--par-MaxThreadCount-1";
		if (arg.substr(0,8) == "--debug-") arg = "--loglvl-" + arg.substr(8);
		if (arg.substr(0,9) == "--detail-") arg = "--loglvl-" + arg.substr(9);

		//	execution file
		if (!(arg.length() > 2 && arg.substr(0,2) == "--"))
		{
			//	do path translation
			arg = os::expandpath(arg);

			instance.setOperation(OP_EXECUTE, arg);
			instance.executionFilename = arg;
			createEngine.executionFilename = instance.executionFilename.c_str();
			continue;
		}

		//	exit file
		if (arg.length() >= 11 && arg.substr(0,11) == "--exitfile-")
		{
			instance.exitFilename = arg.substr(11);
			continue;
		}

		//	log file
		if (arg.length() >= 10 && arg.substr(0,10) == "--logfile-")
		{
			instance.logFilename = arg.substr(10);
			createEngine.logFilename = instance.logFilename.c_str();
			continue;
		}

		//	out file
		if (arg.length() >= 10 && arg.substr(0,10) == "--outfile-")
		{
			instance.exitFilename = arg.substr(10) + ".exit";
			instance.logFilename = arg.substr(10) + ".xml";
			createEngine.logFilename = instance.logFilename.c_str();
			continue;
		}

		//	voice
		if (arg.length() > 8 && arg.substr(0,8) == "--voice-")
		{
			//	check
			if (createEngine.voiceIndex != VOICE_UNDEFINED)
				client_err << "E_INVOCATION: voice already defined at \"" << arg << "\"";

			//	ask engine to set voice index from MPI layer
			if (arg == "--voice-mpi")
				createEngine.voiceIndex = VOICE_FROM_MPI;

			//	get the voice index from the command line
			else
			{
				createEngine.voiceIndex = atof(arg.substr(8).c_str());
				ostringstream ss;
				ss << "--voice-" << createEngine.voiceIndex;
				if (ss.str() != arg) client_err << "E_INVOCATION: unrecognised option \"" + arg + "\"";
				if (createEngine.voiceIndex < 1 || createEngine.voiceIndex > 1024) client_err << "E_INVOCATION: voice out of range \"" + arg + "\"";
				createEngine.voiceIndex--; // change to zero-based
			}

			//	ok
			continue;
		}

		//	execution parameter
		if (arg.length() >= 6 && arg.substr(0,6) == "--par-")
		{
			string xy = arg.substr(6);
			size_t pos = xy.find('-');
			if (pos == string::npos) pos = xy.find('=');
			if (pos == string::npos) client_err << "E_INVOCATION: unrecognised option \"" + arg + "\"";
			instance.execPars += xy.substr(0, pos) + "=" + xy.substr(pos + 1) + ";";
			createEngine.executionParameters = instance.execPars.c_str();
			continue;
		}

		if (arg == "--logfmt-xml")
		{
			createEngine.logFormat = FMT_XML;
			continue;
		}

		if (arg == "--logfmt-text")
		{
			createEngine.logFormat = FMT_TEXT;
			continue;
		}

		//	log level
		if (arg.length() > 9 && arg.substr(0,9) == "--loglvl-")
		{
			//	pass on to engine
			instance.logLevels += arg.substr(9) + ";";
			createEngine.logLevels = instance.logLevels.c_str();
			continue;
		}

		//	handle --d
		if (arg == "--d")
		{
			//	treat as --loglvl-f --loglvl-l--Main_Loop --maxerr
			instance.logLevels += "f;";
			instance.logLevels += "l--Main_Loop;";
			createEngine.logLevels = instance.logLevels.c_str();
			arg = "--maxerr";
			//	deliberate drop-through...
		}

		//	handle --dd
		if (arg == "--dd")
		{
			//	treat as --loglvl-f --maxerr
			instance.logLevels += "f;";
			createEngine.logLevels = instance.logLevels.c_str();
			arg = "--maxerr";
			//	deliberate drop-through...
		}

		//	operations
		if (arg == "--version")
		{
			instance.setOperation(OP_VERSION, arg);
			continue;
		}

		if (arg == "--license")
		{
			instance.setOperation(OP_LICENSE, arg);
			continue;
		}

		if (arg == "--credits")
		{
			instance.setOperation(OP_CREDITS, arg);
			continue;
		}

                // Show the include directory for compiling against the C/C++ APIs
		if (arg == "--showinclude")
		{
			instance.setOperation(OP_SHOWINCLUDE, arg);
			continue;
		}

		if (arg == "--showlib")
		{
			instance.setOperation(OP_SHOWLIB, arg);
			continue;
		}

		if (arg == "--shownamespace")
		{
			instance.setOperation(OP_SHOWNAMESPACE, arg);
			continue;
		}

		if (arg == "--walk")
		{
			instance.setOperation(OP_WALK, arg);
			instance.walkLevel = WL_LIST;
			continue;
		}

		if (arg == "--Walk")
		{
			instance.setOperation(OP_WALK, arg);
			instance.walkLevel = WL_LOAD_NATIVE;
			continue;
		}

		if (arg == "--WALK")
		{
			instance.setOperation(OP_WALK, arg);
			instance.walkLevel = WL_LOAD_ALL;
			continue;
		}

		if (arg == "--pause")
		{
			createEngine.engineFlags |= F_PAUSE_ON_EXIT;
			continue;
		}

		if (arg == "--maxerr")
		{
			createEngine.engineFlags |= F_MAXERR;
			continue;
		}

		if (arg == "--segfault")
		{
			instance.segfault = true;
			continue;
		}

		if (arg == "--audit")
		{
			instance.setOperation(OP_AUDIT, arg);
			continue;
		}

                if (arg == "--nogui")
                {
                    instance.nogui = true;
                    continue;
                }

		//	no go
		client_err << "E_INVOCATION: unrecognised option \"" << arg << "\"";
	}
}



////////////////	EXECUTE

/*
	This function instantiates the engine and hands control on to the next
	layer. Any error is formatted using the engine (i.e. no further information
	is added).
*/

struct EngineResult
{
	EngineResult()
	{
		error = false;
		messageCount = 0;
	}

	bool error;
	UINT32 messageCount;
	string localError;
};

struct Engine
{
	Engine()
	{
		//	create engine
		hEngine = S_NULL;
		Symbol result = engine_create(&createEngine);
//		if (S_ERROR(result)) throw string("engine_create() failed");
		if (S_ERROR(result)) throw string(createEngine.errorMessage);
		hEngine = result;
	}

	void up()
	{
		//	initialise engine
		Symbol result = engine_up(hEngine);
		if (S_ERROR(result)) throw result;
	}

	EngineResult down() throw()
	{
		//	return true if error
		EngineResult ret;
		ret.error = engine_down(hEngine, &ret.messageCount) != C_OK;
		return ret;
	}

	~Engine()
	{
		if (hEngine)
		{
			Symbol result = engine_destroy(hEngine);
			if (S_ERROR(result)) throw result;
		}
	}

	Symbol hEngine;
};

#ifdef __WIN__
//	CRT security enhancements
#define freopen freopen_local
void inline freopen_local(const char* path, const char* mode, FILE* stream)
{
	FILE* f = NULL;
	freopen_s(&f, path, mode, stream);
}
#endif

#ifndef __NOX11__
// Global pointer to the ExecuteGUI object.
ExecuteGUI* executeGUI;
#endif

EngineResult execute(int argc, char *argv[])
{
	EngineResult engineResult;

	//	catch expected exceptions
	try
	{
		//	default args
		createEngine.voiceIndex = VOICE_UNDEFINED;
#ifndef __NOX11__
		createEngine.handler = MonitorEventHandlerFunc;
#endif
		createEngine.logFormat = FMT_TEXT;

		//	parse args (engine must exist)
		interpretArgs(argc, argv);

                // Now the args have been interpreted, we can create
                // the gui object if we need it.
#ifndef __NOX11__
                if (instance.nogui == false) {
                    executeGUI = new ExecuteGUI();
                } // else don't instanciate executeGUI
#endif

		//	generate a segfault (for checking how it is handled by the executable and by the scripts)
		if (instance.segfault)
		{
			vector<int> v;
			v[0] = 42;
			return engineResult;
		}

		//	text output operations
		switch (instance.operation)
		{
			case OP_NULL:
				brahms::info::usage();
				return engineResult;

			case OP_LICENSE:
				brahms::info::license();
				return engineResult;

			case OP_SHOWINCLUDE:
				brahms::info::brahmsIncludePath();
				return engineResult;

			case OP_SHOWLIB:
				brahms::info::brahmsLibPath();
				return engineResult;

			case OP_SHOWNAMESPACE:
				brahms::info::brahmsPrimaryNamespace();
				return engineResult;

			case OP_CREDITS:
				brahms::info::credits();
				return engineResult;

			case OP_VERSION:
				brahms::info::version();
				return engineResult;

			case OP_AUDIT:
				brahms::info::audit();
				return engineResult;

			default:
				//	just continue
				;
		}

		//	create engine
		Engine engine;

		//	if any ((VOICE)) tokens are in use, we must have voice at this stage
		size_t f1 = instance.logFilename.find("((VOICE))");
		size_t f2 = instance.exitFilename.find("((VOICE))");
		if (f1 != string::npos || f2 != string::npos)
		{
			//	voice is now defined (if VOICE_FROM_MPI, was defined when engine was created)
			if (createEngine.voiceIndex == VOICE_UNDEFINED)
				client_err << "E_VOICE_UNDEFINED: voice undefined when first required";

			//	so we can replace tokens in file names
			stringstream ss;
			ss << (createEngine.voiceIndex + 1);
			grep(instance.logFilename, "((VOICE))", ss.str());
			grep(instance.exitFilename, "((VOICE))", ss.str());
		}

		//	default error now true
		engineResult.error = true;

		//	engine-instance catch
		try
		{
			//	init engine
			engine.up();

			//	operations
			switch (instance.operation)
			{
				case OP_WALK:
				{
					//	must be done after environment is finalized, so we know where the NamespaceRoots are
					Symbol result = engine_walk(engine.hEngine, instance.walkLevel);
					if (S_ERROR(result)) return engine.down();
					break;
				}

				case OP_EXECUTE:
				{
					//	init
					Symbol result = engine_open(engine.hEngine);
					if (S_ERROR(result)) return engine.down();

					//	run
					result = engine_execute(engine.hEngine);
					if (S_ERROR(result)) return engine.down();

					//	term
					result = engine_close(engine.hEngine);
					if (S_ERROR(result)) return engine.down();
					break;
				}

				default:
				{
					client_err << "E_INVOCATION: no operation specified on command line";
				}
			}
		}

		catch(Symbol e)
		{
			return engine.down();
		}

		//	down
		return engine.down();
	}

	catch(const string& e)
	{
		engineResult.localError = e;
		engineResult.error = true;
		return engineResult;
	}

	catch(const exception e)
	{
		engineResult.localError = e.what();
		engineResult.error = true;
		return engineResult;
	}

	catch(...)
	{
		engineResult.localError = "...";
		engineResult.error = true;
		return engineResult;
	}
}

/*
  main() calls execute(), and makes sure to write an
  exit file if requested in the argument list.
*/

//	result codes must be in 0-255 to be bash-compatible
const int RESULT_SUCCESS = 0;
const int RESULT_ERROR = 66; // "B"
const int RESULT_ERROR_INIITAL_PARSE = RESULT_ERROR + 1;

int main(int argc, char *argv[])
{
#ifndef __NOX11__
        // Init executeGUI:
        executeGUI = (ExecuteGUI*)0;
#endif

	string exitFileText =
			"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"
			"<Exit Version=\"1.0\">"
			"<Error>((ERROR))</Error>"
			"<MessageCount>((MSGCOUNT))</MessageCount>"
			"<LocalError>((LOCALERROR))</LocalError>"
			"</Exit>";

	//	exit file will carry error code and message count (engine result)
	EngineResult engineResult;

	//	we mustn't fall over if an exception is thrown since we *have* to write that exit file
	try
	{
		//	error string
		engineResult = execute(argc, argv);
	}

	catch(...)
	{
		//	report
		____FAIL("exception during call to execute()");
	}

	//	get error code
	int errorcode = engineResult.error ? RESULT_ERROR : RESULT_SUCCESS;

	//	if requested, write exit file
	if (instance.exitFilename.length())
	{
		ofstream file;
		file.open(instance.exitFilename.c_str(), ios::binary); // don't do text translation, we work with LF only

		if (file)
		{
			stringstream ss;
			ss << errorcode;
			grep(exitFileText, "((ERROR))", ss.str());
			ss.str("");
			ss << engineResult.messageCount;
			grep(exitFileText, "((MSGCOUNT))", ss.str());
			grep(exitFileText, "((LOCALERROR))", engineResult.localError);
			file.write(exitFileText.c_str(), exitFileText.length());
			file.close();
		}
		else ____FAIL("failed to write exit file");
	}

	//	if not, write error to stdout
	else
	{
		if (engineResult.localError.length())
			cout << engineResult.localError << endl;
	}
#ifndef __NOX11__

        if (executeGUI != (ExecuteGUI*)0) {
            delete executeGUI;
        }
#endif

	//	return error/no-error code
	return errorcode;
}

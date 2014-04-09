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

	$Id:: matlab.cpp 2437 2009-12-13 19:06:12Z benjmitch       $
	$Rev:: 2437                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-13 19:06:12 +0000 (Sun, 13 Dec 2009)       $
________________________________________________________________

*/



#define COMPONENT_PROCESS
#define COMPONENT_CLASS_CPP bindings__matlab_1258
#define BRAHMS_BUILDING_BINDINGS


//#define __CHECK_TIMING__


//	matlab engine
#include "engine.h"

//	common
#define ENGINE_OBJECT mxArray*
#define DESTROY_ENGINE_OBJECT mxDestroyArray
#define BRAHMS_NO_LEGACY_SUPPORT

//	include component interface
#include "base/base.h"

//	include binding
#include "brahms-1199.h"
using namespace std;
using namespace brahms;

const TYPE TYPE_PREFERRED_STORAGE_FORMAT = TYPE_CPXFMT_ADJACENT | TYPE_ORDER_COLUMN_MAJOR;

struct Port
{
	Port()
	{
		mxPorts = NULL;
		form = S_NULL;
		mxData = NULL;
		bytes = 0;
		type = TYPE_UNSPECIFIED;
		hPort = S_NULL;
		____CLEAR(hev);
	}

	~Port()
	{
		cleanup();
	}

	Symbol fireNoCheck(Symbol type, UINT32 flags, void* data)
	{
		hev.event.type = type;
		hev.event.flags = flags;
		hev.event.data = data;
		return hev.handler(&hev.event);
	}

	Symbol fireCheck(Symbol type, UINT32 flags, void* data, void** dataout = NULL)
	{
		hev.event.type = type;
		hev.event.flags = flags;
		hev.event.data = data;
		Symbol err = hev.handler(&hev.event);
		if (S_ERROR(err)) throw err;
		if (dataout) *dataout = hev.event.data;
		return err;
	}

	ENGINE_OBJECT mxPorts;
	Symbol form;
	UINT64 bytes;
	TYPE type;

	string name;
	Symbol hPort;

	HandledEvent hev;
	
	void give(mxArray* p_mxData)
	{
		cleanup();
		mxData = p_mxData;
	}
	
	mxArray* take()
	{
		mxArray* ret = mxData;
		mxData = NULL;
		return ret;
	}
	
	mxArray* borrow()
	{
		if (!mxData) throw E_INTERNAL;
		return mxData;
	}

private:

	void cleanup()
	{
		//	this belongs to us
		if (mxData)
		{
			DESTROY_ENGINE_OBJECT(mxData);
			mxData = NULL;
		}
	}
	
	ENGINE_OBJECT mxData;
	
};

#include "common.h"



//	MX_API_VER is defined in all mx interfaces after R2006B
//	#define MX_API_VER 0x07030000 (7.3, 2006B)
//	#define MX_API_VER 0x07040000 (7.4, 2007b)
//
//	"mwSize" (and i guess "mwIndex") were introduced in R2006B
#ifndef MX_API_VER
#define MX_API_VER 0x00000000 // so this indicates pre-2006B
#endif
#if MX_API_VER >= 0x07030000
typedef mwSize MX_DIM;
#else
typedef int MX_DIM;
#endif
typedef vector<MX_DIM> MX_DIMS;



/*	ENGINE USE POLICY
	
	See Documentation for 1258 for details.
*/

#ifdef __NIX__

//	use one engine for *all* instances of this wrapper (which will all be in the same
//	thread because we set F_NO_CONCURRENCY)
Engine* engineGlobal = NULL;
#define ENGINE engineGlobal

//	when opening the engine, which call do we use...
Engine* engOpenLocal(/*bool execMultiVoice*/)
{
	return engOpen(NULL);
}

void CloseGlobalEngine()
{
	if (engineGlobal) engClose(engineGlobal);
	engineGlobal = NULL;
}

/*
	Unfortunately, that process is closed when the BRAHMS executable exits, so there's no scope for holding
	it open for fast restart. But at least there are no other considerations.
*/

//	easy way to make sure the shared engine never gets closed prematurely: it'll close itself on module exit
#define ENG_CLOSE(e) // do nothing on call to close

#endif



#ifdef __WIN__

//	no need to use a global engine, because the engOpen() call will make it happen for us anyway, and give
//	us a unique handle for each thread that is valid (handles are not valid between threads)
#define ENGINE engineLocal

//	when opening the engine, which call do we use...
Engine* engOpenLocal(/*bool execMultiVoice*/)
{
	int retstatus; // ignored
	//if (execMultiVoice) return engOpenSingleUse(NULL, NULL, &retstatus);
	return engOpen(NULL);
}

void CloseGlobalEngine()
{
}

//	on windows, we do close handles as we're done with them, else engines will get left behind
#define ENG_CLOSE(e) { engClose(e); e = NULL; }

#endif



////////////////	COMPONENT INFORMATION

/*
	F_NO_CONCURRENCY:
	see error tag W_MATLAB_ENGINE_NOT_THREAD_SAFE

	F_NO_CHANGE_THREAD:
	since we are attached to a particular matlab engine, and matlab engine handles are
	thread sensitive, we can't have our thread changed. naturally, we can't have our
	voice changed either, and this flag implies F_NO_CHANGE_VOICE.
*/
#define MODULE_MODULE_FLAGS				(F_NO_CONCURRENCY | F_NO_CHANGE_THREAD)


//	per-module version information
const FrameworkVersion MODULE_VERSION_ENGINE = VERSION_ENGINE;

ModuleInfo MODULE_MODULE_INFO =
{
	&MODULE_VERSION_ENGINE,
	ARCH_BITS,
	1258,
	MODULE_MODULE_FLAGS,
};



const UINT32 OUTPUT_BUFFER_SIZE = 16384;


struct CachedInfo
{
	string cls;
	ComponentVersion component;
	string additional;
	string libraries;
};

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	constructor and destructor, only if required
	COMPONENT_CLASS_CPP(EventModuleCreateBindings* emc);
	~COMPONENT_CLASS_CPP();

	Symbol event(Event* event);
	ComponentInfo* getThisComponentInfo();

	vector<Set> iifx;
	vector<Set> oifx;

	void newInputSetAdded(const char* name);
	void newOutputSetAdded(const char* name);

////////////////	MEMBERS

	//	methods
	void			buildStrings(string suffix);
	void			startEngine();
	void			closeEngine(bool safeToUseBout);
	void			fireWrappedFunction(bool stepCall);
	void			clearWorkspace();
	void			putVariable(const char* name, mxArray* var);
	mxArray*		getVariable(const char* name);

	void			engineCall(const char* command);

	//	helpers
	mxArray*		dataMLToMat(DataMLNode& node);
	void			makeAllInputsAvailable();
	string			mxToString(mxArray* arr, const char* name);
	UINT32			mxToUINT32(mxArray* arr, const char* name, bool acceptDouble = false);
	UINT16			mxToUINT16(mxArray* arr, const char* name, bool acceptDouble = false);

	//	wrapped class path
	string			releaseFolder;
	string			moduleFilename;

	//	variable names
	struct
	{
		string		varInput;
		string		varOutput;
		string		varPersist;
		string		varFunctionHandle;

		string		varLE;
		string		cmdCallFunction;

		string		varVM;
		string		varVJ;
	}
	strings;

	//	state
	Engine*			engineLocal;
	mxArray*		mxInput;
	mxArray*		mxOutput;
	mxArray*		mxPersist;

	//	sub-fields of mxInput
	mxArray*		mxEvent;
	mxArray*		mxObjects;			//	access to Utility objects
	mxArray*		mxTime;					//	Process::time
	mxArray*		mxIif;					//	Process::iif
	mxArray*		mxOif;					//	Process::oif

	//	pointers into mxInput
	UINTA*				p_eventType;
	UINT32*				p_eventCont;
	UINT32*				p_eventFlags;

	//	pointers into mxInput (timing data)
	UINT64*				p_baseSampleRate_num;
	UINT64*				p_baseSampleRate_den;
	DOUBLE*				p_baseSampleRate_rate;
	DOUBLE*				p_baseSampleRate_period;
	BaseSamples*		p_executionStop;
	UINT64*				p_sampleRate_num;
	UINT64*				p_sampleRate_den;
	DOUBLE*				p_sampleRate_rate;
	DOUBLE*				p_sampleRate_period;
	BaseSamples*		p_samplePeriod;
	BaseSamples*		p_now;

	//	output buffer
	char				outputBuffer[OUTPUT_BUFFER_SIZE];

	//	other vars set in fireWrappedFunction()
	Symbol				response;

	//	utility objects requested by scripts
	vector<Utility> utilities;
	vector<UtilityFunction> utilityFunctions;

	//	cached event info
	CachedInfo			info;

	//	check
	UINT32 mxCount;

	//	per-instance version info
	ComponentVersion	versionComponent;

	ComponentInfo componentInfo;

#ifdef __CHECK_TIMING__
	//	timing check
	Timer timer;
	DOUBLE t_run, t_run_matlab;
#endif
};




//	flags set based on the execution environment
//bool execDevMode = false;
//bool execMultiVoice = false;
string VoiceID;



#include "matlab-support.cpp"



void COMPONENT_CLASS_CPP::clearWorkspace()
{
	//	clear all variables from the workspace
	//	DON'T THROW, JUST WARN, COS IF WE'RE GETTING CALLED IN THE DESTRUCTOR WE
	//	DON'T WANT TO CAUSE A FUSS - OTHER COMPONENTS MAY THEN NOT GET DELTEED!
	string cmd = "clear " + strings.varPersist;
	if (engEvalString(ENGINE, cmd.c_str()))
		bout << "Matlab engine session has been terminated whilst calling \"" << cmd << "\"" << D_WARN;
	cmd = "clear " + strings.varInput;
	if (engEvalString(ENGINE, cmd.c_str()))
		bout << "Matlab engine session has been terminated whilst calling \"" << cmd << "\"" << D_WARN;
	cmd = "clear " + strings.varOutput;
	if (engEvalString(ENGINE, cmd.c_str()))
		bout << "Matlab engine session has been terminated whilst calling \"" << cmd << "\"" << D_WARN;
	cmd = "clear " + strings.varFunctionHandle;
	if (engEvalString(ENGINE, cmd.c_str()))
		bout << "Matlab engine session has been terminated whilst calling \"" << cmd << "\"" << D_WARN;
}

void COMPONENT_CLASS_CPP::buildStrings(string suffix)
{
	strings.varInput = "input" + suffix;
	strings.varOutput = "output" + suffix;
	strings.varPersist = "persist" + suffix;
	strings.varFunctionHandle = "function" + suffix;

	string cmd = "cd '" + releaseFolder + "'; " +
		"[" + strings.varPersist + ", " + strings.varOutput + "] = "
		+ moduleFilename + "(" + strings.varPersist + ", " + strings.varInput + ");";

	strings.varLE = "le" + suffix;
	strings.cmdCallFunction = "try, le" + suffix + " = []; ";
	strings.cmdCallFunction += cmd;
	strings.cmdCallFunction += " catch, le" + suffix + " = lasterror; end";

	strings.varVM = "vm" + suffix;
	strings.varVJ = "vj" + suffix;
}

void COMPONENT_CLASS_CPP::putVariable(const char* name, mxArray* var)
{
	if (!engPutVariable(ENGINE, name, var)) return;
	berr << E_MATLAB << "(error putting variable \"" << name << "\")";
}

mxArray* COMPONENT_CLASS_CPP::getVariable(const char* name)
{
	mxArray* mxVar = engGetVariable(ENGINE, name);
	if (mxVar) return mxVar;
	berr << E_MATLAB << "(error getting variable \"" << name << "\")";
	return NULL;
}




////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP(EventModuleCreateBindings* emc)
{
	engineLocal = NULL;
	mxOutput = NULL;
	mxCount = 0;
	ComponentVersion v = {-1, -1};
	versionComponent = v;

	//	lay in path information
	releaseFolder = emc->wrapped.namespaceRootPath + string("/") + emc->wrapped.componentClass + string("/") + emc->wrapped.releasePath;
	moduleFilename = emc->wrapped.moduleFilename;
	grep(releaseFolder, "\\", "/");

	//	create input array
	mxInput = mxCreateStructMatrix(1, 1, 0, 0);
	if (!mxInput) berr << E_MEMORY;

	//	create persist array
	mxPersist =  mxCreateStructMatrix(1, 1, 0, 0);
	if (!mxPersist) berr << E_MEMORY;

	p_eventType = NULL;
	p_eventCont = NULL;
	p_eventFlags = NULL;

	p_baseSampleRate_num = NULL;
	p_baseSampleRate_den = NULL;
	p_baseSampleRate_rate = NULL;
	p_baseSampleRate_period = NULL;
	p_executionStop = NULL;
	p_sampleRate_num = NULL;
	p_sampleRate_den = NULL;
	p_sampleRate_rate = NULL;
	p_sampleRate_period = NULL;
	p_samplePeriod = NULL;
	p_now = NULL;

	//	used during early calls (we should change this so the process gets exec pars before it calls)
	buildStrings("__nosuffix");

	//	constants
	mxArray* mxConstants = addEmptyStruct(mxPersist, "constants");
	addScalarUINTA(mxConstants, "EVENT_MODULE_INIT", EVENT_MODULE_INIT);
	addScalarUINTA(mxConstants, "EVENT_STATE_GET", EVENT_STATE_GET);
	addScalarUINTA(mxConstants, "EVENT_STATE_SET", EVENT_STATE_SET);
	addScalarUINTA(mxConstants, "EVENT_INIT_PRECONNECT", EVENT_INIT_PRECONNECT);
	addScalarUINTA(mxConstants, "EVENT_INIT_CONNECT", EVENT_INIT_CONNECT);
	addScalarUINTA(mxConstants, "EVENT_INIT_POSTCONNECT", EVENT_INIT_POSTCONNECT);
	addScalarUINTA(mxConstants, "EVENT_LOG_INIT", EVENT_LOG_INIT);
	addScalarUINTA(mxConstants, "EVENT_BEGIN_RUNPHASE", EVENT_BEGIN_RUNPHASE);
	addScalarUINTA(mxConstants, "EVENT_BEGIN_TERMPHASE", EVENT_BEGIN_TERMPHASE);
	addScalarUINTA(mxConstants, "EVENT_RUN_PLAY", EVENT_RUN_PLAY);
	addScalarUINTA(mxConstants, "EVENT_RUN_RESUME", EVENT_RUN_RESUME);
	addScalarUINTA(mxConstants, "EVENT_RUN_SERVICE", EVENT_RUN_SERVICE);
	addScalarUINTA(mxConstants, "EVENT_RUN_PAUSE", EVENT_RUN_PAUSE);
	addScalarUINTA(mxConstants, "EVENT_RUN_STOP", EVENT_RUN_STOP);
	addScalarUINTA(mxConstants, "EVENT_LOG_TERM", EVENT_LOG_TERM);
	addScalarUINT32(mxConstants, "F_FIRST_CALL", F_FIRST_CALL);
	addScalarUINT32(mxConstants, "F_LAST_CALL", F_LAST_CALL);
	addScalarUINT32(mxConstants, "F_GLOBAL_ERROR", F_GLOBAL_ERROR);
	addScalarUINT32(mxConstants, "F_LOCAL_ERROR", F_LOCAL_ERROR);
	addScalarUINTA(mxConstants, "S_NULL", S_NULL);
	addScalarUINTA(mxConstants, "C_OK", C_OK);
	addScalarUINTA(mxConstants, "C_STOP_USER", C_STOP_USER);
	addScalarUINTA(mxConstants, "C_STOP_EXTERNAL", C_STOP_EXTERNAL);
	addScalarUINTA(mxConstants, "C_STOP_CONDITION", C_STOP_CONDITION);
	addScalarUINTA(mxConstants, "C_STOP_THEREFOREIAM", C_STOP_THEREFOREIAM);
	addScalarUINT32(mxConstants, "OPERATION_NULL", OPERATION_NULL);
	/*
	addScalarUINT32(mxConstants, "OPERATION_IIF_CREATE_SET", OPERATION_IIF_CREATE_SET);
	addScalarUINT32(mxConstants, "OPERATION_OIF_CREATE_SET", OPERATION_OIF_CREATE_SET);
	*/
	addScalarUINT32(mxConstants, "OPERATION_ADD_PORT", OPERATION_ADD_PORT);
	addScalarUINT32(mxConstants, "OPERATION_SET_CONTENT", OPERATION_SET_CONTENT);
	addScalarUINT32(mxConstants, "OPERATION_BOUT", OPERATION_BOUT);
	addScalarUINT32(mxConstants, "OPERATION_GET_UTILITY_OBJECT", OPERATION_GET_UTILITY_OBJECT);
	addScalarUINT32(mxConstants, "OPERATION_GET_UTILITY_FUNCTION", OPERATION_GET_UTILITY_FUNCTION);
	addScalarUINT32(mxConstants, "OPERATION_CALL_UTILITY_FUNCTION", OPERATION_CALL_UTILITY_FUNCTION);
	//addScalarUINT32(mxConstants, "OPERATION_GET_PROCESS_STATE", OPERATION_GET_PROCESS_STATE);
	addScalarUINT32(mxConstants, "OPERATION_GET_RANDOM_SEED", OPERATION_GET_RANDOM_SEED);
	addScalarUINT32(mxConstants, "D_NONE", D_NONE);
	addScalarUINT32(mxConstants, "D_WARN", D_WARN);
	addScalarUINT32(mxConstants, "D_INFO", D_INFO);
	addScalarUINT32(mxConstants, "D_VERB", D_VERB);

	addScalarUINT32(mxConstants, "F_UNDEFINED", F_UNDEFINED);
	addScalarUINT32(mxConstants, "F_ZERO", F_ZERO);
	addScalarUINT32(mxConstants, "F_NEEDS_ALL_INPUTS", F_NEEDS_ALL_INPUTS);
	addScalarUINT32(mxConstants, "F_INPUTS_SAME_RATE", F_INPUTS_SAME_RATE);
	addScalarUINT32(mxConstants, "F_OUTPUTS_SAME_RATE", F_OUTPUTS_SAME_RATE);
	addScalarUINT32(mxConstants, "F_NOT_RATE_CHANGER", F_NOT_RATE_CHANGER);
	//addScalarUINT32(mxConstants, "F_FRESH", F_FRESH);

	//	template output
	mxArray* mxOutput = addEmptyStruct(mxPersist, "output");
	mxArray* mxOutputEvent = addEmptyStruct(mxOutput, "event");
	addScalarUINTA(mxOutputEvent, "response", S_NULL);
	addEmptyCell(mxOutput, "operations");

	//	place to put states of other objects
	addEmptyStruct(mxPersist, "states");

	//	event
	mxEvent = addEmptyStruct(mxInput, "event");
	p_eventType = addScalarUINTA(mxEvent, "type", 0);
	p_eventCont = addScalarUINT32(mxEvent, "continuation", 0);
	p_eventFlags = addScalarUINT32(mxEvent, "flags", 0);

	//	objects and state arrays
	mxObjects = addEmptyStruct(mxInput, "objects");

	//	set as much info as we can without calling wrapped function (remainder is set in EVENT_MODULE_INIT)
	info.cls = emc->wrapped.componentClass;

	ComponentInfo c =
	{
		info.cls.c_str(),
		CT_PROCESS,
		&versionComponent,
		0,
		"",
		""
	};

	componentInfo = c;


#ifdef __CHECK_TIMING__
	//	timing
	t_run = 0.0;
	t_run_matlab = 0.0;
#endif
}





////////////////////////////////////////////////////////////////
//
//	DESTRUCTOR
//
////////////////////////////////////////////////////////////////

COMPONENT_CLASS_CPP::~COMPONENT_CLASS_CPP()
{
	//	close engine
	closeEngine(false);

	//	check
	if (mxCount) bout << "WARNING: matlab process bindings report " << mxCount << " mxArrays remain";
}




////////////////////////////////////////////////////////////////
//
//	START ENGINE
//
////////////////////////////////////////////////////////////////

void COMPONENT_CLASS_CPP::startEngine()
{
	//	if already started, do nothing
	if (!ENGINE)
	{
		bout << "starting Matlab engine..." << D_VERB;// (execMultiVoice == " << execMultiVoice << ")" << D_VERB;

		ENGINE = engOpenLocal(/*execMultiVoice*/);

		//	check
		if (!ENGINE) berr << E_FAILED_START_MATLAB_ENGINE << "failed to start matlab engine";

		//	clear error status
		if (engEvalString(ENGINE, "lasterr(''); lastwarn('');"))
			berr << E_MATLAB << "matlab engine seemed to open ok but was unresponsive on first call";

		//	attach output buffer
		engOutputBuffer(ENGINE, outputBuffer, OUTPUT_BUFFER_SIZE - 1);
	}

	//	put persistent variable
	putVariable(strings.varPersist.c_str(), mxPersist);
}

void COMPONENT_CLASS_CPP::closeEngine(bool safeToUseBout)
{
	//	get rid of arrays
	clearAndNullArray(mxInput);
	clearAndNullArray(mxOutput);
	clearAndNullArray(mxPersist);

	//	if in linux
	//		ENG_CLOSE() does nothing, so this code is irrelevant
	//	if in windows
	//		if not in dev mode, we always close engines anyway
	//		otherwise, we can skip the close engine call, to leave the single shared engine open for reuse (we also leave its workspace unchanged for inspection by the user after the execution - user can close engine to clear it)
	//		if in multi voice mode, we can't use engOpen(), so engines will be orphaned (left open) but that at least allows the developer to check their state
	if (ENGINE)
	{
		/*
		if (execDevMode)
		{
			if (safeToUseBout)
				bout << "not closing Matlab engine (DevelopmentMode == true)" << D_INFO;
			else
				cerr << "not closing Matlab engine (DevelopmentMode == true)" << endl;

			//	mark ENGINE NULL so we don't try this again in the destructor
			//	this amounts to simply "forgetting" about the engine
			ENGINE = NULL;
		}
		else
		{
			*/
			ENG_CLOSE(ENGINE);
			/*
		}
		*/
	}
}





////////////////////////////////////////////////////////////////
//
//	ENGINE ERROR
//
////////////////////////////////////////////////////////////////

Symbol attachError(Symbol error, const char* msg, UINT32 flags = 0)
{
	EventErrorMessage data;
	data.error = error;
	data.msg = msg;
	data.flags = flags;

	EngineEvent event;
	event.hCaller = 0;
	event.flags = 0;
	event.type = ENGINE_EVENT_ERROR_MESSAGE;
	event.data = &data;

	return brahms::brahms_engineEvent(&event);
}

void COMPONENT_CLASS_CPP::engineCall(const char* command)
{
	//	clear output buffer
	outputBuffer[0] = 0;

	//	call function (via eval)
	if (engEvalString(ENGINE, command))
		berr << E_MATLAB << "matlab engine session has been terminated whilst calling \"" << command << "\"";

	//	print output buffer to console
	if (outputBuffer[0])
	{
		outputBuffer[OUTPUT_BUFFER_SIZE-1] = 0;
		cerr << outputBuffer;
	}

	//	and check for an error
	mxArray* le = engGetVariable(ENGINE, strings.varLE.c_str());

	//	if error
	if (le)
	{
		//	empty is how we leave it if there is no error
		if (mxIsEmpty(le))
		{
			//	no error!
			clearAndNullArray(le);
		}

		//	otherwise it should be an error
		else
		{
			//	was there an error?
			if (!mxIsStruct(le))
			{
				//	no error!
				clearAndNullArray(le);
				berr << E_MATLAB << "failed whilst getting lasterror";
			}

			//	retrieve fields
			string err;

			//	field "message"
			mxArray* field = mxGetField(le, 0, "message");
			if (field)
				err += mxToString(field, "field of lasterror");

			//	remove final period
			if (err.length() && err.substr(err.length()-1) == ".")
				err = err.substr(0, err.length() - 1);

			//	field "identifier"
			field = mxGetField(le, 0, "identifier");
			if (field)
			{
				string s = mxToString(field, "field of lasterror");
				if (s.length())
					err += " (" + s + ")";
			}

			//	remove trace info, if present, since we're going to add it explicitly
			size_t pos = err.find("Error using ==>");
			if (pos != string::npos)
			{
				size_t pos2 = err.find("\n");
				if (pos2 != string::npos)
					err = err.substr(pos2+1);
			}

			//	create error
			Symbol e = attachError(E_USER, err.c_str());

			//	field "stack"
			field = mxGetField(le, 0, "stack");
			if (field && mxIsStruct(field))
			{
				int N = mxGetM(field);
				for (int n=0; n<N; n++)
				{
					string trace = "at line ";
					trace += n2s(mxGetScalar(mxGetField(field, n, "line")));
					trace += " of ";
					trace += mxToString(mxGetField(field, n, "name"), "entry in error stack");
					trace += " [[ ";
					trace += mxToString(mxGetField(field, n, "file"), "entry in error stack");
					trace += " :: ";
					trace += n2s(mxGetScalar(mxGetField(field, n, "line")));
					trace += " ]]";
					e = attachError(e, trace.c_str(), F_TRACE);
				}
			}

			//	delete retrieved object now we've got the string out
			clearAndNullArray(le);

			//	throw user error
			throw e;

	/*
			//	collect more info from engine
			string err = mxToString(em, "lasterr message");
			clearAndNullArray(em);

			//	collect more info from engine
			mxArray* ec = engGetVariable(ENGINE, "ec");
			if (ec)
			{
				err += " (" + mxToString(ec, "lasterr code") + ")";
				clearAndNullArray(ec);
			}
	*/

	/*
			//	collect more info from engine
			if (engEvalString(ENGINE, strings.cmdGetWorkingDir.c_str())) break;
			temp = engGetVariable(ENGINE, strings.varWorkingDir.c_str());
			if (!temp) break;
			string wd = mxToString(temp, "working directory");
			error += "\n    with working dir \"" + wd + "\"";
			clearAndNullArray(temp);
			*/

/*
		don't think so actually - only cmds that ever get executed start with "le = [];" so it's ok!

			//	must clear the lasterr state of the engine, so that
			//	on subsequent calls it doesn't look like we've errored again!

			//	clear error status
			if (engEvalString(ENGINE, "le = [];"))
				berr << E_MATLAB << "matlab engine was closed";

				*/

	/*
			Actually, this makes it hard to see the error message, and it's
			not very helpful because it only shows the esoteric variables
			used by this wrapper.

			//	dump "whos" to console
			engOutputBuffer(engine, outputBuffer, OUTPUT_BUFFER_SIZE - 1);
			if (engEvalString(engine, "whos"))
				berr << E_MATLAB << "Matlab engine session has been terminated whilst calling \"whos\"";
			outputBuffer[OUTPUT_BUFFER_SIZE-1] = 0;
			cerr << outputBuffer;
	*/
		}
	}
}





void COMPONENT_CLASS_CPP::newInputSetAdded(const char* name)
{
	//	create set
	Symbol hSet = iif.getSet(name);
	iifx.resize(iifx.size() + 1);
	Set& set = iifx.back();
	set.hSet = hSet;
	set.name = name;

	//	get field name
	string fieldName = set.name;
	if (!fieldName.length()) fieldName = "default";

	//	create set field, add index field, add ports field
	set.mxSet = addEmptyStruct(mxIif, fieldName.c_str());
}

void COMPONENT_CLASS_CPP::newOutputSetAdded(const char* name)
{
	//	create set
	Symbol hSet = oif.getSet(name);
	oifx.resize(oifx.size() + 1);
	Set& set = oifx.back();
	set.hSet = hSet;
	set.name = oif.getSetName(hSet);

	//	get field name
	string fieldName = set.name;
	if (!fieldName.length()) fieldName = "default";

	//	create set field, add index field, add ports field
	set.mxSet = addEmptyStruct(mxOif, fieldName.c_str());
}



////////////////////////////////////////////////////////////////
//
//	MAKE ALL INPUTS AVAILABLE
//
////////////////////////////////////////////////////////////////

void COMPONENT_CLASS_CPP::makeAllInputsAvailable()
{
	for (UINT32 s=0; s<iifx.size(); s++)
	{
		//	extract
		Set& set = iifx[s];

		//	foreach port on set
		for (UINT32 p=0; p<set.size(); p++)
		{
			//	resolve port
			Port& port = *set[p];

			//	make unavailable
			port.give(mxGetField(port.mxPorts, p, "data"));
			mxSetField(port.mxPorts, p, "data", NULL);

			//	ensure attached
			if (port.form == S_NULL)
			{
				//	try attach
				if (!port.hev.event.object) continue;

				//	ok, attached: get form
				EventGenericForm egf;
				port.fireCheck(EVENT_GENERIC_FORM_ADVANCE, 0, &egf);
				port.form = egf.form;
				port.type = egf.type;

				//	special case (see note below)
				if (port.form == C_FORM_FIXED_BUT_LAST)
					port.form = C_FORM_BOUNDED;


				/*

				C_FORM_FIXED

					In the C_FORM_FIXED case, we can create the mxArray
					and just set its content through the GENERIC interface.

				C_FORM_UNBOUNDED

					In the C_FORM_UNBOUNDED case, we have to create the
					array anew on each call to makeAllInputsAvailable() since
					we can't know in advance how big it will be.

				C_FORM_BOUNDED

					In the C_FORM_BOUNDED case, slightly non-intuitively,
					we can also create the mxArray and set its content through
					the GENERIC interface. We have to be careful though to
					also set its dimensions. But memory allocation is not a
					problem - the following is a quote from the matlab docs
					on mxSetDimensions():

					"mxSetDimensions does not allocate or deallocate any space
					for the pr or pi arrays. Consequently, if your call to
					mxSetDimensions increases the number of elements in the mxArray,
					then you must enlarge the pr (and pi, if it exists) arrays accordingly."

				C_FORM_FIXED
				C_FORM_BOUNDED

					To protect against segfaulting if the data object sends too
					much data back, we keep track of the actual capacity of the
					mxArray we've created in userDataB (in real bytes).

				C_FORM_FIXED_BUT_LAST

					We don't give this case special handling, currently, instead
					treating it as C_FORM_BOUNDED.

				*/

				//	for fixed-ish forms, create data array
				if (port.form == C_FORM_FIXED || port.form == C_FORM_BOUNDED)
				{
					/*

						For these cases, we create the array in advance (for other
						cases, we create it on-the-fly).

					*/

					//	convert dims if necessary
					MX_DIMS dims;
					Dims b_dims = egf.dims;
					for (UINT32 d=0; d<b_dims.size(); d++)
						dims.push_back(b_dims.at(d));

					if ((egf.type & TYPE_CPXFMT_MASK) == TYPE_CPXFMT_INTERLEAVED)
						berr << E_NOT_IMPLEMENTED << "TYPE_CPXFMT_INTERLEAVED not accepted as input to a matlab process";

					//	in these cases, we can create the data object in advance...
					port.give(mxCreateNumericArray(
						dims.size(), &dims[0],
						genericNumericType2clsID(egf.type),
						egf.type & TYPE_COMPLEX ? mxCOMPLEX : mxREAL));

					//	get byte capacity of this array
					port.bytes = b_dims.getNumberOfElements() * TYPE_BYTES(egf.type);
				}

				//	otherwise, in the C_FORM_UNBOUNDED case, we have to create
				//	the mxArray anew at every call to this function

				//	set class into ports array
				EventGetPortInfo portInfo;
				portInfo.hPort = port.hPort;
				portInfo.flags = 0;
				portInfo.name = NULL;
				portInfo.componentInfo = NULL;

				EngineEvent event;
				event.hCaller = hComponent;
				event.flags = 0;
				event.type = ENGINE_EVENT_GET_PORT_INFO;
				event.data = (void*) &portInfo;

				/*brahms::Symbol result = */brahms::brahms_engineEvent(&event);
				mxSetField(port.mxPorts, p, "class", mxCreateString(portInfo.componentInfo->cls));

				//	get data structure and set it in the ports array
				EventGenericStructure eas = {0};
				eas.type = TYPE_PREFERRED_STORAGE_FORMAT;
				port.fireCheck(EVENT_GENERIC_STRUCTURE_GET, 0, &eas);
				mxSetField(port.mxPorts, p, "structure", mxCreateString(eas.structure));
			}

			//	if available, make available
			if (port.hev.event.object)
			{
				//	switch on mode
				switch(port.form)
				{
					case C_FORM_FIXED:
					{
						//	get content through generic access interface
						EventGenericContent egc = {0};
						egc.type = TYPE_PREFERRED_STORAGE_FORMAT;
						port.fireCheck(EVENT_GENERIC_CONTENT_GET, 0, &egc);
						
						//	interpret return
						if (egc.bytes && !egc.real)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "no real data came back");
						if (egc.bytes && mxIsComplex(port.borrow()) && !egc.imag)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "no imaginary data came back");
						if (egc.bytes != port.bytes)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "wrong number of bytes came back, " << egc.bytes << " instead of " << port.bytes);
		
						if (egc.real) memcpy(mxGetData(port.borrow()), egc.real, port.bytes);
						if (egc.imag) memcpy(mxGetImagData(port.borrow()), egc.imag, port.bytes);

						//	copy
						mxSetField(port.mxPorts, p, "data", port.take());

						//	ok
						break;
					}

					case C_FORM_BOUNDED:
					{
						//	have to also set the dimensions anew
						EventGenericForm egf;
						port.fireCheck(EVENT_GENERIC_FORM_CURRENT, 0, &egf);

						//	convert dims if necessary
						MX_DIMS dims;
						Dims b_dims = egf.dims;
						for (UINT32 d=0; d<b_dims.size(); d++)
							dims.push_back(b_dims.at(d));
						UINT64 count = b_dims.getNumberOfElements();
						UINT64 expectedBytes = count * (TYPE_BYTES(egf.type));
						if (expectedBytes > port.bytes)
							____NOT_COMPLIANT("EVENT_GENERIC_FORM_CURRENT", "too many bytes came back for C_FORM_BOUNDED, " << expectedBytes << " was greater than " << port.bytes);

						//	set dimensions
						mxSetDimensions(port.borrow(), &dims[0], dims.size());

						//	get content through generic access interface
						EventGenericContent egc = {0};
						egc.type = TYPE_PREFERRED_STORAGE_FORMAT;
						port.fireCheck(EVENT_GENERIC_CONTENT_GET, 0, &egc);

						//	interpret return
						if (egc.bytes && !egc.real)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "no real data came back");
						if (egc.bytes && mxIsComplex(port.borrow()) && !egc.imag)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "no imaginary data came back");
						if (egc.bytes != expectedBytes)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "wrong number of bytes came back, " << egc.bytes << " instead of " << expectedBytes);

						//	copy in
						if (egc.real) memcpy(mxGetData(port.borrow()), egc.real, egc.bytes);
						if (egc.imag) memcpy(mxGetImagData(port.borrow()), egc.imag, egc.bytes);

						//	copy
						mxSetField(port.mxPorts, p, "data", port.take());

						//	ok
						break;
					}

					case C_FORM_UNBOUNDED:
					{
						//	have to create the data object every time
						EventGenericForm egf;
						port.fireCheck(EVENT_GENERIC_FORM_CURRENT, 0, &egf);

						if ((egf.type & TYPE_CPXFMT_MASK) == TYPE_CPXFMT_INTERLEAVED)
							berr << E_NOT_IMPLEMENTED << "TYPE_CPXFMT_INTERLEAVED not accepted as input to a matlab process";

						//	convert dims if necessary
						MX_DIMS dims;
						Dims b_dims = egf.dims;
						for (UINT32 d=0; d<b_dims.size(); d++)
							dims.push_back(b_dims.at(d));

						//	recreate array (give() will clean up any existing array)
						port.give(mxCreateNumericArray(
							dims.size(), &dims[0],
							genericNumericType2clsID(egf.type),
							egf.type & TYPE_COMPLEX ? mxCOMPLEX : mxREAL));
						UINT64 bytes = b_dims.getNumberOfElements() * TYPE_BYTES(egf.type);

						//	get content through generic access interface
						EventGenericContent egc = {0};
						egc.type = TYPE_PREFERRED_STORAGE_FORMAT;
						port.fireCheck(EVENT_GENERIC_CONTENT_GET, 0, &egc);

						//	interpret return
						if (egc.bytes && !egc.real)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "no real data came back");
						if (egc.bytes && mxIsComplex(port.borrow()) && !egc.imag)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "no imaginary data came back");
						if (egc.bytes != bytes)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "wrong number of bytes came back, " << egc.bytes << " instead of " << bytes);
						if (egc.real) memcpy(mxGetData(port.borrow()), egc.real, bytes);
						if (egc.imag) memcpy(mxGetImagData(port.borrow()), egc.imag, bytes);

						//	copy
						mxSetField(port.mxPorts, p, "data", port.take());

						//	ok
						break;
					}

					default:
					{
						berr << E_INTERNAL << "unexpected!";
					}
				}

			}
		}
	}
}







////////////////////////////////////////////////////////////////
//
//	FIRE WRAPPED FUNCTION
//
////////////////////////////////////////////////////////////////

void COMPONENT_CLASS_CPP::fireWrappedFunction(bool stepCall)
{
	//	check engine
	if (!ENGINE) berr << E_INTERNAL << "fireWrappedFunction() before starting engine";

//	cerr << "fireWrappedFunction(" << engine << ", " << strings.varInput << ", " << hex << (UINT32)mxInput << ")" << endl;

	//	loop until no continuation
	while (true)
	{
#ifdef __CHECK_TIMING__
	DOUBLE t0 = timer.elapsed();
#endif

		//	copy latest input variable into engine
		putVariable(strings.varInput.c_str(), mxInput);

		//	clear output variable, but there's no need to clear it
		//	in the engine, because the only way it can not be set
		//	by the subsequent call is if an error occurs, and we'll
		//	pick that up anyway.
		clearAndNullArray(mxOutput);

		//	call engine
		engineCall(strings.cmdCallFunction.c_str());

		//	get output var
		mxOutput = getVariable(strings.varOutput.c_str());
		if (!mxIsStruct(mxOutput))
			berr << E_NOT_COMPLIANT << "function should return a structure";

#ifdef __CHECK_TIMING__
	t_run_matlab += timer.elapsed() - t0;
#endif

		//	read "event"
		mxArray* mxEvent = mxGetField(mxOutput, 0, "event");
		if (!mxEvent || !mxIsStruct(mxEvent) || (mxGetNumberOfElements(mxEvent) != 1))
			berr << E_NOT_COMPLIANT << "output.event should be scalar structure";
		mxArray* temp = mxGetField(mxEvent, 0, "response");
		if (!temp || !mxIsUintA(temp) || (mxGetNumberOfElements(temp) != 1))
			berr << E_NOT_COMPLIANT << "output.event.response should be scalar UINTA";
		response = *((UINTA*)mxGetData(temp));

		//	read continuation field
		temp = mxGetField(mxEvent, 0, "continuation");
		if (temp)
		{
			if (!mxIsDouble(temp) || (mxGetNumberOfElements(temp) != 1) || mxIsComplex(temp))
				berr << E_NOT_COMPLIANT << "output.event.continuation should be scalar DOUBLE";
			*p_eventCont = (UINT32) *mxGetPr(temp);
		}
		else *p_eventCont = 0;

		//	read "error"
		temp = mxGetField(mxOutput, 0, "error");
		if (temp)
		{
			if (!mxIsChar(temp))
				berr << E_NOT_COMPLIANT << "output.error should be string";
			char err[1024];
			if (mxGetString(temp, err, 1023))
				berr << E_INTERNAL << "an error was returned by the wrapped function, but i failed to propagate it";
			berr << err;
		}

		//	here, we clear all results of called functions, so we don't
		//	keep passing them round and round... but not in a step call, because
		//	if the function is being used in step calls, it's going to keep getting
		//	used, and in that case it's more efficient not to destroy these arrays
		//	each time and rebuild them. note that this heuristic is imperfect: if
		//	the script uses a function on-the-way-in to step, and never uses it
		//	again, we'll still be passing back and forth the results. might consider
		//	warning the user of this case. e.g.
		//
		//	EVENT_RUN_RESUME - call OPERATION_CALL_UTILITY_FUNCTION
		//	EVENT_RUN_SERVICE - don't call OPERATION_CALL_UTILITY_FUNCTION, repeatedly
		//	(inefficient)
		//
		//	however, this situation is unlikely to be of practical interest since it
		//	would mean the the results from e.g. EVENT_RUN_RESUME would not be used,
		//	unless a special case were used in EVENT_RUN_SERVICE, and this is poor usage
		//	style of BRAHMS anyway.

		/*

		I THINK BASICALLY WE DECIDED NOT TO DO THIS, BECAUSE THE STATE MAY BE CLEARED
		ON AN INTERVENING EVENT THAT THE USER DOES NOT EVEN SERVICE

		if (!stepCall)
		{
			for (UINT32 f=0; f<functions.size(); f++)
			{
				mxArray* cell = functions[f].array;
				for (UINT32 c=0; c<mxGetNumberOfElements(cell); c++)
				{
					mxArray* temp = mxGetCell(cell, c);
					if (temp)
					{
						//cerr << "destroying an arg...\n";
						mxDestroyArray(temp);
						mxSetCell(cell, c, NULL);
					}
				}
			}
		}
		*/


		//	read "operations"
		mxArray* mxOps = mxGetField(mxOutput, 0, "operations");
		if (mxOps)
		{
			if (!mxIsCell(mxOps)) berr << E_NOT_COMPLIANT << "output.operations should be cell array";
			INT32 N = mxGetNumberOfElements(mxOps);

			for (INT32 n=0; n<N; n++)
			{
				mxArray* mxOp = mxGetCell(mxOps, n);
				if (mxOp)
				{
					if (!mxIsCell(mxOp)) berr << E_NOT_COMPLIANT << "each operation should be cell array";
					INT32 Nargs = mxGetNumberOfElements(mxOp);
					if (Nargs < 1) berr << E_NOT_COMPLIANT << "each operation should be non-empty";
					mxArray* mxOpCode = mxGetCell(mxOp, 0);
					if (!mxIsUint32(mxOpCode)) berr << E_NOT_COMPLIANT << "first element of each operation should be an operation code";
					UINT32 opCode = *((UINT32*)mxGetData(mxOpCode));

					switch(opCode)
					{
						case OPERATION_NULL:
						{
							berr << E_NOT_COMPLIANT << "null operation";
						}

/*
						case OPERATION_IIF_CREATE_SET:
						{
							if (Nargs != 2) berr << E_NOT_COMPLIANT << "invalid OPERATION_IIF_CREATE_SET (expects 1 argument)";
							string setName = mxToString(mxGetCell(mxOp, 1), "OPERATION_IIF_CREATE_SET argument 1");
							mxArray* mxSet = addEmptyStruct(mxIif, setName.c_str());
							iif.addSet(setName.c_str());
							bout << "created input set " << setName << D_VERB;

							//	update set structure
							newInputSetAdded(setName.c_str());

							break;
						}

						case OPERATION_OIF_CREATE_SET:
						{
							if (Nargs != 2) berr << E_NOT_COMPLIANT << "invalid OPERATION_OIF_CREATE_SET (expects 1 argument)";
							string setName = mxToString(mxGetCell(mxOp, 1), "OPERATION_OIF_CREATE_SET argument 1");
							oif.addSet(setName.c_str());
							bout << "created output set " << setName << D_VERB;
		
							//	update set structure
							newOutputSetAdded(setName.c_str());

							break;
						}
*/

						case OPERATION_SET_CONTENT:
						{
							if (Nargs != 3) berr << E_NOT_COMPLIANT << "OPERATION_SET_CONTENT expects 2 arguments";
							mxArray* mxPort = mxGetCell(mxOp, 1);
							mxArray* mxData = mxGetCell(mxOp, 2);
							if (!mxIsNumeric(mxData)) berr << E_INVALID_ARG << "OPERATION_SET_CONTENT argument 2 (content) should be numeric";

							//	extract port handle and thence data type
							Symbol hPort = mxGetHandle(mxPort);
							if (!hPort)
								berr << E_INVALID_ARG << "OPERATION_SET_CONTENT argument 1 invalid (should be a port handle, was 0x" << hex << hPort << ")";

							//	get port object
							UINT32 s = 0, p = 0;
							for (s=0; s<oifx.size(); s++)
							{
								Set& set = oifx[s];
								for (p=0; p<set.size(); p++)
								{
									Port& port = *set[p];
									if (port.hPort == hPort)
										break;
								}
								if (p != set.size()) break;
							}
							if (s == oifx.size())
								berr << E_INVALID_ARG << "invalid port handle, hPort == " << hPort;
							Set& set = oifx[s];
							Port& port = *set[p];

							//	check that data is present
							if (!port.hev.event.object)
								berr << E_PORT_EMPTY;

							//	check data type matches
							if (port.type)
							{
								if ((port.type & TYPE_REAL) && mxIsComplex(mxData))
									berr << E_INVALID_ARG << "OPERATION_SET_CONTENT argument 2 (content) should be real";
								if ((port.type & TYPE_COMPLEX) && !mxIsComplex(mxData))
									berr << E_INVALID_ARG << "OPERATION_SET_CONTENT argument 2 (content) should be complex";
								mxClassID clsid = mxGetClassID(mxData);
								TYPE mxType = clsID2dataType(clsid);
								if (mxType != (port.type & TYPE_ELEMENT_MASK))
									berr << E_INVALID_ARG << "OPERATION_SET_CONTENT argument 2 (content) should be numeric type " << type2string(port.type);
							}

							//	make the call
							EventGenericContent eac = {0};
							eac.type = TYPE_PREFERRED_STORAGE_FORMAT;
							eac.bytes = mxGetNumberOfElements(mxData) *  mxGetElementSize(mxData);
							eac.real = mxGetData(mxData);
							eac.imag = mxGetImagData(mxData);
							port.fireCheck(EVENT_GENERIC_CONTENT_SET, 0, &eac);

							//	ok
							break;
						}

						case OPERATION_ADD_PORT:
						{
							if (Nargs < 4 || Nargs > 6) berr << E_NOT_COMPLIANT << "OPERATION_ADD_PORT expects 3 to 5 arguments";
							string setName = mxToString(mxGetCell(mxOp, 1), "OPERATION_ADD_PORT argument 1");
							string className = mxToString(mxGetCell(mxOp, 2), "OPERATION_ADD_PORT argument 2");
							string dataStructure = mxToString(mxGetCell(mxOp, 3), "OPERATION_ADD_PORT argument 3");

							//	optional arg port name
							string portName = "";
							if (Nargs >= 5)
								portName = mxToString(mxGetCell(mxOp, 4), "OPERATION_ADD_PORT argument 4");

							//	optional arg sample rate
							SampleRate sr = time->sampleRate;
							if (Nargs >= 6)
							{
								mxArray* mxArg = mxGetCell(mxOp, 5);
								if (mxIsNumeric(mxArg) && mxGetNumberOfElements(mxArg) == 1)
								{
									DOUBLE srd = mxGetScalar(mxArg);
									if (srd != floor(srd))
										berr << E_INVALID_ARG << "non-integral sample rates should be specified as fractions (e.g. '513/10')";
									if (srd <= 0.0)
										berr << E_INVALID_ARG << "non-positive sample rates are illegal";
									sr.num = srd;
									sr.den = 1;
								}
								else if (mxIsChar(mxArg) && mxGetNumberOfDimensions(mxArg) == 2 && mxGetM(mxArg) == 1)
								{
									char* str = mxArrayToString(mxArg);
									if (!str) berr << E_INVALID_ARG << "OPERATION_ADD_PORT argument 5 was an invalid string";
									string err = stringToSampleRate(str, sr);
									if (err.length()) berr << E_INVALID_ARG << err << " (OPERATION_ADD_PORT argument 5)";
								}
								else berr << E_INVALID_ARG << "OPERATION_ADD_PORT argument 5 should be a scalar numeric or a string";
							}

							//	get set handle
							Symbol hSet = oif.getSet(setName.c_str());
							UINT32 s;
							for (s=0; s<oifx.size(); s++)
								if (oifx[s].hSet == hSet) break;
							if (s == oifx.size()) berr << E_INTERNAL;
							Set& set = oifx[s];
//							UINT32 portIndex = set.size();

							//	create new port
							set.push_back(new Port);
							Port& port = *set.back();

							//	create port
							port.hPort = oif.addPortHEV(hSet, className.c_str(), 0, &port.hev);
//							cout << "hPOrt < " << hex << port.hPort << endl;
							port.name = portName;
							if (portName.length()) oif.setPortName(port.hPort, portName.c_str());
							if (sr.num) oif.setPortSampleRate(port.hPort, sr);
							port.mxPorts = set.mxPorts;

							//	add entry in "input.oif.<set>" holding port handle
							addScalarUINTA(set.mxSet, portName.c_str(), port.hPort);

							//	set structure
							EventGenericStructure eas = {0};
							eas.structure = dataStructure.c_str();
							eas.type = TYPE_PREFERRED_STORAGE_FORMAT;
							port.fireCheck(EVENT_GENERIC_STRUCTURE_SET, 0, &eas);

							//	get numeric type back from object once it has parsed its structure string
							EventGenericForm egf;
							port.fireCheck(EVENT_GENERIC_FORM_ADVANCE, 0, &egf);
							port.type = egf.type;

							//	ok
							continue;
						}

						case OPERATION_BOUT:
						{
							if (Nargs != 3) berr << E_NOT_COMPLIANT << "OPERATION_BOUT expects 2 arguments";
							string out = mxToString(mxGetCell(mxOp, 1), "OPERATION_BOUT argument 1");
							mxArray* mxLevel = mxGetCell(mxOp, 2);
							if (!mxIsUint32(mxLevel)) berr << "OPERATION_BOUT argument 2 should be a detail level constant";
							EnumDetailLevel level = (EnumDetailLevel)*((UINT32*)mxGetData(mxLevel));
							bout << out << level;
							break;
						}

						case OPERATION_GET_UTILITY_OBJECT:
						{
							//	new resource...
							Utility utility;
							utility.handle = utilities.size();

							//	wrapped script is asking for a utility object - let's fetch one, soldier!!!
							if (Nargs != 4) berr << E_NOT_COMPLIANT << "OPERATION_GET_UTILITY_OBJECT expects 3 arguments";
							string cls = mxToString(mxGetCell(mxOp, 1), "OPERATION_GET_UTILITY_OBJECT argument 1");
							UINT16 release = mxToUINT16(mxGetCell(mxOp, 2), "OPERATION_GET_UTILITY_OBJECT argument 2", true);
							utility.identifier = mxToString(mxGetCell(mxOp, 3), "OPERATION_GET_UTILITY_OBJECT argument 3");

							//	check objectID is not in use
							if (!validateID(utility.identifier.c_str()))
								berr << E_INVALID_ARG << "OPERATION_GET_UTILITY_OBJECT, invalid object identifer \"" << utility.identifier << "\"";
							if (mxGetField(mxObjects, 0, utility.identifier.c_str()))
								berr << E_INVALID_ARG << "OPERATION_GET_UTILITY_OBJECT, the object identifier \"" << utility.identifier << "\" is in use";

							//	ask framework to create utility
							EventCreateUtility data;
							data.flags = 0;
							data.hUtility = S_NULL;
							data.spec.cls = cls.c_str();
							data.spec.release = release;
							data.name = NULL;
							data.handledEvent = &utility.hev;

							EngineEvent event;
							event.hCaller = hComponent;
							event.flags = 0;
							event.type = ENGINE_EVENT_CREATE_UTILITY;
							event.data = (void*) &data;

							utility.hUtility = brahms::brahms_engineEvent(&event);
							if (S_ERROR(utility.hUtility)) throw utility.hUtility;

							//	generate the mx arrays we need...
							utility.mxUtility = addEmptyStruct(mxObjects, utility.identifier.c_str());
							addScalarUINT32(utility.mxUtility, "id", utility.handle);

							//	and place the new object in the objects array
							utilities.push_back(utility);

							//	ok
							break;
						}

						case OPERATION_GET_UTILITY_FUNCTION:
						{
							//	new resource...
							UtilityFunction function;
							function.clientHandle = utilityFunctions.size();

							//	wrapped script is asking for a utility function - let's find it one, marine!!!
							if (Nargs != 3) berr << E_NOT_COMPLIANT << "OPERATION_GET_UTILITY_FUNCTION expects 2 arguments";
							UINT32 utilityHandle = mxToUINT32(mxGetCell(mxOp, 1), "OPERATION_GET_UTILITY_FUNCTION argument 1");
							function.identifier = mxToString(mxGetCell(mxOp, 2), "OPERATION_GET_UTILITY_FUNCTION argument 2");

							//	access object resource
							if (utilityHandle >= utilities.size()) berr << E_INVALID_ARG << "OPERATION_GET_UTILITY_FUNCTION, invalid utility handle";
							Utility* utility = &utilities[utilityHandle];
							function.utility = utility;

							//	check objectID is not in use
							if (!validateID(function.identifier.c_str()))
								berr << E_INVALID_ARG << "OPERATION_GET_UTILITY_FUNCTION, invalid function identifer \"" << function.identifier << "\"";
							if (mxGetField(utility->mxUtility, 0, function.identifier.c_str()))
								berr << E_INVALID_ARG << "OPERATION_GET_UTILITY_FUNCTION, the function identifier \"" << function.identifier << "\" is in use";

							//	ask utility object for function
							EventFunctionGet egf = {0};
							egf.name = function.identifier.c_str();
							utility->fireCheck(EVENT_FUNCTION_GET, 0, &egf);
							function.moduleHandle = egf.handle;
							UINT32 count = egf.argumentModifyCount;
							if (!function.moduleHandle) berr << E_NOT_COMPLIANT << "unexpected response from utility object (zero event type)";

							//	generate the mx arrays we need...
							mxArray* temp = addEmptyStruct(utility->mxUtility, function.identifier.c_str());
							addScalarUINT32(temp, "id", function.clientHandle);
							function.mxFunction = addEmptyCell(temp, "output", count); // the important one to keep is the output cell array!

							//	and place the new function in the functions array
							utilityFunctions.push_back(function);

							//	ok
							break;
						}

						case OPERATION_CALL_UTILITY_FUNCTION:
						{
							//	wrapped script wants to kick some function booty woooo!!!! let's plough the way, men!!!!
							if (Nargs != 3) berr << E_NOT_COMPLIANT << "OPERATION_CALL_UTILITY_FUNCTION expects 2 arguments";
							UINT32 functionHandle = mxToUINT32(mxGetCell(mxOp, 1), "OPERATION_CALL_UTILITY_FUNCTION argument 1");
							mxArray* mxArgs = mxGetCell(mxOp, 2);
							if (!mxIsCell(mxArgs)) berr << E_INVALID_ARG << "OPERATION_CALL_UTILITY_FUNCTION argument 2 should be cell array of function arguments";

							//	access function resource
							if (functionHandle >= utilityFunctions.size()) berr << E_INVALID_ARG << "OPERATION_CALL_UTILITY_FUNCTION argument 1, invalid function handle";
							UtilityFunction* function = &utilityFunctions[functionHandle];

							//	prepare the argument list
							vector<Argument> argsx;
							vector<Dims> argdimsx;
							vector<string> argstring;

							UINT32 nArgs = mxGetNumberOfElements(mxArgs);
							for (UINT32 a=0; a<nArgs; a++)
							{
								//	extract ath argument
								mxArray* mxArg = mxGetCell(mxArgs, a);

								//	string args
								if (mxIsChar(mxArg))
								{
									if (mxGetNumberOfDimensions(mxArg) != 2) berr << E_INVALID_ARG << "string argument must be 1xN";
									if (mxGetM(mxArg) != 1) berr << E_INVALID_ARG << "string argument must be 1xN";
									UINT32 N = mxGetN(mxArg);

									Argument arg = {0};

									//	set type
									arg.type = TYPE_CHAR8 | TYPE_REAL | TYPE_CPXFMT_ADJACENT | TYPE_ORDER_COLUMN_MAJOR;

									//	set dims
									Dims dimsx;
									dimsx.push_back(N);
									argdimsx.push_back(dimsx);
									arg.dims.count = 1;

									//	store string
									string s;
									if (N)
									{
										CHAR16* cc = (CHAR16*) mxGetData(mxArg);
										for (UINT32 n=0; n<N; n++)
										{
											CHAR16 c = cc[n];
											if (c > 127) berr << E_INVALID_ARG << "only first half-codepage (ASCII 0-127) can be used in argument strings";
											char c_ = c;
											s = s + c_;
										}
									}
									argstring.push_back(s);

									//	ok
									argsx.push_back(arg);
									continue;
								}

								//	numeric args
								if (!mxIsNumeric(mxArg)) berr << E_INVALID_ARG << "all arguments to function should be numeric";

								Argument arg = {0};
								Dims dimsx;
								INT32 nDims = mxGetNumberOfDimensions(mxArg);
								const MX_DIM* pdims = mxGetDimensions(mxArg);
								for (int i=0; i<nDims; i++)
									dimsx.push_back(pdims[i]);

								arg.dims.count = nDims;
								arg.type = clsID2dataType(mxGetClassID(mxArg)) | TYPE_CPXFMT_ADJACENT | TYPE_ORDER_COLUMN_MAJOR;
								if (mxIsComplex(mxArg))
								{
									arg.type |= TYPE_COMPLEX;
									arg.imag = mxGetImagData(mxArg);
								}
								else
								{
									arg.type |= TYPE_REAL;
								}
								arg.real = mxGetData(mxArg);

								argdimsx.push_back(dimsx);
								argstring.push_back("");
								argsx.push_back(arg);
							}

							for (UINT32 a=0; a<nArgs; a++)
							{
								//	set pointers *after* making all vector extensions
								INT32 nDims = argdimsx[a].size();
								if (nDims) argsx[a].dims.dims = &argdimsx[a][0];
								else argsx[a].dims.dims = 0;

								//	fix up pointers to strings
								UINT32 sLength = argstring[a].length();
								if (sLength) argsx[a].real = (void*) argstring[a].c_str();
							}

							//	make the call
							EventFunctionCall ecf = {0};
							ecf.handle = function->moduleHandle;
							ecf.argumentCount = nArgs;
							std::vector<Argument*> tempargs;
							for (UINT32 a=0; a<nArgs; a++)
								tempargs.push_back(&argsx[a]);
							ecf.arguments = &tempargs[0];
							Symbol err = function->utility->fireNoCheck(EVENT_FUNCTION_CALL, 0, &ecf);

							if (S_ERROR(err))
							{
								if (err == E_BAD_ARG_COUNT)
									berr << E_BAD_ARG_COUNT << "expected " << ecf.argumentCount << " arguments";

								if (err == E_BAD_ARG_SIZE)
									berr << E_BAD_ARG_SIZE << "argument " << (ecf.argumentCount + 1);

								if (err == E_BAD_ARG_TYPE)
									berr << E_BAD_ARG_TYPE << "argument " << (ecf.argumentCount + 1);

								throw err;
							}

							if (err == S_NULL) berr << E_NOT_SERVICED << "the utility did not service EVENT_FUNCTION_CALL";

							//	place the outputs (list of all modified args: additional
							//	args returned will be flagged as modified, too)

							//	first, count them
							UINT32 count = 0;
							for (UINT32 a=0; a<argsx.size(); a++)
								if (argsx[a].flags & F_MODIFIED) count++;

							//	next, make sure we've got the right number of output cells
							if (mxGetNumberOfElements(function->mxFunction) != count)
								berr << E_NOT_COMPLIANT << "called function modified a different number of arguments than it promised it would";

							//	for each array, check the size, resize if necessary, and fill it
							count = 0;
							for (UINT32 a=0; a<argsx.size(); a++)
							{
								if (argsx[a].flags & F_MODIFIED)
								{
									//	get argument (note that according to the protocol of
									//	EVENT_FUNCTION_CALL, the *contents* of the argument
									//	could have changed completely, with the called function
									//	pointing them at different things, so we will assume
									//	this is the case)
									Argument& arg = argsx[a];
									Dims dims(arg.dims);

									//	get cell that will return the output for this argument
									//	to the script
									mxArray* cell = mxGetCell(function->mxFunction, count);

									//	now, in the interests of efficiency, we only need to
									//	recreate the cell that will hold the output if it is
									//	in any way non-concommitant with the data we need to
									//	sling into it. we will recreate it if...
									bool recreate = true;
									while(true)
									{
										//	...there *is no* cell
										if (!cell) break;

										//	...its numeric type is incorrect
										TYPE cellType = mxGetType(cell);
										if (arg.type != cellType) break;

										//	...its dimensions are not right
										//	NOTE: we could in some cases just do a resize here! (but we don't)
										Dims cellDims = mxGetDims(cell);
										if (dims != cellDims) break;

										//	ok, happy to keep the one we've got
										recreate = false;
										break;
									}

									//	recreate?
									if (recreate)
									{
										if (cell) mxDestroyArray(cell);
										MX_DIMS mxDims;
										for (UINT32 d=0; d<dims.size(); d++)
											mxDims.push_back(dims[d]);
										cell = mxCreateNumericArray(mxDims.size(), &mxDims[0],
											genericNumericType2clsID(arg.type), (arg.type & TYPE_COMPLEX) ? mxCOMPLEX : mxREAL);
										mxSetCell(function->mxFunction, count, cell);
									}

									//	copy stuff across
									UINT32 bytes = dims.getNumberOfElements() * TYPE_BYTES(arg.type);
									if (bytes) memcpy(mxGetData(cell), arg.real, bytes);
									if (bytes && (arg.type & TYPE_COMPLEX)) memcpy(mxGetImagData(cell), arg.imag, bytes);

									//	increment
									count++;
								}
							}

							//	ok
							break;
						}

/*
						case OPERATION_GET_PROCESS_STATE:
						{
							//	wrapped script wants to access a peer's StateML (flags arg is optional)
							if (Nargs != 3 && Nargs != 4) berr << E_NOT_COMPLIANT << "invalid OPERATION_GET_PROCESS_STATE (expects 3 or 4 arguments)";
							string relativeIdentifier = mxToString(mxGetCell(mxOp, 1), "OPERATION_GET_PROCESS_STATE argument 1");
							string expectedClassName = mxToString(mxGetCell(mxOp, 2), "OPERATION_GET_PROCESS_STATE argument 2");
							UINT32 flags = 0;
							if (Nargs == 4)
								flags = mxToUINT32(mxGetCell(mxOp, 3), "OPERATION_GET_PROCESS_STATE argument 3");

							//	make the call...
							GetProcessState gps = {0};
							gps.hCaller = hComponent;
							gps.identifier = relativeIdentifier.c_str();
							gps.spec.cls = expectedClassName.c_str();
							gps.spec.release = 0;
							gps.flags = flags;
							Symbol processState = getProcessState(&gps);
							if (S_ERROR(processState)) berr << processState;

							//	field name is relative identifier, grepped...
							grep(relativeIdentifier, "/", "_");

							//	now we just need to drop it into persist/states array...
							mxArray* temp = getVariable(strings.varPersist.c_str());
							mxArray* st = mxGetField(temp, 0, "states");
							if (!st)
								berr << E_NOT_COMPLIANT << "you removed the persist.states array, and then asked me to OPERATION_GET_PROCESS_STATE something into it - you're a bad person";
							int fieldNumber = mxGetFieldNumber(st, relativeIdentifier.c_str());
							if (fieldNumber == -1)
							{
								fieldNumber = mxAddField(st, relativeIdentifier.c_str());
								if (fieldNumber == -1) berr << E_INTERNAL;
							}
							mxArray* current = mxGetFieldByNumber(st, 0, fieldNumber);
							if (current)
							{
								mxSetFieldByNumber(st, 0, fieldNumber, NULL);
								mxDestroyArray(current);
							}

							XMLNode xmlnode(processState);
							DataMLNode mnode(&xmlnode);
							mxArray* matState = dataMLToMat(mnode);
							mxSetFieldByNumber(st, 0, fieldNumber, matState);
							putVariable(strings.varPersist.c_str(), temp);
							mxDestroyArray(temp);

							//	ok
							break;
						}
						*/

						case OPERATION_GET_RANDOM_SEED:
						{
							VUINT32 seed;

							if (Nargs == 1)
							{
								//	send back as many as are available
								seed = getRandomSeed();
							}
							else if (Nargs == 2)
							{
								//	send back as many as were requested
								UINT32 count = mxToUINT32(mxGetCell(mxOp, 1), "OPERATION_GET_RANDOM_SEED argument 1");
								seed = getRandomSeed(count);
							}
							else berr << E_NOT_COMPLIANT << "OPERATION_GET_RANDOM_SEED expects 0 or 1 arguments";

							//	if no field exists in persist, create it
							mxArray* temp = getVariable(strings.varPersist.c_str());
							int f = mxGetFieldNumber(temp, "seed");
							mxArray* mxSeed;
							if (f == -1)
							{
								MX_DIMS dims;
								dims.push_back(seed.size());
								mxSeed = mxCreateNumericArray(1, &dims[0], mxUINT32_CLASS, mxREAL);
								int f = mxAddField(temp, "seed");
								mxSetFieldByNumber(temp, 0, f, mxSeed);
							}
							else
							{
								mxSeed = mxGetFieldByNumber(temp, 0, f);
								if (mxGetM(mxSeed) != seed.size())
								{
									mxDestroyArray(mxSeed);
									MX_DIMS dims;
									dims.push_back(seed.size());
									mxSeed = mxCreateNumericArray(1, &dims[0], mxUINT32_CLASS, mxREAL);
									mxSetFieldByNumber(temp, 0, f, mxSeed);
								}
							}

							//	copy
							UINT32* pseed = (UINT32*)mxGetData(mxSeed);
							for (UINT32 i=0; i<seed.size(); i++)
								pseed[i] = seed[i];

							putVariable(strings.varPersist.c_str(), temp);
							mxDestroyArray(temp);

							break;
						}

						default:
							berr << E_NOT_COMPLIANT << "unrecognised operation";
					}
				}
			}
		}

		//	if not continuation, don't do any more
		if (!*p_eventCont) break;

	}
}





////////////////////////////////////////////////////////////////
//
//	DATAML TO MAT
//
////////////////////////////////////////////////////////////////

mxArray* COMPONENT_CLASS_CPP::dataMLToMat(DataMLNode& node)
{
	//	switch on type of DataML node
	TYPE type = node.getElementType();
	switch (type)
	{
		case TYPE_STRUCT:
		{
			mxArray* mxRet = mxCreateStructMatrix(1, 1, 0, 0);
			VSTRING children = node.getFieldNames();
			for (UINT32 c=0; c<children.size(); c++)
			{
				DataMLNode child = node.getField(children[c].c_str());
				INT32 field = mxAddField(mxRet, children[c].c_str());
				mxArray* temp = dataMLToMat(child);
				mxSetFieldByNumber(mxRet, 0, field, temp);
			}
			return mxRet;
		}

		case TYPE_CELL:
		{
			//	access array metadata
			Dims b_dims = node.getDims();
			UINT32 count = node.getNumberOfElementsReal();

			//	convert dims if necessary
			MX_DIMS dims;
			for (UINT32 d=0; d<b_dims.size(); d++)
				dims.push_back(b_dims.at(d));

			//	create return array
			mxArray* mxRet = mxCreateCellArray(dims.size(), &dims[0]);

			//	fill children
			for (UINT32 c=0; c<count; c++)
			{
				DataMLNode child = node.getCell(c);
				mxSetCell(mxRet, c, dataMLToMat(child));
			}
			return mxRet;
		}

		default:
		{
			//	don't worry: dataType2clsID() will throw if type is not a matlab numeric

			//	access array metadata
			Dims b_dims = node.getDims();
			bool complex = node.getComplexity();
//			UINT32 count = node.getNumberOfElementsReal();
			UINT32 bytes = node.getNumberOfBytesReal();

			//	convert dims if necessary
			MX_DIMS dims;
			for (UINT32 d=0; d<b_dims.size(); d++)
				dims.push_back(b_dims.at(d));

			//	create return array
			mxArray* mxRet = mxCreateNumericArray(
				dims.size(), &dims[0], dataType2clsID(type), complex ? mxCOMPLEX : mxREAL);

			//	do the blat (mxGetPi() will return NULL if there's no complex data)
			if (bytes) node.getRaw((BYTE*)mxGetPr(mxRet), (BYTE*)mxGetPi(mxRet));

			//	ok
			return mxRet;
		}
	}

	//	unhandled type
	berr << "unhandled data type in dataMLToMat()";
}




////////////////////////////////////////////////////////////////
//
//	EVENT
//
////////////////////////////////////////////////////////////////

ComponentInfo* COMPONENT_CLASS_CPP::getThisComponentInfo()
{

	//	get info from wrapped function
	*p_eventType = EVENT_MODULE_INIT;
	*p_eventFlags = 0;
	startEngine();
	fireWrappedFunction(false);

	//	check was processed
	if (response != C_OK) berr << E_NOT_SERVICED << "EVENT_MODULE_INIT";

	//	look for info field in output
	mxArray* mxInfo = mxGetField(mxOutput, 0, "info");
	if (!mxInfo || !mxIsStruct(mxInfo) || (mxGetNumberOfElements(mxInfo) != 1))
		berr << E_NOT_COMPLIANT << "invalid: output.info";

	//	get all fields
	int N = mxGetNumberOfFields(mxInfo);
	for (int n=0; n<N; n++)
	{
		string key = mxGetFieldNameByNumber(mxInfo, n);
		mxArray* mxField = mxGetFieldByNumber(mxInfo, 0, n);

		if (key == "component")
		{
			//	should be two-element row vector
			if (!mxIsDouble(mxField) || (mxGetNumberOfElements(mxField) != 2))
				____NOT_COMPLIANT("EVENT_INFO", "invalid: info.component");

			double* p = mxGetPr(mxField);
			versionComponent.release = UINT16(p[0]);
			versionComponent.revision = UINT16(p[1]);
		}

		else if (key == "flags")
		{
			//	should be scalar UINT32
			if (!mxIsUint32(mxField) || (mxGetNumberOfElements(mxField) != 1))
				____NOT_COMPLIANT("EVENT_INFO", "invalid: info.flags");

			componentInfo.flags |= *(UINT32*)mxGetData(mxField);
		}

		else if (key == "additional")
		{
			//	add to info array
			if (!mxIsChar(mxField))
				____NOT_COMPLIANT("EVENT_INFO", "invalid: info.additional");

			char buf[1024] = "";
			mxGetString(mxField, buf, 1023);
			info.additional = buf;
			while(1)
			{
				string::size_type f = info.additional.find("\\n");
				if (f == string::npos) break;
				info.additional = info.additional.substr(0, f) + "\n" + info.additional.substr(f+2);
			}
			componentInfo.additional = info.additional.length() ? info.additional.c_str() : NULL;
		}

		else if (key == "libraries")
		{
			//	add to info array
			if (!mxIsChar(mxField))
				____NOT_COMPLIANT("EVENT_INFO", "invalid: info.libraries");

			char buf[1024] = "";
			mxGetString(mxField, buf, 1023);
			info.libraries = buf;
			componentInfo.libraries = info.libraries.length() ? info.libraries.c_str() : NULL;
		}

		else
		{
			____NOT_COMPLIANT("EVENT_INFO", "unrecognised field \"info." << key << "\"");
		}
	}

	//	also get matlab version
	string ver_get = strings.varVM + " = version; " + strings.varVJ + " = version('-java');";
	engEvalString(ENGINE, ver_get.c_str());
	mxArray* temp = engGetVariable(ENGINE, strings.varVM.c_str());
	if (temp)
	{
		string vm = mxToString(temp, "vm");
		info.libraries += "MATLAB=" + vm + "\n";
	}
	else info.libraries += "MATLAB=error getting version\n";
	temp = engGetVariable(ENGINE, strings.varVJ.c_str());
	if (temp)
	{
		string vj = mxToString(temp, "vj");
		info.libraries += "MATLAB_JAVA=" + vj + "\n";
	}
	else info.libraries += "MATLAB_JAVA=error getting version\n";
	ver_get = "clear " + strings.varVM + " " + strings.varVJ;
	engEvalString(ENGINE, ver_get.c_str());

	info.libraries += "MX_API_VER=" + h2s(MX_API_VER) + "\n";
	componentInfo.libraries = info.libraries.length() ? info.libraries.c_str() : NULL;

	//	ok
	return &componentInfo;
}


Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	//	set in event type and flags to mxEvent
	if (p_eventType) *p_eventType = event->type;
	if (p_eventFlags) *p_eventFlags = event->flags;

	//	switch on event type
	switch(event->type)
	{

		case EVENT_STATE_SET:
		{
			//	empty iif & oif
			mxIif = addEmptyStruct(mxInput, "iif");
			mxOif = addEmptyStruct(mxInput, "oif");

			//	create default sets on iifx and oifx
			newOutputSetAdded("");
			newInputSetAdded("");

			//	get data
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);

			//	get parameters
			DataMLNode nodeState(&xmlNode);

			//	start engine
			startEngine();

			//	create persistent variable
			/*

				the function in matlab may need a persistent state, and rather
				than pipe it back and forth from the matlab engine, we generate
				it in the engine, and just pass it in to the function and out
				again. if even this is an overhead, the function can maintain
				its own persistent state, but it will have to be careful since
				it may be called in multiple instances within the same engine.
				therefore in general, it is recommended that the provided mechanism
				is used rather than something bespoke.

			*/

			//	process
			mxArray* mxProcess = addEmptyStruct(mxPersist, "process");
			addString(mxProcess, "name", componentData->name);

			//	state
			int fieldNumber = mxAddField(mxPersist, "state");
			mxArray* temp = dataMLToMat(nodeState);
			mxSetFieldByNumber(mxPersist, 0, fieldNumber, temp);

			//	put persistent variable
			putVariable(strings.varPersist.c_str(), mxPersist);

			//	time
			mxTime = addEmptyStruct(mxInput, "time");
			temp = addEmptyStruct(mxTime, "baseSampleRate");
			p_baseSampleRate_num = addScalarUINT64(temp, "num", time->baseSampleRate.num);
			p_baseSampleRate_den = addScalarUINT64(temp, "den", time->baseSampleRate.den);
			p_baseSampleRate_rate = addScalarDOUBLE(temp, "rate", sampleRateToRate(time->baseSampleRate));
			p_baseSampleRate_period = addScalarDOUBLE(temp, "period", 1.0 / sampleRateToRate(time->baseSampleRate));
			p_executionStop = addScalarUINT64(mxTime, "executionStop", time->executionStop);
			temp = addEmptyStruct(mxTime, "sampleRate");
			p_sampleRate_num = addScalarUINT64(temp, "num", time->sampleRate.num);
			p_sampleRate_den = addScalarUINT64(temp, "den", time->sampleRate.den);
			p_sampleRate_rate = addScalarDOUBLE(temp, "rate", sampleRateToRate(time->sampleRate));
			p_sampleRate_period = addScalarDOUBLE(temp, "period", 1.0 / sampleRateToRate(time->sampleRate));
			p_samplePeriod = addScalarUINT64(mxTime, "samplePeriod", time->samplePeriod);
			p_now = addScalarUINT64(mxTime, "now", time->now);

			//	call matlab
			fireWrappedFunction(false);

			//	ok
			return response;
		}

		case EVENT_INIT_PRECONNECT:
		{
			//	generate iifx ports
			for (UINT32 s=0; s<iifx.size(); s++)
			{
				//	get set
				Set& set = iifx[s];

				//	resize set
				set.resize(iif.getNumberOfPorts(set.hSet));

				//	create child fields
				set.mxIndex = addEmptyStruct(set.mxSet, "index");
				set.mxPorts = addNonScalarStruct(set.mxSet, "ports", set.size());

				//	add fields to ports field
				mxAddField(set.mxPorts, "name");
				mxAddField(set.mxPorts, "class");
				mxAddField(set.mxPorts, "structure");
				mxAddField(set.mxPorts, "data");

				//	for each port in set, add a port entry to set mxArray
				for (UINT32 p=0; p<set.size(); p++)
				{
					//	create port
					set[p] = new Port;
					Port& port = *set[p];
					port.hPort = iif.getPortHEV(set.hSet, p, &port.hev);
//							cout << "hPOrt < " << hex << port.hPort << endl;
					port.name = iif.getPortName(port.hPort);
					port.mxPorts = set.mxPorts;

					//	stick it into mxIif object
					mxArray* mxName = mxCreateString(port.name.c_str());
					mxSetField(set.mxPorts, p, "name", mxName);

					//	make entry in index:
					//	if two ports in a set have the same name, they will overwrite, and we want
					//	to (by convention) index the first not the last, so we only
					//	do this if the index doesn't already have such an entry...
					if (!mxGetField(set.mxIndex, 0, port.name.c_str()))
						addScalarUINTA(set.mxIndex, port.name.c_str(), p + 1); // one-based
				}
			}

			//	call matlab
			fireWrappedFunction(false);

			//	ok
			return response;
		}

		case EVENT_INIT_CONNECT:
		{
			//	make all inputs available
			makeAllInputsAvailable();

			//	call matlab
			fireWrappedFunction(false);
		
			//	ok
			return response;
		}

		case EVENT_INIT_POSTCONNECT:
		{
			//	update time data (base sample rate is now settled)
			*p_baseSampleRate_num = time->baseSampleRate.num;
			*p_baseSampleRate_den = time->baseSampleRate.den;
			*p_baseSampleRate_rate = sampleRateToRate(time->baseSampleRate);
			*p_baseSampleRate_period = 1.0 / sampleRateToRate(time->baseSampleRate);
			*p_executionStop = time->executionStop;
			*p_samplePeriod = time->samplePeriod;

			//	call matlab
			fireWrappedFunction(false);

			//	ok
			return response;
		}

		case EVENT_STATE_GET:
		{
			//	need to offer current state to process, and have it return
			//	its updated state. but this requires a translation to/from
			//	DataML, and i haven't coded it yet

			//	"not serviced", rather than "no update to make"
			return S_NULL;
		}

		case EVENT_RUN_SERVICE:
		{
			//	set step bounds
			*p_now = time->now;

			//	make all inputs available
			makeAllInputsAvailable();

			//	call matlab
			fireWrappedFunction(true);

			//	ok
			return response;
		}

		case EVENT_BEGIN_RUNPHASE:
		{
			//	call matlab
			fireWrappedFunction(false);

			//	we're moving out of the caller thread, so take a copy of the persist variable
			clearAndNullArray(mxPersist);
			mxPersist = getVariable(strings.varPersist.c_str());

			//	ok
			return response;
		}

		case EVENT_RUN_PLAY:
		{
			//	start run-thread engine
			startEngine();

			//	so now we have to copy the persist variable over to the
			//	run-phase engine instance (we got a copy during EVENT_INIT_POSTCONNECT)
			putVariable(strings.varPersist.c_str(), mxPersist);

			//	call matlab
			fireWrappedFunction(false);

			//	ok
			return response;
		}

		case EVENT_LOG_INIT:
		case EVENT_LOG_TERM:
		case EVENT_RUN_STOP:
		{
			//	call matlab
			fireWrappedFunction(false);

			//	ok
			return response;
		}

		case EVENT_RUN_PAUSE:
		{
			//	call matlab
			fireWrappedFunction(false);

#ifdef __CHECK_TIMING__
			t_run = timer.elapsed() - t_run;
			cerr << "total run-phase time: " << t_run << endl;
			cerr << "total run-phase time spent calling matlab: " << t_run_matlab << endl;
#endif

			//	ok
			return response;
		}

		case EVENT_RUN_RESUME:
		{
			//	call matlab
			fireWrappedFunction(false);

#ifdef __CHECK_TIMING__
			t_run = timer.elapsed();
			t_run_matlab = 0.0;
#endif

			//	ok
			return response;
		}

		case EVENT_BEGIN_TERMPHASE:
		{
			//	ok
			return C_OK;
		}

/*
		case EVENT_THREAD_EXIT:
		{
			closeEngine(true);

			//	ok
			return C_OK;
		}
*/

		default:
		{
			//	make sure we're handling all possible events
			berr << E_INTERNAL << "event not handled " << getSymbolString(event->type);
			return E_ERROR;
		}

	}

}

BRAHMS_DLL_EXPORT Symbol EventHandler(Event* event)
{
	try
	{
		switch (event->type)
		{
			case EVENT_MODULE_QUERY:
			{
				brahms::EventModuleQuery* query = (brahms::EventModuleQuery*) event->data;
				query->interfaceID = N_BRAHMS_INTERFACE;
				return C_OK;
			}

			case EVENT_MODULE_INIT:
			{
				EventModuleInit* init = (EventModuleInit*) event->data;
				executionInfo = init->executionInfo;
				init->moduleInfo = &MODULE_MODULE_INFO;

				//	extract execution parameters
				XMLNode xmlNode(executionInfo->executionParameters);
				//execDevMode = xmlNode.getChild("DevelopmentMode")->nodeText() == string("1");
				//execMultiVoice = xmlNode.getChild("MultiVoice")->nodeText() == string("1");
				VoiceID = xmlNode.getChild("VoiceID")->nodeText();

				return C_OK;
			}

			case EVENT_MODULE_TERM:
			{
				//EventModuleTerm* term = (EventModuleTerm*) event->data;
				
				//	close matlab engine on platforms that use a module-level engine
				CloseGlobalEngine();
				
				return C_OK;
			}

			case EVENT_MODULE_CREATE:
			{
				EventModuleCreateBindings* emc = (EventModuleCreateBindings*) event->data;

				COMPONENT_CLASS_CPP* newObject = new COMPONENT_CLASS_CPP(emc);
				event->object = newObject;

				//	prepare execution strings
				string suffix = "__" + VoiceID + "_" + emc->data->name;
				grep(suffix, "/", "_");
				grep(suffix, "-", "_");
				newObject->buildStrings(suffix);

				newObject->initialize(emc->hComponent, emc->data);
				emc->info = newObject->getThisComponentInfo();

				return C_OK;
			}

			case EVENT_MODULE_DESTROY:
			{
				//Sleep(10000);

				delete (COMPONENT_CLASS_CPP*) event->object;
				return C_OK;
			}

			case EVENT_MODULE_DUPLICATE:
			{
				return attachError(E_NOT_IMPLEMENTED, "cannot duplicate matlab process");
			}

			default:
			{
				COMPONENT_CLASS_CPP* object = (COMPONENT_CLASS_CPP*) event->object;
				if (!object) return E_NO_INSTANCE;
				return object->event(event);
			}
		}
	}

	catch(Symbol s)
	{
		return s;
	}

	catch(const char* e)
	{
		return attachError(E_USER, e);
	}

	catch(std::exception se)
	{
		Symbol e = attachError(E_STD, se.what());
		stringstream ss;
		ss << "at " << __FILE__ << ":" << __LINE__;
		e = attachError(e, ss.str().c_str(), F_DEBUGTRACE);
		return e;
	}

	catch(...)
	{
		return E_UNRECOGNISED_EXCEPTION;
	}
}


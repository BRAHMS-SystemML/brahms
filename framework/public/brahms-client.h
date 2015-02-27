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

	$Id:: brahms-client.h 2361 2009-11-12 18:39:39Z benjmitch  $
	$Rev:: 2361                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-12 18:39:39 +0000 (Thu, 12 Nov 2009)       $
________________________________________________________________

*/




#ifndef INCLUDED_BRAHMS_CLIENT_INTERFACE
#define INCLUDED_BRAHMS_CLIENT_INTERFACE



////////////////////////////////////////////////////////////////////////////////
//
//	"brahms-client.h" defines the Client Interface, which is the
//	interface to the Engine used by Clients. Thus, it is included in builds
//	of Clients and the Engine itself.
//
////////////////////////////////////////////////////////////////////////////////


////////////////    PRE-AMBLE

	//  BRAHMS_BUILD_TARGET must be defined
	#ifndef BRAHMS_BUILD_TARGET
	#define BRAHMS_BUILD_TARGET ENGINE_CLIENT
	#endif



////////////////	INCLUDE COMPONENT INTERFACE

	#include "brahms-component.h"



////////////////    START NAMESPACE / EXTERN C

#ifdef __cplusplus
namespace brahms { extern "C" {
#endif



////////////////	ADDITIONAL CONSTANTS

	typedef UINT32 VoiceIndex;
	typedef UINT32 VoiceCount;

	const VoiceIndex VOICE_MASTER = 0;
	const VoiceIndex VOICE_UNDEFINED = 0xFFFFFFFF;
	const VoiceIndex VOICE_FROM_MPI = 0xFFFFFFFE;

	const VoiceCount VOICECOUNT_UNDEFINED = 0;

	const UINT32 F_PAUSE_ON_EXIT				= 0x00000001;
	const UINT32 F_MAXERR						= 0x00000002;

	const Symbol C_BASE_MONITOR_EVENT			= C_BASE_ENGINE_EVENT + 0x1000;

	const Symbol EVENT_MONITOR_OPEN				= C_BASE_MONITOR_EVENT + 0x01;
	const Symbol EVENT_MONITOR_CLOSE			= C_BASE_MONITOR_EVENT + 0x02;
	const Symbol EVENT_MONITOR_SHOW				= C_BASE_MONITOR_EVENT + 0x03;
	const Symbol EVENT_MONITOR_CANCEL			= C_BASE_MONITOR_EVENT + 0x04; // notify that execution is cancelling

	const Symbol EVENT_MONITOR_PHASE			= C_BASE_MONITOR_EVENT + 0x11; // update on the phase being performed (init, run, term)
	const Symbol EVENT_MONITOR_OPERATION		= C_BASE_MONITOR_EVENT + 0x12; // update on the operation being performed within that phase
	const Symbol EVENT_MONITOR_SERVICE			= C_BASE_MONITOR_EVENT + 0x13; // update on progress during run-phase
	const Symbol EVENT_MONITOR_PROGRESS			= C_BASE_MONITOR_EVENT + 0x14; // update on progress during other operations



////////////////	ENGINE API

	struct MonitorTime : public ComponentTime
	{
		BaseSamples nowAdvance;
		DOUBLE wallclockTime;
	};

	enum ExecPhase
	{
		EP_NULL = 0,
		EP_OPEN,
		EP_EXECUTE,
		EP_CLOSE
	};

	enum WalkLevel
	{
		WL_NULL = 0,
		WL_LIST,
		WL_LOAD_NATIVE,
		WL_LOAD_ALL
	};

	struct CreateEngine;

	struct MonitorEvent
	{
		Symbol type;
		CreateEngine* createEngine;
		ExecPhase phase; // subject of EVENT_MONITOR_PHASE
		const char* message; // subject of EVENT_MONITOR_PHASE, EVENT_MONITOR_OPERATION
		MonitorTime* monitorTime; // subject of EVENT_MONITOR_SERVICE
		DOUBLE progressMin; // subject of EVENT_MONITOR_PROGRESS
		DOUBLE progressMax; // subject of EVENT_MONITOR_PROGRESS
	};

	//	declaration of progress monitor callback function
	typedef Symbol (*MonitorEventHandler) (const MonitorEvent* event);

	enum TextFormat
	{
		FMT_NULL = 0,
		FMT_TEXT = 1,
		FMT_XML = 2
	};

	const UINT32 CREATEENGINE_MSG_LENGTH = 256;

	struct CreateEngine
	{
		VoiceCount voiceCount;					//	VOICECOUNT_UNDEFINED, 1, 2, ...
		VoiceIndex voiceIndex;					//	(zero-based) 0, 1, ..., (voiceCount-1); VOICE_FROM_MPI; VOICE_UNDEFINED
		const char* logFilename;				//	if non-NULL, log to this file (rather than stdout)
		const char* logLevels;					//	log level control in form "L-T-S;L-T-S;\0"
		TextFormat logFormat;					//	log format (XML or text)
		MonitorEventHandler handler;			//	callback function to monitor progress
		UINT32 engineFlags;						//	engine flags
		const char* executionFilename;			//	filename of Execution File, or NULL if none
		const char* executionParameters;		//	exec pars in form "X=Y;X=Y;\0"
		char errorMessage[CREATEENGINE_MSG_LENGTH];
	};

	BRAHMS_ENGINE_VIS Symbol engine_create(CreateEngine* createEngine);
	BRAHMS_ENGINE_VIS Symbol engine_up(Symbol engine);
	BRAHMS_ENGINE_VIS Symbol engine_down(Symbol engine, UINT32* messageCount);
	BRAHMS_ENGINE_VIS Symbol engine_destroy(Symbol engine);

	BRAHMS_ENGINE_VIS Symbol engine_walk(Symbol hEngine, WalkLevel walkLevel);

	BRAHMS_ENGINE_VIS Symbol engine_open(Symbol hEngine);
	BRAHMS_ENGINE_VIS Symbol engine_execute(Symbol hEngine);
	BRAHMS_ENGINE_VIS Symbol engine_close(Symbol hEngine);



////////////////    END EXTERN C / NAMESPACE

#ifdef __cplusplus
} }
#endif



////////////////    CERR MACROS

	#define ____INFO(msg) { stringstream ss; ss << "BRAHMS (INFO): " << msg << endl; string s = ss.str(); cerr << s.c_str(); }
	#define ____WARN(msg) { stringstream ss; ss << "BRAHMS (WARN): " << msg << endl; string s = ss.str(); cerr << s.c_str(); }
	#define ____FAIL(msg) { stringstream ss; ss << "BRAHMS (FAIL): " << msg << endl; string s = ss.str(); cerr << s.c_str(); }



////////////////	INCLUSION GUARD

#endif

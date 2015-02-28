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

	$Id:: execution.h 2451 2010-01-25 16:48:58Z benjmitch      $
	$Rev:: 2451                                                $
	$Author:: benjmitch                                        $
	$Date:: 2010-01-25 16:48:58 +0000 (Mon, 25 Jan 2010)       $
________________________________________________________________

*/



#ifndef INCLUDED_ENGINE_EXECUTION
#define INCLUDED_ENGINE_EXECUTION

#include "channel.h"
#include "support/xml.h"
#include <vector>
#include <string>

using std::vector;
using std::string;

////////////////	NAMESPACE

namespace brahms
{



	////////////////	GROUP (OF PROCESSES)

		struct Group
		{
			Group()
			{
				voice = VOICE_UNDEFINED;
				members = 0;
			}

                        vector<string> ident;
			VoiceIndex voice;
			UINT32 members;
		};



	////////////////	PEER VOICE DATA

		struct Voice
		{
			Voice()
			{
				protocol = brahms::channel::PROTOCOL_NULL;
			}

			//	best contact channel to peer voice from local voice
			brahms::channel::Protocol protocol;
			string address;
		};



	////////////////	LOG MODE

		const BaseSamples BASE_SAMPLES_INF = 0xFFFFFFFFFFFFFFFFULL;

		struct RepeatWindowSeconds
		{
			RepeatWindowSeconds()
			{
				//	default to all-time
				t0 = 0.0;
				T = 0.0; // means, window has no step
				t1 = -1.0; // means, window has no end
			}

			//	t0:T:t1 (first origin is at t0, next at t0+T, origin at t1 is not included)
			DOUBLE t0;
			DOUBLE T;
			DOUBLE t1; // t1 of 0.0 means that window has no end
		};

		struct LogWindowSeconds
		{
			DOUBLE t0;
			DOUBLE t1;
		};

		struct LogOriginSeconds
		{
			//	repeat windows
			RepeatWindowSeconds repeat;

			//	log windows within each repeat window
			vector<LogWindowSeconds> windows;
		};

		struct LogWindowBaseSamples
		{
			BaseSamples t0;
			BaseSamples t1;
		};

		struct LogOriginBaseSamples
		{
			//	t0:T:t1 (first origin is at t0, next at t0+T, origin at t1 is not included)
			BaseSamples t0;
			BaseSamples T;
			BaseSamples t1;

			//	log windows
			vector<LogWindowBaseSamples> windows;
		};

		struct LogMode
		{
			LogMode()
			{
				precision = PRECISION_DO_NOT_LOG;
				encapsulated = false;
				recurse = true;
			}

			LogMode(INT32 p_precision, bool p_encapsulated, bool p_recurse, vector<LogOriginSeconds>& p_origins)
			{
				precision = p_precision;
				encapsulated = p_encapsulated;
				recurse = p_recurse;
				origins = p_origins;
			}

			INT32 precision;
			bool encapsulated;
			bool recurse;
			vector<LogOriginSeconds> origins;
		};



	////////////////	LOG RULE

		struct LogRule
		{
			LogRule()
			{
			}

			LogRule(string p_name, INT32 p_precision, bool p_encapsulated, bool p_recurse, vector<LogOriginSeconds>& p_origins)
			{
				name = p_name;
				mode = LogMode(p_precision, p_encapsulated, p_recurse, p_origins);
			}

			string name;
			LogMode mode;
			vector<string> logged;	//	list of items that were logged according to this rule
		};



	////////////////	EXECUTION

		class Execution
		{

		public:

			//	tors
			Execution();
			~Execution();

			//	interface
			void load(string path, VoiceCount* voiceCount, VoiceIndex* voiceIndex, brahms::output::Source& fout);
			brahms::xml::XMLNode* prepareReportFile(const char* ExecutionID, const char* VoiceID, INT32 voiceIndex, INT32 voiceCount);

			//	parameters
			string					name;
			double					executionStop;
			bool					timing;
			VUINT32					seed;
			string					fileSysIn;
			string					fileSysOut;
			string					fileReport;			//	name of output file (ReportML)
			string					workingDirectory;
			vector<Group>			groups;
			vector<Voice>			voices;

			//	logs
			LogMode					defaultLogMode;
			vector<LogRule>			logRules;

			//	Execution File
			brahms::xml::XMLNode	nodeExecution;

			//	Report File
			brahms::xml::XMLNode	nodeReport;

		};


}



////////////////	INCLUSION GUARD

#endif

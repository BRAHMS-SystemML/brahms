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

	$Id:: output.h 2379 2009-11-16 01:58:12Z benjmitch         $
	$Rev:: 2379                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-16 01:58:12 +0000 (Mon, 16 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_FRAMEWORK_BASE_OUTPUT
#define INCLUDED_BRAHMS_FRAMEWORK_BASE_OUTPUT

#include <stack>
using std::stack;
#include <string>
using std::string;
#include <sstream>
using std::ostringstream;
#include <vector>
using std::vector;
#include <fstream>
using std::ofstream;
#include "base/os.h"

////////////////	NAMESPACE

namespace brahms
{
	namespace output
	{



////////////////    OUTPUT DETAIL LEVEL

		//	safely in C++, we change detail level to be a typed value (enum)
		#undef D_NONE
		#undef D_WARN
		#undef D_INFO
		#undef D_VERB

		//	have to specify values of each, so they match those used in the #define's
		enum EnumDetailLevel
		{
			D_NONE =			0x00,
			D_WARN =		    0x20,
			D_INFO =			0x40,
			D_VERB =			0x60,
			D_VERB_HILITE =		0x5F,		//	D_VERB with HILITE
			D_VERB_ERROR =		0x5E,		//	D_VERB with HILITE ERROR
			D_LOQU =			0x80,
			D_FULL =			0xA0,

			D_INFO_UNHIDEABLE = 0xF0 // D_INFO, semantically, but cannot be hidden (e.g. additional information required to debug an E_DEADLOCK)
		};



////////////////	OUTPUT SOURCE

		class Sink;

		class Source
		{

			friend class Sink;

		public:

			//	tors
			Source();
			Source(const Source& rhs);
			Source& operator=(const Source& rhs);
			~Source();

			//	connect/disconnect
			void connect(Sink* sink, const char* threadIdentifier);
			void disconnect();

			//	send buffered output to sink
			Source& operator<<(const EnumDetailLevel& level);

			//	buffer output (template)
			template<class T> Source& operator<<(const T& data)
			{
				buffer << data;
				return *this;
			}

			//	buffer output (template)
			template<class T> Source& operator<<(T& (*data)(T&))
			{
				buffer << data;
				return *this;
			}

			//	retrieve current detail level
			EnumDetailLevel getLevel()
			{
				return level;
			}

		private:

			//	pointer to Sink object
			Sink* sink;

			//	log level control
			EnumDetailLevel level;

			//	log level control
			stack<EnumDetailLevel> levelsStack;

			//	buffer
			ostringstream buffer;

			//	prefix for this sink
			string threadIdentifier; // cached copy of thread's identifier
			string prefix;
		};



////////////////	OUTPUT SINK

		struct LogLevelSwitch
		{
			string threadIdentifier;
			string section;
			EnumDetailLevel level;
			vector<Source*> sources;
		};

		struct DataBurst
		{
			UINT32 voiceCount;
			INT32 voiceIndex;
			UINT32 workerCount;
		};

		class Sink
		{

			friend class Source;

		public:

			//	tors
			Sink();
			~Sink();

			//	init
			void init(const CreateEngine& createEngine);

			//	connect from source
			void connect(Source* source);

			//	run
			void message(Source* source, EnumDetailLevel level);

			//	enter/leave section
			void section(const string& key);

			//	term
			UINT32 term(const DataBurst& dataBurst, vector<brahms::error::Error>& errors);

		private:

			//	log parameters
			TextFormat format;
			bool debugtrace;

			//	log level control
			EnumDetailLevel defaultLevel;
			stack<string> sections;
			vector<LogLevelSwitch> switches;

			//	connected sources
			vector<Source*> sources;

			//	log state
			brahms::os::Mutex mutex;
			brahms::os::Timer sinkTimer;
			ofstream logfile;

			//	audit
			UINT32 messageCount;

			//	final output
			void final(const char* s);

		};



////////////////	NAMESPACE

	}
}



////////////////	INCLUSION GUARD

#endif

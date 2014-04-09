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

	$Id:: core.h 2309 2009-11-06 00:49:32Z benjmitch           $
	$Rev:: 2309                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-06 00:49:32 +0000 (Fri, 06 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_FRAMEWORK_BASE_CORE
#define INCLUDED_BRAHMS_FRAMEWORK_BASE_CORE



////////////////	NAMESPACE

namespace brahms
{
	namespace base
	{



////////////////	CONDITION

		enum EnumCondition
		{
			COND_LOCAL_ERROR = 0,
			COND_PEER_ERROR = 1,
			COND_LOCAL_CANCEL = 2,
			COND_PEER_CANCEL = 3,
			COND_END_RUN_PHASE = 4,			//	run-phase should end immediately, rather than proceeding to end of execution
			COND_MAX
		};

		struct Condition
		{
			Condition()
			{
				for (int i=0; i<((int)COND_MAX); i++)
					cond[i] = false;
			}

			void set(EnumCondition c)
			{
				cond[(int)c] = true;
				cond[(int)COND_END_RUN_PHASE] = true;
			}

			bool get(EnumCondition c)
			{
				return cond[(int)c];
			}

			bool* get_p(EnumCondition c)
			{
				return &cond[(int)c];
			}

			//	conditions
			bool cond[(int)COND_MAX];
		};



////////////////	ERROR STACK

		struct ErrorStack
		{
			void push(brahms::error::Error& error);
			vector<brahms::error::Error>& getErrors();
			void clearSpurious();

		private:

			vector<brahms::error::Error> errors;
			brahms::os::Mutex mutex;
		};



////////////////	EXECUTION PARAMETERS

		struct ExecutionParameter
		{
			string key;
			string val;
		};

		struct ExecutionParameters
		{
			string gets(const char* key)
			{
				for (unsigned int p=0; p<parameters.size(); p++)
					if (parameters[p].key == key) return parameters[p].val;
				ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \"" << key << "\" not found";
				return "";
			}

			UINT32 getu(const char* key)
			{
				stringstream ss;
				ss.str(gets(key));
				UINT32 uval;
				ss >> uval;
				return uval;
			}

			void set(const char* key, const char* val)
			{
				for (unsigned int p=0; p<parameters.size(); p++)
				{
					if (parameters[p].key == key)
					{
						parameters[p].val = val;
						return;
					}
				}
				parameters.resize(parameters.size() + 1);
				parameters.back().key = key;
				parameters.back().val = val;
			}

			vector<ExecutionParameter> parameters;
		};



////////////////	CORE OBJECT

		struct Core
		{
			Core()
				:
				caller(brahms::thread::TC_CALLER, 0, *this)
			{
			}

			VoiceIndex getVoiceIndex()
			{
				return createEngine.voiceIndex;
			}

			void setVoiceIndex(VoiceIndex voiceIndex)
			{
				createEngine.voiceIndex = voiceIndex;
			}

			VoiceCount getVoiceCount()
			{
				return createEngine.voiceCount;
			}

			void setVoiceCount(VoiceCount voiceCount)
			{
				createEngine.voiceCount = voiceCount;
			}

			ErrorStack errorStack;
			Condition condition;
			CreateEngine createEngine;
			brahms::thread::ThreadBase caller;
			ExecutionParameters execPars;
			brahms::output::Sink sink;
		};



	}
}



////////////////	INCLUSION GUARD

#endif



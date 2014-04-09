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

	$Id:: set.h 2317 2009-11-07 22:10:20Z benjmitch            $
	$Rev:: 2317                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-07 22:10:20 +0000 (Sat, 07 Nov 2009)       $
________________________________________________________________

*/



namespace brahms
{
	namespace systemml
	{



		class Set : public RegisteredObject
		{

		public:

			Set(string name, EngineData& engineData, ComponentType objectType, Process* process);
			virtual ~Set();

			vector<Port*> ports;

			//	interface
			UINT32 getPortCount() const;
			Port* getPortByName(const char* name);
			Port* getPortByIndex(UINT32 index);
			void getSetInfo(EventGetSetInfo* info);

			//	reference to engine data
			EngineData& engineData;

			//	reference to parent process
			Process* process;

			//	set flags
			UINT32 flags;
		};

		class OutputSet : public Set
		{

		public:

			OutputSet(string name, EngineData& engineData, Process* process);

			void getAllSampleRates(vector<SampleRate>& sampleRates);
			void finalizeAllComponentTimes(SampleRate baseSampleRate, BaseSamples executionStop);
			void listAvailableOutputs(brahms::output::Source& fout);

			//	interface
			OutputPort* addPort(const char* className, UINT16 release);
			OutputPort* addPortByCopy(Data* rhs);
		};

		class InputSet : public Set
		{

		public:

			InputSet(string name, EngineData& engineData, Process* process);

			//	interface
			InputPort* addPort(Link* link);
		};




	}
}



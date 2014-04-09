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

	$Id:: interface.h 2158 2009-10-23 02:51:13Z benjmitch      $
	$Rev:: 2158                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-10-23 03:51:13 +0100 (Fri, 23 Oct 2009)       $
________________________________________________________________

*/





namespace brahms
{
	namespace systemml
	{



	////////////////	INTERFACE

		class Interface
		{

		public:

			Interface(EngineData& engineData, Process* process);
			virtual ~Interface();

			//	set factory
			virtual Set* create(string name) = 0;

			//	interface
			UINT32 getPortCount() const;
			UINT32 getSetCount();
			Set* getSetByIndex(UINT32 index);
			Set* getSetByName(const char* name);
			Set* addSet(const char* name);

			//	extras
			UINT32 listMissingInputs(const char* processName, brahms::output::Source& fout);

			//	set list
			vector<Set*> sets;

			//	reference to engine data
			EngineData& engineData;

			//	reference to parent process
			Process* process;
		};

		class OutputInterface : public Interface
		{

		public:

			OutputInterface(EngineData& engineData, Process* process);

			Set* create(string name);
			OutputSet* getSetByName(const char* name);

			//	find an output that has the name expected by the passed input port
			OutputPort* findOutput(Identifier& outputName);

			void getAllSampleRates(vector<SampleRate>& sampleRates);

			void finalizeAllComponentTimes(SampleRate baseSampleRate, BaseSamples executionStop);

			void listAvailableOutputs(brahms::output::Source& fout);

			//	acquire write lock (if due)
			Symbol writeLock(BaseSamples now);

			//	release write lock (if due)
			BaseSamples writeRelease(BaseSamples now, BaseSamples stop, brahms::output::Source* tout);

			//	list of ports on all sets
			vector<OutputPort*> ports;

		};

		class InputInterface : public Interface
		{

		public:

			InputInterface(EngineData& engineData, Process* process);

			Set* create(string name);
			InputSet* getSetByName(const char* name);


			//	acquire read lock (if due)
			Symbol readLock(BaseSamples now, brahms::output::Source* tout);

			//	relase read lock (if due)
			BaseSamples readRelease(BaseSamples now, BaseSamples stop);

			//	list of ports on all sets
			vector<InputPortLocal*> ports;

		};





	}
}

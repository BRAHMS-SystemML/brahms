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

	$Id:: port.h 2317 2009-11-07 22:10:20Z benjmitch           $
	$Rev:: 2317                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-07 22:10:20 +0000 (Sat, 07 Nov 2009)       $
________________________________________________________________

*/





#include "alternator.h"



/*


		NOTES IMPORTED FROM manifold.h IN BRAHMS 0.7.1 (BEFORE COMPONENT INTERFACE REFACTORING)



		Distribution Manifolds:
		-----------------------

		A distribution manifold is the conceptual object constructed from one
		Inlet, and a number of Outlets. Each Inlet-Outlet pair is a Pipe, so
		all pipes in a manifold share the same inlet.

		An Inlet is written by the writer, as follows:

			* Symbol hData = Inlet->lock(); (for writing)
			* write(hData)
			* Inlet->release(); (for writing)

		An Outlet is read by the reader, as follows:

			* Symbol hData = Outlet->lock(); (for reading)
			* read(hData)
			* Outlet->release(); (for reading)

		For local inlets and outlets, the reader and writer are the service()
		routines of the worker threads that are running the processes that
		write the Inlet and read the Outlet, respectively.

		For remote inlets and outlets, the reader and writer are the comms
		channels that are connected to the remote voices that hold the processes
		that write the Inlet and read the Outlet, respectively.

		There is one nuance, which is that the reader of a remote Outlet, the
		comms channel, does not know about the timing logic between the processes
		that are being computed locally, so it doesn't know in which order to read
		from the Outlets. Rather than have it maintain its own copy of the timing
		logic or, alternatively, to have the worker thread (that maintains the
		timing logic) nudge the channel only for remote outlets, we have the remote
		outlet itself nudge the associated channel, acting as the reader for the
		lock() part of the read operation.

		For more information on pipes that cross between processes, see the
		multi-processing extensions in Charlie.





		The type of pipe we originally used in solo is the double-buffer pipe, where a separate
		object exists at each end, and their identities are flipped after each write to the
		inlet. This type of pipe does implement mutually exclusive locking, in that a flip of
		the buffers will occur whilst the inlet is locked and has just been written, but while
		the outlet is released. The inlet and outlet will typically be in different threads for
		this type of pipe. Their procedures are as follows:

		INLET:

			* DO
				* output = inlet.Lock()		: returns a copy to the pipe's inlet buffer
				* (run process, inc. write to output)
				* inlet.Release()			: inlet buffer is now written
					- wait for READ_COMPLETE
					- flip inlet and outlet buffers
					- signal READ_READY
			* LOOP

		OUTLET:

			* DO
				* input = outlet.Lock()		: returns a copy to the pipe's outlet buffer
					- wait for READ_READY
				* (run process, inc. read from input)
				* outlet.Release()			: outlet buffer is now read
					- signal READ_COMPLETE
			* LOOP




*/




namespace brahms
{
	namespace systemml
	{



	////////////////	RING BUFFER
		
		class InputPort;

		struct RingBufferItem
		{
			RingBufferItem(brahms::systemml::Data* p_data);
			~RingBufferItem();

			//	single data object
			brahms::systemml::Data* data;

			//	one alternator object for each reader (writer addresses them all)
			vector<Alternator*> alternators;
		};

		struct RingBuffer : public vector<RingBufferItem*>
		{
			RingBuffer(brahms::systemml::Data* frontBufferObject);
			~RingBuffer();

			UINT32 getNumberOfReaders();
			Data* getWriteBuffer(UINT32 offset = 0);
			Data* expand(InputPort* port, UINT32 lag, bool redundant, brahms::output::Source& source, const bool* cancel);
			void dump();
			void destroy(brahms::output::Source* source);
			void initLocks(UINT32 reader, UINT32 buffer, bool writeReady);

			//	list of attached readers
			vector<InputPort*> attachedReaders;

			//	index into ring of current write buffer
			INT32 writeBuffer;
		};




	////////////////	PORT

		class Port : public RegisteredObject
		{

		public:

			Port(string name, EngineData& engineData, ComponentType objectType, Set* parentSet);
			virtual ~Port();

			//	port flags
			UINT32 flags;

			//	only used to get structural info on the data object (e.g. sample period)
			virtual Data* getZerothData() = 0;
			virtual Data* getZerothDataOrNull() = 0;

			//	port info
			void getPortInfo(EventGetPortInfo* info);

			//	parent
			Set* parentSet;

			//	engine data
			EngineData& engineData;

			//	attach port
			bool attach(HandledEvent* event, UINT32 flags, const ComponentSpec* spec = NULL);

			//	get/set due data
			Data* getDueData();
			void setDueData(Data* dueData);

			//	validate spec
			void validateSpec();

		private:

			//	port event
			HandledEvent* portEvent;

			//	stored for validatiion when an input is attached
			ComponentSpec validationSpec;

			//	placeholder for "due" data - this is written/erased
			//	continually by the worker thread, and is where sml_getPortData
			//	fetches the data from (which could, therefore, be absent)
			Data* dueData;
		};



	////////////////	INPUT PORT

		class InputPort : public Port
		{

		public:

			InputPort(string name, EngineData& engineData, Set* parentSet);

			//	this is NULL until the output port is connected
			OutputPort* connectedOutputPort;

			//	we have the lag stored here
			UINT32 lag;

			//	pointer to next entry to lock in ring buffer (or current locked entry)
			UINT32 readBuffer;

			//	pointer to read-ring in ring buffer we're connected to
			UINT32 readerIndex;

			//	we fetch this from the connected output port
			Data* getZerothData();
			Data* getZerothDataOrNull();

		};

		class InputPortLocal : public InputPort
		{

		public:

			InputPortLocal(string name, EngineData& engineData, Link* link, Set* parentSet);

			//	pointer to formative link, through which to obtain reference data
			Link* link;

			//	acquire/release read lock (if due)
			Symbol readLock(BaseSamples now);
			BaseSamples readRelease(BaseSamples now, BaseSamples nextService);
		};

		class InputPortRemote : public InputPort
		{

		public:

			//	ctor
			InputPortRemote(string name, EngineData& engineData, brahms::channel::Channel* channel, UINT32 msgStreamID, UINT32 remoteVoiceIndex);
			~InputPortRemote();

			//	inlet interface, only need be overridden by outlets that use these callbacks
			void additionalInputAttached(UINT32 numberOfBuffers);
			void outputWriteReleased(brahms::output::Source* tout);
			void readRelease(UINT32 index);

			//	comms channel that will carry this stream
			brahms::channel::Channel* channel;

			//	state
			UINT32 remoteVoiceIndex;
			UINT32 nextBufferToRead;
			UINT32 msgStreamID;
			UINT32 messagesSent;

			//	output messages (one for each buffer)
			vector<brahms::base::IPM*> msgs;
		};

		void messageAway(brahms::base::IPM* ipm);



	////////////////	OUTPUT PORT

		/*

			NOTE that "OutputPort" is in fact the same object used for local
			and remote output ports. This is in contrast to "InputPort", which
			is instantiated as "InputPortLocal" or "InputPortRemote" depending
			on the role it plays.

		*/

		class OutputPort : public Port, public Loggable
		{

		public:

			OutputPort(string name, EngineData& engineData, Data* data, Set* parentSet);
			~OutputPort();

			//	output buffer
			RingBuffer ring;

			//	interface
			void connectInput(InputPort* input, UINT32 lag, brahms::output::Source& source);
			void startLog(bool encap, string filename, brahms::output::Source* tout);

			//	only used to get structural info on the data object (e.g. sample period)
			Data* getZerothData();
			Data* getZerothDataOrNull();

			//	true initially, until marshalled by framework
			bool newlyCreated;

			//	cached sample period (set in finalizeComponentTime())
			BaseSamples samplePeriod;

			//	acquire/release read lock (if due)
			Symbol readLock(InputPort* port, BaseSamples now, UINT32 bufferIndex, Data** data);
			BaseSamples readRelease(InputPort* port, BaseSamples now, BaseSamples nextService);

			//	client interface
			void setName(const char* name);
			void setSampleRate(SampleRate rate);

			//	framework interface
			void finalizeAllComponentTimes(SampleRate baseSampleRate, BaseSamples executionStop);

			//	acquire/release write lock (if due)
			Symbol writeLock(BaseSamples now);
			BaseSamples writeRelease(BaseSamples now, brahms::output::Source* tout, BaseSamples nextService);

			//	attached remotes
			void connectRemoteInput(InputPortRemote* port);
			vector<InputPortRemote*> remoteInputs;

			//	init locks
			void initInterThreadLocks(brahms::output::Source& fout);

			//	init data
			void endInitPhase(brahms::output::Source& fout);

		};





	}
}


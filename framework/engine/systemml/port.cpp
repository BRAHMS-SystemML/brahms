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

	$Id:: port.cpp 2451 2010-01-25 16:48:58Z benjmitch         $
	$Rev:: 2451                                                $
	$Author:: benjmitch                                        $
	$Date:: 2010-01-25 16:48:58 +0000 (Mon, 25 Jan 2010)       $
________________________________________________________________

*/




#include "systemml.h"


using namespace brahms::math;


namespace brahms
{
	namespace systemml
	{




	////////////////	PORT

		Port::Port(string p_name, EngineData& p_engineData, ComponentType objectType, Set* p_parentSet)
			: RegisteredObject(objectType, p_name), parentSet(p_parentSet), engineData(p_engineData)
		{
			//	due data begins NULL, and is only written (temporarily) during the service section in the
			//	main loop of the worker thread - see WorkerThread::Service()
			dueData = NULL;

			//	initially, no flags set
			flags = 0;

			//	no port event
			portEvent = NULL;

			//	no validation spec
			validationSpec.cls = NULL;
			validationSpec.release = 0;
		}

		Port::~Port()
		{
		}

		void Port::getPortInfo(EventGetPortInfo* info)
		{
			info->name = getObjectName().c_str();
			info->flags = flags;
			Data* data = getZerothDataOrNull();
			if (data) info->componentInfo = data->getComponentInfo();
			else info->componentInfo = NULL;
		}

		void Port::validateSpec()
		{
			if (validationSpec.cls)
			{
				//	validate
				const ComponentInfo* cinfo = dueData->getComponentInfo();
				string name = dueData->getObjectName();
				string dcls = cinfo->cls;
				UINT32 drel = cinfo->componentVersion->release;
				if (dcls != validationSpec.cls)
					ferr << E_INVALID_INPUT << "expected \"" << name << "\" to be of class \"" << validationSpec.cls << "\"";
				if (drel < validationSpec.release)
					ferr << E_INVALID_INPUT << "expected \"" << name << "\" to be release " << validationSpec.release << " or greater";
			}
		}

		//	attach port
		bool Port::attach(HandledEvent* event, UINT32 flags, const ComponentSpec* spec)
		{
			//	validation spec
			if (spec)
				validationSpec = *spec;

			//	on attachment, set handler and current object
			event->handler = NULL;
			event->event.object = NULL;
			
			//	if due at time of attachment, lay that in
			if (dueData)
			{
				event->handler = dueData->module->getHandler();
				event->event.object = dueData->object;
				validateSpec();
			}

			//	store unless explicitly asked not to
			if (!(flags & F_NO_STORE))
				portEvent = event;

			//	ok
			return dueData != NULL;
		}

		//	get due data
		Data* Port::getDueData()
		{
			return dueData;
		}

		//	set due data
		void Port::setDueData(Data* p_dueData)
		{
			//	set locally
			dueData = p_dueData;

			//	set in attached port event
			if (portEvent)
			{
				if (dueData)
				{
					//	TODO: small, but an optimisation - do this at init, rather than having to check it at run-time
					if (!portEvent->handler)
					{
						//	first time data has been due since attach
						portEvent->handler = dueData->module->getHandler();
						validateSpec();
					}

					portEvent->event.object = dueData->object;
				}
				else
				{
					portEvent->event.object = NULL;
				}
			}
		}



	////////////////	INPUT PORT

		InputPort::InputPort(string name, EngineData& engineData, Set* parentSet)
			:
			Port(name, engineData, CT_INPUT_PORT, parentSet)
		{
			//	this is NULL until the output port is connected
			connectedOutputPort = NULL;

			//	irrelevant, really
			readBuffer = 0;
			readerIndex = 0;
			lag = 0;

		}

		InputPortLocal::InputPortLocal(string name, EngineData& engineData, Link* link, Set* parentSet)
			: InputPort(name, engineData, parentSet)
		{
			//	store link for connecting this port later
			this->link = link;

			//	set F_IMPLICIT_NAME if appropriate
			if (link->dstPortNameInferred)
				flags |= F_IMPLICIT_NAME;
		}

		Symbol InputPortLocal::readLock(BaseSamples now)
		{
			return connectedOutputPort->readLock(this, now, readBuffer, NULL);
		}

		BaseSamples InputPortLocal::readRelease(BaseSamples now, BaseSamples nextService)
		{
			return connectedOutputPort->readRelease(this, now, nextService);
		}

		Data* InputPort::getZerothData()
		{
			if (!connectedOutputPort) ferr << E_INTERNAL;
			return connectedOutputPort->getZerothData();
		}

		Data* InputPort::getZerothDataOrNull()
		{
			if (!connectedOutputPort) return NULL;
			return connectedOutputPort->getZerothData();
		}



	////////////////	REMOTE

		InputPortRemote::InputPortRemote(string name, EngineData& engineData, brahms::channel::Channel* p_channel, UINT32 p_msgStreamID, UINT32 p_remoteVoiceIndex)
			:
			InputPort(name, engineData, NULL)
		{
			channel = p_channel;
			msgStreamID = p_msgStreamID;
			messagesSent = 0;
			remoteVoiceIndex = p_remoteVoiceIndex;
			nextBufferToRead = 0;
		}

		InputPortRemote::~InputPortRemote()
		{
			for (unsigned int m=0; m<msgs.size(); m++)
				delete msgs[m];
		}

		void InputPortRemote::additionalInputAttached(UINT32 numberOfBuffers)
		{
			//	this is called every time a new input is attached to the
			//	output to which we are connected, so that we can expand
			//	our number of messages to match the new number of buffers
			//	(which may not have changed on consecutive calls!)
			while(msgs.size() < numberOfBuffers)
				msgs.push_back(new brahms::base::IPM(messageAway, this, msgs.size(), remoteVoiceIndex));
		}

		void InputPortRemote::outputWriteReleased(brahms::output::Source* tout)
		{
			/*

				For a local outlet, the outlet is lock()ed when the reader
				in the worker thread wants the data from the outlet. For
				a remote outlet, there is no local reader, so we have to
				emulate that behaviour somewhere else. Since we have just
				been released by the writer in the worker thread, we have
				just been written, so this is the earliest point at which
				we can initiate the push behaviour. So we do it now.

				We are going to behave exactly as the worker thread would
				behave - that is, we'll lock the outlet for reading, then
				push the message, passing the comms layer a handle to this
				outlet, so that when the push is complete, it can act as
				the worker thread, release()ing this outlet for reading,
				allowing the inlet to be written again on the next time
				round the ring buffer.

				So, to summarize: we (this outlet, running in the worker
				thread, as it releases the inlet for writing) will act as
				the first half of the reader, locking() the outlet. The
				comms layer will act as the second half of the reader,
				releasing the outlet again after it's pushed the message.

				NOTE that "readBuffer", as for local input ports, holds the
				offset to the next buffer *to be released*, whereas
				"nextBufferToRead" holds the offset to the next buffer *to be
				locked*. These can get out of sync with a remote input port,
				because sending can get held up whilst queueing-for-send
				(this function) continues. Callbacks to messageAway() catch
				up readBuffer towards nextBufferToRead.

			*/



			/*	DOCUMENTATION: LOCK_RELEASE_INTERLEAVING

				InputPortLocal receives calls to readLock() and readRelease() from the thread
				that is processing its output. As a result, the calls are strictly interleaved,
				thus:

					readLock()
					readRelease()
					readLock()
					readRelease()
					...

				When InputPortLocal::readLock() is called, it just passes through to call
				OutputPort::readLock(InputPort* this) on the connected OutputPort. That
				function sets the dueData member of the InputPort. Therefore, when readRelease()
				is called on the InputPort, it knows that its dueData member is correctly
				set, because of the strict interleaving of lock/release. OutputPort::readRelease()
				sets the InputPort dueData member back to NULL, for safety.

				InputPortRemote generates its own readLock() calls (hence it has no readLock()
				member function), but its readRelease() calls come from the comms layer, as and
				when the comms layer has sent the message. Therefore, they are not guaranteed
				to be interleaved (in fact, often won't be), thus:

					readLock()
					readRelease()
					readLock()
					readLock()
					readRelease()
					readRelease()
					readLock()
					readRelease()
					...

				Note that this is not a problem for knowing "what" is being released at a call
				to readRelease(), since InputPortRemote maintains separately the index of which
				buffer needs to next be released (readBuffer) and which buffer needs to next be
				read (nextBufferToRead). However, the dueData member can be changed at any time,
				since readRelease() calls can come at any time and set it back to NULL, or
				readLock() calls can come and set it to something spurious. When we make the call
				to OutputPort::readLock(), below, our dueData member is set correctly (after [[A]]).
				However, it may be NULL'd by a readRelease() call before we have finished using
				it (at [[B]]). Therefore, we use a local value of that variable, which is returned
				to us by the readLock() call.

				An alternative strategy is to protect the [[A]] => [[B]] section with a mutex, and
				protect the readRelease() call with the same mutex. This works, but is presumably
				slower.

			*/

			/* [[A]] */

			//	lock inlet for reading
			Data* data = NULL;
			Symbol result = connectedOutputPort->readLock(this, 0, nextBufferToRead, &data); // we *know* it's due, so we can pass 0 as the time
			if (result == C_CANCEL)
			{
				//	lock did not complete, so we don't need to release it
				return;
			}

			//	send the current write buffer, which has just been written, to our destination
			//	ask write buffer object to serialize()
			EventContent ec;
			brahms::EventEx event(
				EVENT_CONTENT_GET,
				0,
				data,
				&ec,
				true,
				NULL
			);
			event.fire();

			/* [[B]] */



			/* DOCUMENTATION: PUSHDATA_IDENTICAL_TO_ALL_SENDERS

				We construct the [header|msg] in the buffer provided to us by the data object,
				which avoids having to make an unnecessary copy to do the message construction.
				This has a side effect - if we are sending the value of this data object to
				multiple target voices, we will construct this header multiple times (in principle,
				and currently - in practice, see below...). Therefore, the header must be
				identical as it is sent to all target voices, or else we might overwrite it with
				the data for B before it has been sent to A! The key, here, is that the msgStreamID
				must be the same for all target peers. This leads us to how we choose the
				msgStreamID. We were, previously, choosing it as the index of the InputPortRemote
				in System::inputPortRemotes. But since the two InputPortRemote's are different,
				that would give them different msgStreamID's. We fix this by setting msgStreamID
				for both to the index of the first in System::inputPortRemotes (search for the
				tag PUSHDATA_IDENTICAL_TO_ALL_SENDERS in system.cpp).

				TODO: both of the TODO's below should be done together, and against a test system
				that has the right properties (multiple receivers of the same output, all on different
				voices, probably should have one or two receivers on each voice, and some on this one
				as well, just to really exercise the problem).

				TODO: We currently call EVENT_CONTENT_GET for every peer we're 
				sending to, and then construct the header, beacuse each peer we're sending to
				has an associated InputPortRemote. it would be better to call EVENT and construct
				the header just once, then send the same total data block down multiple channels.
				this is easy enough - when some peer sends IPMTAG_FINDINLET, if we've already
				an InputPortRemote corresponding, just link another target peer to that, instead
				of adding a new InputPortRemote. thus, one EVENT, one header construction, and
				no ambiguity about the value of msgStreamID either - we can just use the index
				of the InputPortRemote in the array of those held by the system (called inputPorts).
				Thus, this would be an optimisation, but also reduce the complexity, well worth it.

				TODO: Currently, the comms layer, if using compression, will compress this message
				once for every destination it's sent to. the compression will be identical each time,
				and so will bytesAfterHeaderCompressed, so there won't be any corruption, but it will
				take unnecessary CPU. possible fix is to only compress in the comms layer if
				bytesAfterHeaderCompressed is zero. But this won't work, because the different
				sends/compresses are made in multiple threads, so they might do it simultaneously.
				Also, when compress is done, it's done to a separate buffer, so that won't propagate
				back. Basically, that doesn't work. If we've already done the above TODO (one
				InputPortRemote for multiple receivers), then we can do the compression _before_ we
				even send the message to the comms layer. It has, historically, been the responsibility
				of the comms channel to do this, but we could move it up into the engine (or rather
				into a separate "compression" library, since we'd like to avoid dependence on it).
				Then, we would get the message ready for sending, then if necessary compress it into
				another buffer, then send that message to all receiving channels. When they've all
				called messageAway(), the buffer could be discarded.
				
			*/



			//	form an IPM around the serialized data by adding a header
			//	in the space that the data object has reserved for it; we
			//	also make sure the header is zeroed in other places.
			brahms::base::IPM_HEADER* header = (brahms::base::IPM_HEADER*)ec.stream;

			//	A
			header->sig = brahms::base::IPM_SIGNATURE;
			//header->from = will be filled in by comms layer
			header->tag = brahms::base::IPMTAG_PUSHDATA;
			//header->fmt = will be filled in by comms layer

			//	B
			header->order = messagesSent;
			header->msgStreamID = msgStreamID;

			//	C
			header->bytesAfterHeaderUncompressed = ec.bytes;
			//header->bytesAfterHeaderCompressed = will be filled in by comms layer

			//	D
			header->D = 0;



			//	set the stream of one of our static messages to the buffer we've just constructed
			msgs[nextBufferToRead]->setExternalStream(ec.stream);

			//	if channel returns non-zero pause length (indicating congestion at the other
			//	end), we delay that long and try again
			while(true)
			{
				//	send message over channel
				UINT32 msec = channel->push(msgs[nextBufferToRead], tout);

				//	if sent, continue
				if (!msec) break;

				//	otherwise, we've been asked to hold our horses (congestion protection)
				//	we better check in case cancel is set, because otherwise we'll get
				//	stuck here indefinitely
					

				//	ok, sleep a bit, before trying to send again
				brahms::os::msleep(msec);
			}

			//	advance to next read buffer
			nextBufferToRead = nextBufferToRead ? nextBufferToRead - 1 : msgs.size() - 1;

			//	audit number of messages sent
			messagesSent++;
		}

		void InputPortRemote::readRelease(UINT32 index)
		{
			//	check
			if (index != readBuffer)
				ferr << E_INTERNAL << "disagreement whilst doing comms";

			//	call readRelease() on the output port we're connected to
			connectedOutputPort->readRelease(this, 0, 1);
		}

		void messageAway(brahms::base::IPM* ipm)
		{
			//	get parent and index
			InputPortRemote* port = (InputPortRemote*)ipm->getParentPort();
			port->readRelease(ipm->getIndex());
		}



	////////////////	RING BUFFER

		RingBufferItem::RingBufferItem(brahms::systemml::Data* p_data)
		{
			data = p_data;
		}

		RingBufferItem::~RingBufferItem()
		{
			delete data;
			for (UINT32 a=0; a<alternators.size(); a++)
				delete alternators[a];
		}

		RingBuffer::RingBuffer(brahms::systemml::Data* frontBufferObject)
		{
			//	create front buffer - other buffers will be created between
			//	CONNECT and POSTCONNECT, all at once for ease of algorithm
			//	in the meantime, we've got our example of the data on this
			//	port in the front buffer, which is also...
			push_back(new RingBufferItem(frontBufferObject));

			//	...initially the write buffer.
			writeBuffer = 0;
		}

		RingBuffer::~RingBuffer()
		{
			//	clear ring buffer
			for (UINT32 i=0; i<size(); i++)
				delete at(i);
		}

		void RingBuffer::initLocks(UINT32 reader, UINT32 buffer, bool writeReady)
		{
			at(buffer)->alternators[reader]->init(writeReady);
		}

		void RingBuffer::dump()
		{
			//	show nothing if no readers attached
			int nReaders = getNumberOfReaders();
			if (!nReaders) return;

			//	output debug information
			cerr << "\nRingBuffer::dump(" << at(0)->data->getObjectName() << ")" << endl;

			for (int r=-1; r<nReaders; r++)
			{
				if (r == -1)
				{
					cerr << "lag:         ";
					for (UINT32 l=0; l<size(); l++)
						cerr << " " << l;
				}
				else
				{
					cerr << "reader[" << r << "]:   ";
					for (UINT32 l=0; l<size(); l++)
						cerr << " " << at(l)->alternators[r]->debugState;
				}
				cerr << endl;
			}

			cerr << endl;
		}

		void RingBuffer::destroy(brahms::output::Source* tout)
		{
			//	clear ring buffer
			for (UINT32 i=0; i<size(); i++)
				at(i)->data->destroy(tout);
		}

		UINT32 RingBuffer::getNumberOfReaders()
		{
			//	the ring buffer has L entries (L is maximum lag of all currently connected inputs
			//	and each one has N alternators (N is the number of currently connected inputs).
			//	we can obtain N, thus, from any one of the entries.
			return at(0)->alternators.size();
		}

		Data* RingBuffer::getWriteBuffer(UINT32 offset)
		{
			//	validate - note that offset == size() is allowed (this is the
			//	same as zero, but has the semantic that it's the *previous*
			//	value we're pulling out)
			if (offset > size()) ferr << E_INTERNAL << "offset is " << offset << ", size is " << size();

			//	usually, offset is zero, and we return the write buffer
			//	but we can also use this to return the buffer that was
			//	written "offset" samples before the current write buffer
			//	gets written
			INT32 index = (writeBuffer - offset) % size();
			return at(index)->data;
		}

		Data* RingBuffer::expand(InputPort* port, UINT32 lag, bool redundant, brahms::output::Source& fout, const bool* cancel)
		{
			//	store reader
			attachedReaders.push_back(port);

			//	current number of "readers" for this port (*before* this call!)
			UINT32 N = at(0)->alternators.size();

			//	add any new buffers that are required - alternators for existing buffers
			//	will be set to WRITE_READY since they have "already" been read by the
			//	attached readers (where "already" means "at t = 0 - readerLag"), i.e. it
			//	never physically happened
			//
			//	also - make sure that we always have at least two buffers (front and back)
			//	so that we can have the writer and reader working in parallel even if they
			//	are linked with a link of lag zero
			UINT32 requiredBuffers = lag + 1;
			if (requiredBuffers < 2) requiredBuffers = 2;
			while (size() < requiredBuffers)
			{
				//	add new buffer with duplicate of front buffer data object
				push_back(new RingBufferItem(at(0)->data->duplicate(&fout)));

				//	initialise all these alternators to WRITE_READY
				for (UINT32 n=0; n<N; n++)
				{
					/*	DOCUMENTATION: INTERTHREAD_SIGNALLING
					
						alternator redundancy is equal around the ring (time), but not across the ring (readers)
						so we can propagate it when we add a new ring buffer entry. all new alternators around this
						reader-ring will thus share the "redundant" setting of the front buffer alternator on this reader-ring
					*/

					bool redundant_n = at(0)->alternators[n]->getRedundant();

					//	create enough alternators in new ring item for existing readers (not for new reader yet, that's below!)
					//	init as WRITE_READY because these "used" buffers for existing readers will be written first
					at(size() - 1)->alternators.push_back(
						new Alternator(/*true,*/ brahms::os::SIGNAL_INFINITE_WAIT, cancel, redundant_n)
					);
				}
			}

			//	now that the existing readers are all set (and the number of buffers is
			//	correct given the new max lag on this port) we conclude by adding the new reader

			//	add alternator to each buffer
			for (UINT32 i=0; i<size(); i++)
			{
				//	create [1, 2, ..., lag-1, lag] as READ_READY
				if (i && i<=lag)
				{
					at(i)->alternators.push_back(
						new Alternator(/*false,*/ brahms::os::SIGNAL_INFINITE_WAIT, cancel, redundant)
					);
				}

				//	create [0] and [lag+1, ...] as WRITE_READY
				else
				{
					at(i)->alternators.push_back(
						new Alternator(/*true,*/ brahms::os::SIGNAL_INFINITE_WAIT, cancel, redundant)
					);
				}
			}

			//	return lagged data, whether new or not
			return at(lag)->data;
		}



	////////////////	OUTPUT PORT

		OutputPort::OutputPort(string name, EngineData& engineData, Data* data, Set* parentSet)
			:
			Port(name, engineData, CT_OUTPUT_PORT, parentSet),
			Loggable(data),
			ring(data)
			/* logEvent *always* fired on front buffer object, so this initialisation is ok for ever */
		{
			//	set data's parent port
			data->parentPort = this;

			//	initially new
			newlyCreated = true;
		}

		OutputPort::~OutputPort()
		{
			//	just need to delete data in the ring buffer whilst we've still got an output source
			ring.destroy(&engineData.core.caller.tout);
		}

		void OutputPort::setName(const char* p_name)
		{
			//	validate
			if (!engineData.flag_allowCreatePort)
				ferr << E_SYSTEMML << "cannot modify ports on this interface during this event";

			/*

				We can only set the name of an output port from the process - input
				ports are named, implicitly, in the System File, by the specification
				of Links (input ports are named for the Link destination port name, if
				present, or copied from the Link source port name, if not).

				When we name an output port (here) the name has to be a valid name, which
				has the usual form.

NO WE DON'T!!!!

				As a shortcut, we allow the name to be specified as the name of the data
				object on an input port. For instance, a data may be called:

					system/process>out

				We allow this to be sent directly, so we trim off, here, anything up to
				the last > chevron. Anything else must be compliant with the usual naming
				convention (alpha, then alpha/digit/underscore).

			*/

			//	non-NULL
			if (!p_name)
				ferr << E_INVALID_ARG << "must specify a port name";



			string s_name = p_name;

/*
			//	we allow the idiom of setPortName(data->name), in which case we have
			//	to extract the port name from the data object's SystemML identifier
			size_t pos = s_name.rfind(">");
			if (pos != string::npos) s_name = s_name.substr(pos + 1);
			*/

			//	validate name
			if (!validPortName(s_name.c_str()))
				ferr << E_NOT_COMPLIANT << "illegal port name \"" << s_name << "\"";

			//	parentSet should not be NULL
			if (!parentSet) ferr << E_INTERNAL;

			//	check not in use
			if (parentSet->getPortByName(s_name.c_str()))
				ferr << E_NOT_COMPLIANT << "port name \"" << s_name << "\" in use on this set";

			//	change the name of the port
			setObjectName(s_name);
		}

		void OutputPort::setSampleRate(SampleRate sampleRate)
		{
			//	validate
			if (!engineData.flag_allowCreatePort)
				ferr << E_SYSTEMML << "cannot modify ports on this interface during this event";

			//	validate
			if (!validSampleRate(sampleRate))
				ferr << E_SYSTEMML << "invalid sample rate";

			//	change the sample rate of the first data object
			getZerothData()->componentTime.sampleRate = sampleRate;
		}

		void OutputPort::connectInput(InputPort* input, UINT32 lag, brahms::output::Source& fout)
		{
			//	listened
			flags |= F_LISTENED;

			//	link ports
			input->connectedOutputPort = this;
			input->lag = lag;
			input->readBuffer = lag;
			input->readerIndex = ring.getNumberOfReaders();

			/*	DOCUMENTATION: INTERTHREAD_SIGNALLING

				no interthread signalling is needed if the outlet and the
				inlet are in the same thread, because their temporal relationship is
				guaranteed by the order in which they are computed, without a need
				for additional sync.
			*/

			/*

				*REMOTE* OutputPorts are created *without* a parentSet (i.e. it is
				set to NULL). Since these are serviced by the comms thread, and any
				connected input is in a worker thread, these are necessarily not
				redundant...

			*/

			bool redundant = false;
			if (parentSet && input->parentSet)
				redundant = parentSet->process->thread->getThreadIndex() == input->parentSet->process->thread->getThreadIndex();

			//	expand ring buffer and set reader index of attached input port
			//	the alternators should all stop waiting if COND_END_RUN_PHASE is met
			ring.expand(input, lag, redundant, fout, engineData.core.condition.get_p(brahms::base::COND_END_RUN_PHASE));

			//	place into due data so that calls to sml_getPortData() before run-phase
			//	gets a valid data object
			input->setDueData(ring.at(lag)->data);

			//	check for internal error
			if (!input->getDueData()->getObjectName().length())
				ferr << E_INTERNAL << "whilst connecting new input of process, data object is unnamed";

			//	announce attachment to any attached remote inputs
			for (UINT32 p=0; p<remoteInputs.size(); p++)
				remoteInputs[p]->additionalInputAttached(ring.size());
		}

		void OutputPort::startLog(bool encap, string filename, brahms::output::Source* tout)
		{
			//	listened
			flags |= F_LISTENED;

			//	the very fact that we fire EVENT_LOG_INIT on the data object is what
			//	tells the data object that it is storing - all it needs to know
			//	here is what format it's going to be asked to output its results in
			//	so all we have to do is set the flags. however, we have to know
			//	further down that this data object is storing, so we create
			//	the output node in the output XML object now as a flag of this.
			logEventData.flags = encap ? F_ENCAPSULATED : 0;
			setSuggestedOutputFilename(filename);
			logEvent.type = EVENT_LOG_INIT;
			logEvent.tout = tout;
			logEvent.fire();

			//	set log event type to EVENT_LOG_SERVICE, ready for run phase
			logEvent.type = EVENT_LOG_SERVICE;
		}

		void OutputPort::finalizeAllComponentTimes(SampleRate baseSampleRate, BaseSamples executionStop)
		{
			for (UINT32 i=0; i<ring.size(); i++)
				ring[i]->data->finalizeComponentTime(baseSampleRate, executionStop);

			if (ring.size())
				samplePeriod = ring[0]->data->componentTime.samplePeriod;
		}



		Data* OutputPort::getZerothData()
		{
			if (!ring.size()) ferr << E_INTERNAL;
			return ring.at(0)->data;
		}

		Data* OutputPort::getZerothDataOrNull()
		{
			if (!ring.size()) return NULL;
			return ring.at(0)->data;
		}


		//	acquire write lock (if due)
		Symbol OutputPort::writeLock(BaseSamples now)
		{
			RingBufferItem* buffer ;

			//	if not due, don't go any further
			if (now%samplePeriod) return S_NULL;

			//	ASSERT
			if (ring.writeBuffer >= ((INT32)ring.size()))
				ferr << E_INTERNAL << "ring.writeBuffer >= ring.size() (" << ring.writeBuffer << " >= " << ring.size() << ")";

			//	get buffer
			buffer = ring.at(ring.writeBuffer);

			//	get number of pipes in this manifold
			//	TODO: THIS COULD BE CACHED
			UINT32 pipeCount = buffer->alternators.size();

#ifdef DEBUG_ALTERNATORS
			cerr << "(before write lock of buffer " << ring.writeBuffer << ")" << endl;
			ring.dump();
#endif

			//	lock all streams for write
			for (UINT32 o=0; o<pipeCount; o++)
			{

#ifdef DEBUG_ALTERNATORS
			cerr << "(before " << o << "th write lock of buffer " << ring.writeBuffer << ")" << endl;
			ring.dump();
#endif

				Symbol result = buffer->alternators[o]->writeLock();
				if (result == C_CANCEL) return C_CANCEL;

#ifdef DEBUG_ALTERNATORS
			cerr << "(after " << o << "th write lock of buffer " << ring.writeBuffer << ")" << endl;
			ring.dump();
#endif

			}

#ifdef DEBUG_ALTERNATORS
			cerr << "(after write lock)" << endl;
			ring.dump();
#endif

/*
			//	call back to each outlet
			for(INT32 c=0; c<outlets.size(); c++)
			{
				outlets[c]->inletLock();
			}
			*/

/*				//	return new write buffer data object
			return ring[writeBuffer].data;
			*/

			//	store due data object into due slot
			setDueData(buffer->data);

			//	ok
			return C_OK;
		}

		//	release write lock (if due)
		BaseSamples OutputPort::writeRelease(BaseSamples now, brahms::output::Source* tout, BaseSamples nextService)
		{
			//	if not due, don't go any further
			if (now%samplePeriod) return min(nextService, now - (now%samplePeriod) + samplePeriod);

			//	get period
			Data* dataW = ring.getWriteBuffer();

			//	handle windowing
			bool recordingState = true; // if no origins, recording state is always true
			if (origins.size())
			{
				//	some origins, assume record is off until proven otherwise
				recordingState = false;

				//	for each origin
				for (UINT32 o=0; o<origins.size(); o++)
				{
					LogOriginBaseSamples& origin(origins[o]);

					//	ignore (leave recordingState as false) unless in range
					//	t1 == -1 means window has no end
					if (now >= origin.t0 && (origin.t1 == BASE_SAMPLES_INF || now < origin.t1))
					{
						//	get distance into repeat window
						//	T will be zero for global window
						BaseSamples dt = now - origin.t0;
						if (origin.T) dt %= origin.T; 

						//	check windows
						for (UINT32 w=0; w<origin.windows.size(); w++)
						{
							LogWindowBaseSamples& window(origin.windows[w]);

							//	handle instants
							if (window.t1 == BASE_SAMPLES_INF)
							{
								if (window.t0 == dt)
									recordingState = true;
							}

							//	handle ranges
							else
							{
								if (window.t0 <= dt && window.t1 > dt)
									recordingState = true;
							}
						}
					}
				}
			}

			//	if storing
			if ((logEventData.precision != PRECISION_DO_NOT_LOG) && recordingState)
			{
				//	get write-/front- buffer data objects
				Data* dataF = ring.at(0)->data;

				//	set write time
				dataF->componentTime.now = now;

				//	pre modify log event data
				logEventData.source = dataW->object;

				//	report service entry
				if (tout)
					(*tout) << getObjectName() << "->service(" << dataF->componentTime.now << ")" << brahms::output::D_FULL;

				//	fire EVENT_LOG_SERVICE (type is already set to EVENT_LOG_SERVICE, immediately after firing EVENT_LOG_INIT)
				logEvent.object = dataF->object;
				logEvent.tout = tout;
				/*Symbol err = */logEvent.fire();
				//Symbol err = dataF->module->getHandler()(&inlet->logEvent);

				//	report service entry
				if (tout)
					(*tout) << "(event returned)" << brahms::output::D_FULL;

				//	post modify log event data
				logEventData.count++;
			}

			//	get buffer
			RingBufferItem* buffer = ring.at(ring.writeBuffer);

			//	get number of pipes in this manifold
			//	TODO: THIS COULD BE CACHED
			UINT32 pipeCount = buffer->alternators.size();

#ifdef DEBUG_ALTERNATORS
			cerr << "(before write release)" << endl;
			ring.dump();
#endif

			//	release all streams for write
			for (UINT32 o=0; o<pipeCount; o++)
				buffer->alternators[o]->writeRelease();

#ifdef DEBUG_ALTERNATORS
			cerr << "(after write release)" << endl;
			ring.dump();
#endif


			//	store NULL data object into due slot
			setDueData(NULL);

			//	select new write buffer
			ring.writeBuffer = ring.writeBuffer ? ring.writeBuffer - 1 : ring.size() - 1;

			//	fire remotes
			for (UINT32 r=0; r<remoteInputs.size(); r++)
				remoteInputs[r]->outputWriteReleased(tout);

			//	ok
			return min(nextService, now + samplePeriod);
		}

		//	acquire read lock (if due)
		Symbol OutputPort::readLock(InputPort* port, BaseSamples now, UINT32 bufferIndex, Data** p_data)
		{
			//	if not due, don't go any further
			if (now%samplePeriod) return S_NULL;

			//	get buffer
			RingBufferItem* buffer = ring.at(bufferIndex);

#ifdef DEBUG_ALTERNATORS
			cerr << "(before read lock)" << endl;
			ring.dump();
#endif

			//	lock for read
			Symbol result = buffer->alternators[port->readerIndex]->readLock();
			if (result == C_CANCEL) return C_CANCEL;

#ifdef DEBUG_ALTERNATORS
			cerr << "(after read lock)" << endl;
			ring.dump();
#endif

			//	store due data object into due slot
			port->setDueData(buffer->data);
			if (p_data) *p_data = buffer->data;
			return C_OK;
		}

		//	release read lock (if due)
		BaseSamples OutputPort::readRelease(InputPort* port, BaseSamples now, BaseSamples nextService)
		{
			//	if not due, don't go any further
			if (now%samplePeriod) return min(nextService, now - (now%samplePeriod) + samplePeriod);

			//	get buffer
			RingBufferItem* buffer = ring.at(port->readBuffer);

#ifdef DEBUG_ALTERNATORS
			cerr << "(before read release)" << endl;
			ring.dump();
#endif

			//	release for read
			buffer->alternators[port->readerIndex]->readRelease();

#ifdef DEBUG_ALTERNATORS
			cerr << "(after read release)" << endl;
			ring.dump();
#endif

			//	store NULL data object into due slot
			port->setDueData(NULL);

			//	advance reader
			port->readBuffer = port->readBuffer ? port->readBuffer - 1 : ring.size() - 1;

			//	due
			return min(nextService, now + samplePeriod);
		}

		void OutputPort::connectRemoteInput(InputPortRemote* port)
		{
			//	attach
			remoteInputs.push_back(port);

			//	announce attachment
			port->additionalInputAttached(ring.size());
		}



		void OutputPort::initInterThreadLocks(brahms::output::Source& fout)
		{
			INT32 writeBuffer = ring.writeBuffer;
			UINT32 numberOfBuffers = ring.size();
			UINT32 numberOfReaders = ring.getNumberOfReaders();

			//	for each reader
			for (UINT32 r=0; r<numberOfReaders; r++)
			{
				//	get the read buffer index
				InputPort* inputPort = ring.attachedReaders[r];
				INT32 readBuffer = inputPort->readBuffer;

				//	now, we can set each signal
				for (UINT32 l=0; l<numberOfBuffers; l++)
				{
					//	is read-ready or write-ready?
					INT32 samplesTillRead = readBuffer - l;
					if (samplesTillRead < 0) samplesTillRead += numberOfBuffers;
					INT32 samplesTillWrite = writeBuffer - l;
					if (samplesTillWrite < 0) samplesTillWrite += numberOfBuffers;
					if (samplesTillRead < 0 || samplesTillWrite < 0) ferr << E_INTERNAL;

					//	write-ready has priority, if they're both pointing at the same buffer
					if (samplesTillRead == samplesTillWrite) samplesTillRead++;

					//	choose
					ring.initLocks(r, l, (samplesTillRead > samplesTillWrite));
				}
			}
		}


		void OutputPort::endInitPhase(brahms::output::Source& fout)
		{
			//	set contentHeaderBytes now; in future, we may want to change this value
			//	depending on what happened during CONNECT - that's why we don't set it
			//	until this point
			EventInitComplete eic;
			eic.contentHeaderBytes = sizeof(brahms::base::IPM_HEADER);

			//	for each data object
			for (UINT32 d=0; d<ring.size(); d++)
			{
				//	get data object
				Data* data = ring[d]->data;

				//	create event
				brahms::EventEx event(
					EVENT_INIT_COMPLETE,
					0,
					data,
					&eic,
					true,
					&fout
				);

				//	fire event
				event.fire();
			}
		}

	}
}


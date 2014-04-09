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

	$Id:: deliverer.cpp 2329 2009-11-09 00:13:25Z benjmitch    $
	$Rev:: 2329                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-09 00:13:25 +0000 (Mon, 09 Nov 2009)       $
________________________________________________________________

*/


	
	

////////////////	DELIVERER CLASS

//const UINT32 DELIVERY_PERIOD_SMOOTHING_L = 64;

void DelivererProc(void* arg);

string n2s(UINT32 n)
{
	ostringstream s;
	s << n;
	return s.str();
}

struct Deliverer : public brahms::thread::Thread
{
	Deliverer(
		PushDataHandler p_pushDataHandler,
		void* p_pushDataHandlerArgument,
		UINT32 delivererIndex,
		ChannelInitData channelInitData,
		brahms::base::Core& p_core
	)
		:
		brahms::thread::Thread(brahms::thread::TC_DELIVERER, delivererIndex, p_core),
		core(p_core),
		delivererIPMPool("Deliverer " + n2s(delivererIndex)),
		q(brahms::os::SIGNAL_INFINITE_WAIT, &stop)
	{
		pushDataHandler = p_pushDataHandler;
		pushDataHandlerArgument = p_pushDataHandlerArgument;

		order = 0;		//	delivery order audit
		stop = false;	//	don't stop yet

		//deliveryPeriod = 0.0;
		//deliveryPeriodCount = 0;
	}

	void start(UINT32 TimeoutThreadTerm)
	{
		brahms::thread::Thread::start(TimeoutThreadTerm, DelivererProc, this);
	}

	void push(IPM* msg)
	{
		q.push(msg);
	}

	QueueAuditData queueAudit()
	{
		return q.audit();
	}

	void audit(ChannelAuditData& data)
	{
		delivererIPMPool.audit(data.pool);
	}

	void terminate(brahms::output::Source& fout)
	{
		//	if already done, be silent
		if (stop) return;

		//	the deliverer thread will keep working for ever, unless we stop it, so
		//	we stop it here - this is graceful, though!
		fout << "asking " << getThreadIdentifier() << " to stop" << D_VERB;

		//	set stop
		stop = true;

		//	also call q.release(), in case q is held on the signal (does no harm if not,
		//	and causes instant drop out rather than having to wait SIGNAL_WAIT_STEP
		//	msec to pick up "stop")
		q.release();

		//	wait for termination
		brahms::thread::Thread::terminate(fout);
		fout << "thread " << getThreadIdentifier() << " has now stopped" << D_VERB;
	}

	void ThreadProc();

	IPM* getIPMFromPool()
	{
		IPM* ipm = delivererIPMPool.get();
		return ipm;
	}

private:

	brahms::base::Core& core;

	IPMPool delivererIPMPool;
	
	PushDataHandler pushDataHandler;
	void* pushDataHandlerArgument;

	IPM_FIFO q;

	UINT32 order;
	bool stop;
};




////////////////	REAL DELIVERY PROCEDURE

void DelivererProc(void* arg)
{
	((Deliverer*)arg)->ThreadProc();
}

void Deliverer::ThreadProc()
{
	//	trace exception
	try
	{
		//DOUBLE t0 = 0.0, t1 = 0.0;

		//	(outer) loop until "stop" is set
		while(true)
		{
			//	pull message from queue (see WAIT STATES)
			IPM* msg = NULL;
			REPORT_THREAD_WAIT_STATE_IN("pull()");
			Symbol result = q.pull(msg);
			REPORT_THREAD_WAIT_STATE_OUT("pull()");

			//	if stop is set, break loop and end thread
			if (result == C_CANCEL) break;

			//	if timeout, that's unexpected!
			if (result == E_SYNC_TIMEOUT) 
				ferr << E_INTERNAL << "E_SYNC_TIMEOUT unexpected";

			/*
			//	if queue is not empty (C_NO) we can do timing on the next sample; if it's
			//	empty (C_YES), we can't because we'll have to wait for it to be filled.
			//	so, if we get C_YES, we zero both our sample times to indicate this.
			if (result == C_YES)
			{
				t0 = 0.0;
				t1 = 0.0;
			}
			*/

			//	confirm delivery order - probably not necessary in TCP/IP? in which case could take it out: TODO
			IPM_HEADER& header = msg->header();
			if (header.order != order)
			{
				brahms::error::Error e;
				e.code = E_OUT_OF_ORDER_DELIVERY;
				stringstream ss;
				ss << header.order << "/" << order;
				e.msg = ss.str();
				storeError(e, tout);
				break;
			}
//			else tout << "in order: " << header.order << "/" << order << ")" << D_INFO;
			order++;
				
			//	call message handler function (if C_CANCEL is returned, break loop, discarding all queued messages)
			UINT32 bytes = header.bytesAfterHeaderUncompressed;
			REPORT_THREAD_WAIT_STATE_IN("pushDataHandler()");
			if (pushDataHandler(pushDataHandlerArgument, msg->body(), bytes) == C_CANCEL)
			{
				tout << "PushDataHandler() returned C_CANCEL, discarding remaining queue" << D_VERB;

				//	release message
				msg->release();

				//	break loop, end thread
				break;
			}
			REPORT_THREAD_WAIT_STATE_OUT("pushDataHandler()");

/*
			//	audit delivery time only if queue is overfull
			UINT32 qs = q.size();
			if (qs > DelivererQueueLimit)
			{
				//	do timing
				t0 = t1;
				t1 = rateTimer.elapsed();

				//	can only audit if both times are valid (non-zero)
				if (t0 && t1)
				{
					//	perform averaging
					if (deliveryPeriodCount == DELIVERY_PERIOD_SMOOTHING_L)
						deliveryPeriod *= (((DOUBLE)DELIVERY_PERIOD_SMOOTHING_L)-1.0) / ((DOUBLE)DELIVERY_PERIOD_SMOOTHING_L);
					else
						deliveryPeriodCount++;
					deliveryPeriod += (t1 - t0);
				}
			}
			else
			{
				//	mark that we're not auditing
				t0 = 0.0;
				t1 = 0.0;
			}
			*/



			/*	DOCUMENTATION: IPM_POOL

				PUSHDATA messages are put() back into whichever pool they came from, here.
			*/

			//	release message (return to pool)
			msg->release();
		}

		//	flush q
		q.flush();
	}

	//	trace exception
	catch(brahms::error::Error& e)
	{
		storeError(e, tout);
	}

	//	trace exception
	catch(std::exception se)
	{
		brahms::error::Error e(E_STD, se.what());
		storeError(e, tout);
	}

	//	trace exception
	catch(...)
	{
		brahms::error::Error e(E_UNRECOGNISED_EXCEPTION);
		storeError(e, tout);
	}
}


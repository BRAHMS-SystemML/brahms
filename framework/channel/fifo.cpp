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

$Id:: fifo.cpp 2283 2009-11-02 00:22:31Z benjmitch         $
$Rev:: 2283                                                $
$Author:: benjmitch                                        $
$Date:: 2009-11-02 00:22:31 +0000 (Mon, 02 Nov 2009)       $
________________________________________________________________

*/

#ifndef _CHANNEL_FIFO_CPP_
#define _CHANNEL_FIFO_CPP_

#include <queue>
using std::queue;
#include "base/os.h"
#include "base/ipm.h"
using brahms::base::IPM;
using brahms::base::QueueAuditData;

////////////////	THREAD-SAFE FIRST-IN-FIRST-OUT QUEUE CLASS

//	global false boolean for FIFOs that don't use cancel
const bool FIFO_global_stop = false;

struct IPM_FIFO
{
	IPM_FIFO(UINT32 timeout, const bool* stop)
		:
		signal(timeout, stop)
	{
		m_stop = stop ? stop : &FIFO_global_stop;
		terminated = false;
		m_audit.clear();
	}

	~IPM_FIFO()
	{
		if (q.size())
			____WARN("FIFO not empty at shutdown (" << q.size() << " entries)");
	}

	void push(IPM* t)
	{
		//	protect q
		brahms::os::MutexLocker locker(mutex);

		//	if terminated, just let it go
		if (terminated)
		{
			t->release();
		}
		else
		{
			//	add to queue and set signal
			q.push(t);
			signal.set();
		}
	}

	QueueAuditData audit()
	{
		//	protect q
		brahms::os::MutexLocker locker(mutex);

		//	audit
		QueueAuditData auditCopy;
		auditCopy = m_audit;
		m_audit.clear();
		return auditCopy;
	}

	Symbol pull(IPM*& t)
	{
		//	return result of waitfor() unless it's C_OK (signal was set, now cleared)
		Symbol result = signal.waitfor();

		//	if release() was called, we will get C_OK, but the reason for calling
		//	release() is because stop has been set, so semantically...
		if (*m_stop) return C_CANCEL;

		//	return any non-OK result
		if (result != C_OK) return result;

		//	protect q
		brahms::os::MutexLocker locker(mutex);

		//	assert q is not empty
		if (!q.size()) ferr << E_INTERNAL << "queue unexpectedly empty";

		//	return item at front of queue
		t = q.front();
		q.pop();

		//	mark bytes pulled
		m_audit.bytes += t->size();
		m_audit.items++;

		//	if queue is not yet empty, set signal so that a subsequent call to pull() will succeed
		if (q.size())
		{
			//	and return C_NO (is queue empty?)
			signal.set();
			return C_NO;
		}

		//	return C_YES (is queue empty?)
		return C_YES;
	}

	UINT32 size()
	{
		brahms::os::MutexLocker locker(mutex);
		return q.size();
	}

	void release()
	{
		signal.set();
	}

	void flush()
	{
		brahms::os::MutexLocker locker(mutex);
		while(q.size())
		{
			IPM* ipm = q.front();
			q.pop();
			ipm->release();
		}

		terminated = true;
	}

	brahms::os::Mutex mutex;			//	protects queue for multi-threaded operation
	brahms::os::Signal signal;			//	set when queue is push()ed
	queue<IPM*> q;						//	the queue itself
	bool terminated;					//	once true, the queue will accept no further messages

	//	keep this for checking for release() on stop, at termination
	const bool* m_stop;

	//	self-audit
	QueueAuditData m_audit;
};

#endif // _CHANNEL_FIFO_CPP_

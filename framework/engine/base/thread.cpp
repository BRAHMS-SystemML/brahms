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

	$Id:: thread.cpp 2376 2009-11-15 23:34:44Z benjmitch       $
	$Rev:: 2376                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-15 23:34:44 +0000 (Sun, 15 Nov 2009)       $
________________________________________________________________

*/

#include "base.h"
#include "support/os.h"

#ifdef __NIX__
# include <pthread.h>
# include <errno.h>
#endif

#ifdef __OSX__
# include <mach/mach.h>
#endif

inline string f2s(DOUBLE n)
{
    ostringstream s;
    s << n;
    return s.str();
}

inline string u2s(UINT64 n)
{
    ostringstream s;
    s << n;
    return s.str();
}

namespace brahms
{
    namespace thread
    {
        ThreadBase::ThreadBase(ThreadClass threadClass, UINT32 threadIndex,
                               brahms::base::Core& p_core)
            : core(p_core)
        {
            // defaults
            flags = 0;

            // store passed data
            this->threadClass = threadClass;
            this->threadIndex = threadIndex;

            // create thread identifier
            switch (threadClass)
            {
            case TC_CALLER: threadIdentifier = 'C'; break;
            case TC_WORKER: threadIdentifier = 'W'; break;
            case TC_SENDER: threadIdentifier = 'S'; break;
            case TC_RECEIVER: threadIdentifier = 'R'; break;
            case TC_DELIVERER: threadIdentifier = 'D'; break;
            default: ferr << E_INTERNAL << "unknown thread class";
            }
            if (threadClass != TC_CALLER) {
                threadIdentifier += u2s(threadIndex+1);
            }
        }

        const char* ThreadBase::getThreadIdentifier()
        {
            return threadIdentifier.c_str();
        }

        INT32 ThreadBase::getThreadIndex()
        {
            return threadIndex;
        }

        ThreadClass ThreadBase::getThreadClass()
        {
            return threadClass;
        }

        void ThreadBase::storeError(brahms::error::Error& error, brahms::output::Source& tout)
        {
            setFlag(F_THREAD_ERROR);
            core.condition.set(brahms::base::COND_LOCAL_ERROR);
            tout << "STORING \"" << brahms::base::symbol2string(error.code)
                 << ": " << error.msg << "\" IN THREAD \"" << threadIdentifier
                 << "\"" << brahms::output::D_VERB_ERROR;
            error.debugtrace("caught in thread \"" + threadIdentifier + "\"");
            error.debugtrace("caught after " + f2s(threadTimer.elapsed()) + " secs (thread time)");
            core.errorStack.push(error);
        }

        bool ThreadBase::flagState(const UINT32 flag)
        {
            return flags & flag;
        }

        void ThreadBase::setFlag(const UINT32 flag)
        {
            flags |= flag;
        }

        ThreadProcRet osThreadProc(ThreadProcArg arg)
        {
            /*
              The only reason we call this function instead of calling
              the thread main procedure directly is that it allows us
              to do some in-thread operations without revealing these
              to the user of class Thread. Specifically, we can set
              the thread state before we call thread main, and we can
              set the Thread state after thread main returns.
            */

            // get wrapper
            Thread* thread = (Thread*)arg;

            // mark timing
            thread->markCPUTime(PHS_PREV);

#ifdef __NIX__
            // set OS thread cancel state
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
            pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
#endif
            // call passed thread main procedure
            try
            {
                // initial message
                thread->tout << "THREAD ENTER" << brahms::output::D_VERB_HILITE;
                thread->threadProc(thread->threadProcArg);
            }
            catch(brahms::error::Error& e)
            {
                thread->tout << e.format(brahms::FMT_TEXT, true) << "\n\n"
                             << "the client thread procedure raised an exception (above) "
                             << "(this is illegal: it should handle its own exceptions)"
                             << brahms::output::D_WARN;
            }
            catch(exception e)
            {
                thread->tout << "E_STD: " << e.what() << "\n\n"
                             << "client thread procedure raised an exception (above) "
                             << "(this is illegal: it should handle its own exceptions)"
                             << brahms::output::D_WARN;
            }
            catch(...)
            {
                thread->tout << "the client thread procedure raised an exception (...) "
                             << "(this is illegal: it should handle its own exceptions)"
                             << brahms::output::D_WARN;
            }

            // thread-safety
            {
                brahms::os::MutexLocker locker(thread->threadMutex);

                // finalize timing
                thread->threadCPUTime.finalize();

                // final message
                thread->tout << "THREAD EXIT" << brahms::output::D_VERB_HILITE;

                // change state
                thread->state = TS_FINISHED;

                // return a dummy unused value
                return 0;
            }
        }

        Thread::Thread(ThreadClass threadClass, UINT32 threadIndex, brahms::base::Core& core)
            : ThreadBase(threadClass, threadIndex, core)
        {
            // set state
            state = TS_VIRGIN;
            threadProc = NULL;
            threadProcArg = NULL;
            TimeoutThreadTerm = 0;
            osThread = 0;

            // connect to sink
            tout.connect(&core.sink, getThreadIdentifier());
        }

        void Thread::start(UINT32 TimeoutThreadTerm, ThreadProc threadProc, void* threadProcArg)
        {
            this->TimeoutThreadTerm = TimeoutThreadTerm;
            this->threadProc = threadProc;
            this->threadProcArg = threadProcArg;

            {
                // thread-safety
                brahms::os::MutexLocker locker(threadMutex);

                // check state
                if (state != TS_VIRGIN) {
                    ferr << E_INTERNAL << "can only start() from TS_VIRGIN";
                }

                // change state
                state = TS_ACTIVE;
            }

#ifdef __WIN__

            // create thread and set priority
            if (!(osThread = CreateThread(NULL, 0,
                                          (LPTHREAD_START_ROUTINE)&osThreadProc,
                                          this, 0, 0 ))) {
                // change start
                state = TS_START_FAILED;
                ferr << E_OS
                     << "Thread::start(): failed to create Windows thread";
            }

            /*
              WINDOWS THREAD PRIORITY CONSTANTS

              THREAD_PRIORITY_IDLE
              THREAD_PRIORITY_LOWEST
              THREAD_PRIORITY_BELOW_NORMAL
              THREAD_PRIORITY_NORMAL
              THREAD_PRIORITY_ABOVE_NORMAL
              THREAD_PRIORITY_HIGHEST
              THREAD_PRIORITY_TIME_CRITICAL

              Thread Priority is governed by the premise that we want to keep
              queues as empty as possible, since the converse (keeping them as
              full as possible) will use up memory and allow peer voices to
              go lone ranger. To achieve this, priority is always higher in a
              thread A that depends on input from a thread B, so that A always
              goes when it can, and B goes only when A gets stuck. The SYSTEM
              thread has to have a higher priority still, so it can poll for
              hangs in the other threads, but since it spends most of its time
              asleep this does not interfere with the functional policy.
            */

            switch(getThreadClass())
            {
            case TC_RECEIVER:
            {
                if (!SetThreadPriority(osThread, THREAD_PRIORITY_IDLE))
                    ferr << E_OS << "failed to set thread priority";
                break;
            }

            case TC_DELIVERER:
            {
                if (!SetThreadPriority(osThread, THREAD_PRIORITY_LOWEST))
                    ferr << E_OS << "failed to set thread priority";
                break;
            }

            case TC_WORKER:
            {
                if (!SetThreadPriority(osThread, THREAD_PRIORITY_BELOW_NORMAL))
                    ferr << E_OS << "failed to set thread priority";
                break;
            }

            case TC_SENDER:
            {
                if (!SetThreadPriority(osThread, THREAD_PRIORITY_NORMAL))
                    ferr << E_OS << "failed to set thread priority";
                break;
            }
            }
#endif

#ifdef __NIX__
            // create thread attribute
            pthread_attr_t attr;
            if (pthread_attr_init(&attr)) {
                ferr << E_OS << "failed to create thread attribute";
            }

            // set up attribute
            if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE)) {
                pthread_attr_destroy(&attr);
                ferr << E_OS << "failed to set thread attribute";
            }

# ifdef UNUSED_SCHEDULING_PRIORITY_CODE
            // set thread priority
            sched_param param;
            int mn = sched_get_priority_min(SCHED_FIFO);
            int mx = sched_get_priority_max(SCHED_FIFO);
            //fout << "mn/mx " << mn << "/" << mx << D_INFO;
            if (pthread_attr_getschedparam (&attr, &param))
            {
                pthread_attr_destroy(&attr);
                ferr << E_OS << "failed to get thread sched param";
            }

            switch(cls)
            {
            case TC_COMMS:
            {
                param.sched_priority = mn + 1;
                break;
            }

            case TC_DELIVERY:
            {
                param.sched_priority = mn + 2;
                break;
            }

            case TC_WORKER:
            {
                param.sched_priority = mn + 3;
                break;
            }
            }

            if (pthread_attr_setschedparam (&attr, &param))
            {
                pthread_attr_destroy(&attr);
                ferr << E_OS << "failed to set thread sched param";
            }
            if (pthread_attr_getschedparam (&attr, &param))
            {
                pthread_attr_destroy(&attr);
                ferr << E_OS << "failed to get thread sched param";
            }
            fout << "set pri " << param.sched_priority << D_INFO;
# endif // UNUSED_SCHEDULING_PRIORITY_CODE

            // create thread (use default priority)
            if (pthread_create(&osThread, &attr, &osThreadProc, this)) {
                // change state
                int theError = errno;
                state = TS_START_FAILED;
                pthread_attr_destroy(&attr);
                ferr << E_OS
                     << "Thread::start(): failed to create (posix) thread using "
                     << "default priority. errno: " << theError;
            }

            // destroy thread attribute
            if (pthread_attr_destroy(&attr)) {
                ferr << E_OS << "failed to destroy thread attribute";
            }

            // Cannot set priority because the default scheduler only supports priority 0.
            // Of course the superuser can use SCHED_FIFO/SCHED_RR but running BRAHMS with
            // SCHED_FIFO is a security nightmare and may hang your system if your threads
            // misbehaved.
#endif
        }

        bool Thread::isActive()
        {
            // if state is TS_ACTIVE
            return state == TS_ACTIVE;
        }

        void Thread::markCPUTime(RunPhase phase)
        {
            // thread-safety
            brahms::os::MutexLocker locker(threadMutex);

            /*
              On Linux, you can't get thread times after it has exited,
              so we musn't call this after the thread has exited
            */

            brahms::time::TIME_CPU cpu = getthreadtimes();

            switch(phase)
            {
            case PHS_PREV:
                threadCPUTime.prev = cpu;
                break;
            case PHS_INIT:
                threadCPUTime.init = cpu;
                break;
            case PHS_RUN:
                threadCPUTime.run = cpu;
                break;
            case PHS_TERM:
                threadCPUTime.term = cpu;
                break;
            }
        }

        void Thread::terminate(brahms::output::Source& fout, UINT32 t_alreadyWaited)
        {
            // thread-safety
            brahms::os::MutexLocker locker(threadMutex);

            // we only have to actually *do* something if the OS thread is running.
            // the OS thread is not started until the thread state has been changed
            // from TS_VIRGIN to TS_ACTIVE (though it may go straight to TS_START_FAILED
            // if the OS thread fails to start). following that, it will go to TS_FINISHED
            // if the thread exits normally (even following an error), or it will stay
            // at TS_ACTIVE (if the thread hangs or segfaults). if we have already
            // terminated the thread, we will be at TS_TERMINATE. so:
            //
            // WARNING: it's possible we should protect state change with a mutex, to
            // avoid mistakes when two threads access the Thread object at once. i don't
            // think this will ever happen, though, so we should be alright.
            //
            // TS_VIRGIN: OS thread not running
            // TS_ACTIVE: OS thread running (though it may be about to finish)
            // TS_FINISHED: OS thread finished (or just about to)
            // TS_START_FAILED: OS thread not running
            // TS_TERMINATED: OS thread not running (presumably)

            switch(state)
            {
            case TS_VIRGIN:
                fout << "thread " << getThreadIdentifier() << " terminate()d from TS_VIRGIN (no action)" << brahms::output::D_VERB;
                state = TS_TERMINATED;
                return;

            case TS_START_FAILED:
                fout << "thread " << getThreadIdentifier() << " terminate()d from TS_START_FAILED (no action)" << brahms::output::D_VERB;
                state = TS_TERMINATED;
                return;

            case TS_FINISHED:
                fout << "thread " << getThreadIdentifier() << " terminate()d from TS_FINISHED (no action)" << brahms::output::D_VERB;
                state = TS_TERMINATED;

#ifdef __NIX__
                // BUG FOUND BY valgrind!
                // must detach pthread after it's completed
                fout << "pthread_detach osThread..." << brahms::output::D_VERB;
                pthread_detach(osThread);
#endif
                return;

            case TS_TERMINATED:
                // no action, and no spurious messages - that way, we can call
                // this function as many times as we want :)
                return;

            case TS_ACTIVE:
                // no action
                break;
            }

            // state must then be TS_ACTIVE, so we will take some action. first, we try
            // and let the thread end gracefully

            UINT32 t_start = threadTimer.elapsedMS();
            UINT32 t_waited = 0;

            while(true)
            {
                // time
                t_waited = threadTimer.elapsedMS() - t_start + t_alreadyWaited;

                // finished
                if (!isActive())
                {
                    fout << "thread " << getThreadIdentifier() << " terminate()d from TS_ACTIVE (graceful finish in " << dec << t_waited << "ms)" << brahms::output::D_VERB;
                    state = TS_TERMINATED;
#ifdef __NIX__
                    // BUG FOUND BY valgrind!
                    // must detach pthread after it's completed
                    pthread_detach(osThread);
#endif
                    return;
                }

                // if already detected as hung, we won't wait the term
                // timeout also NOTE: i won't call pthread_detach,
                // here, since i'm not 100% that that couldn't cause a
                // seg fault (resources freed underneath a thread that
                // may still be running)
                if (flagState(F_THREAD_HUNG)) break;

                // wait a bit
                if (t_waited > TimeoutThreadTerm) break;

                /*
                  NB it's important we sleep for non-zero time, here, since the thread
                  we're terminating (or other threads that need to run before that
                  thread will terminate) may be of lower priority than the thread we're
                  running in. if we call sleep(0), we'll come straight back, in that
                  case, and thus the thread will never (or only very slowly) terminate.
                */
                // unlock mutex while we sleep
                threadMutex.release();
                brahms::os::msleep(1);
                threadMutex.lock();
            }

            // ok, so it's kill you want, is it?
            // (no point in warning that we're killing it if we're going to throw E_THREAD_HUNG in any case...)
            if (!flagState(F_THREAD_HUNG)) {
                fout << "thread " << getThreadIdentifier() << " terminate()d from TS_ACTIVE (not finished in " << dec << t_waited << "ms, thread KILLED - this is not good)" << brahms::output::D_WARN;
            }
            state = TS_TERMINATED;

            // NOTE: i won't call pthread_detach, here, since i'm not 100% that that couldn't cause a seg fault (resources freed underneath a thread that may still be running)

#ifdef __WIN__
            // ask thread to cancel
            bool success = TerminateThread(osThread, 0);
#endif

#ifdef __NIX__

            // SEE DEVELOPER DOCUMENTATION NOTE "pthread_cancel"

            // we don't ask pthread to cancel here because it interferes with C++'s exception
            // handling; search for Dave Butenhof's posts on comp.programming.threads,
            // particularly the one with message ID <449093e2$1@usenet01.boi.hp.com>:
            //
            // "Ultimately, if you want portability, don't mix pthreads and C++ at all.
            // In particular, though, avoid cancellation and C++ exceptions in any
            // program where you do mix them. It'll work smoothly on a very few
            // platforms, inconsistently and awkwardly on a few others, and not at all
            // on most."
            //
            // if we /do/ call pthread_cancel, we get an ugly error message...
            //
            // "FATAL: exception not rethrown"
            //
            // which is little better than an abort. avoiding an abort, incidentally, is
            // why we're trying to make this thread stop in the first place. essentially,
            // we come here if the thread has hung, in which case we have no control over
            // its code flow, and so we'd like to just kill it dead. but seems, with C++
            // and pthreads, we can't do it.
            //
            // unfortunately, as it stands, this means that if a thread hangs, and we unload
            // the module it is running in (e.g. the component module), then we get an abort,
            // presumably because the thread is still running in the memory space where the
            // module used to be mapped. i don't see any way to fix this, really. one approach
            // would be to *not* explicitly unload any modules, but it seems pretty lame...
            // besides, that probably wouldn't lead to a clean shutdown; might leave the
            // threads (processes in pthreads) running?
            //
            // TODO: use a different threads implementation, rather than pthreads?

            //pthread_cancel(osThread);
#endif
        }

        Thread::~Thread()
        {
            // report if still active
            if (isActive())
            {
                // report
                ____WARN("thread " << getThreadIdentifier() << " still isActive() unexpectedly in destructor");
            }

#ifdef __WIN__
            // delete it
            CloseHandle(osThread);
#endif
        }

        // THREAD TIME
        brahms::time::TIME_CPU getthreadtimes()
        {
            brahms::time::TIME_CPU cpu;

#ifdef __WIN__
            FILETIME f1, f2, f3, f4;
            if (!GetThreadTimes(GetCurrentThread(), &f1, &f2, &f3, &f4))
            {
                //fout << "GetThreadTimes() failed" << D_INFO;
                return cpu;
            }

            cpu.kernel = (((double)f3.dwHighDateTime) * 429.4967296) + ((double)f3.dwLowDateTime) * 1.0e-7;
            cpu.user = (((double)f4.dwHighDateTime) * 429.4967296) + ((double)f4.dwLowDateTime) * 1.0e-7;
#endif

#ifdef __OSX__
            // fall back to this, if the proposal doesn't work
            // memset(&cpu, 0, sizeof(cpu));

            // Proposal...
            // check if this works

            // http://www.fi.muni.cz/~xholer/macosx/xnu/thread_info.html
            thread_basic_info info;
            mach_msg_type_number_t my_info_size;
            mach_port_t my_thread_self;
            kern_return_t kr;
            my_thread_self = mach_thread_self();
            my_info_size = sizeof(info)/sizeof(int);
            kr = thread_info(my_thread_self, THREAD_BASIC_INFO, (thread_info_t)&info, &my_info_size);
            mach_port_deallocate(mach_task_self(), my_thread_self);

            // http://felinemenace.org/~nemo/mach/manpages/thread_basic_info.html
            // http://www.gnu.org/software/hurd/gnumach-doc/Host-Time.html
            DOUBLE u = info.user_time.seconds * 1e6 + info.user_time.microseconds;
            cpu.user = u * 1.0e-6;
            DOUBLE k = info.system_time.seconds * 1e6 + info.system_time.microseconds;
            cpu.kernel = k * 1.0e-6;
#endif

#ifdef __GLN__

            clockid_t clock_id;
            struct timespec tp;

            int ret = pthread_getcpuclockid(pthread_self(), &clock_id);
            if (ret)
            {
                //if (ret == ESRCH) fout << "pthread_getcpuclockid() failed in getthreadtimes() because the thread no longer exists" << D_INFO;
                //else fout << "pthread_getcpuclockid() failed in getthreadtimes() for an unknown reason" << D_INFO;
                return cpu;
            }

            if (clock_gettime(clock_id, &tp))
            {
                //fout << "clock_gettime() failed in getthreadtimes()" << D_INFO;
                return cpu;
            }

            cpu.kernel = 0.0;
            cpu.user = ((double)tp.tv_sec) + ((double)tp.tv_nsec) * 1.0e-9;

#endif

            return cpu;
        }
    } // namespace thread
} // namespace brahms

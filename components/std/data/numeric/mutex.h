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

	$Id:: data_numeric.cpp 2410 2009-11-20 10:18:18Z benjmitch $
	$Rev:: 2410                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-20 10:18:18 +0000 (Fri, 20 Nov 2009)       $
________________________________________________________________

*/

#ifndef _COMPONENTS_MUTEX_H_
#define _COMPONENTS_MUTEX_H_

#ifdef __WIN__
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __NIX__
#include <pthread.h>
#endif

struct Mutex
{
    Mutex();
    ~Mutex() throw();

    void lock();
    void release();

private:
#ifdef __WIN__
    //	native state
    CRITICAL_SECTION cs;
#endif
#ifdef __NIX__
    //	native mutex
    pthread_mutex_t mutex;
#endif

};

struct MutexLocker
{
    MutexLocker(Mutex& p_mutex) : mutex(p_mutex) {
        mutex.lock();
    }
    ~MutexLocker() {
        mutex.release();
    }

private:
    Mutex& mutex;
};

/*	MUTEX

        We use a critical section on windows, is a little faster,
        and we don't need cross-process sync.

        NOTE that a windows CRITICAL_SECTION is *not* a mutex in
        the conventional sense - it can be locked as many times as
        required by the same thread, i.e. this code:

        Mutex mutex;
        mutex.lock();
        mutex.lock();
        mutex.release();
        mutex.release();
        print "OK";

        will work just fine. but it will prevent access by another
        thread.
*/

#ifdef __WIN__
Mutex::Mutex()
{
    InitializeCriticalSection(&cs);
}

Mutex::~Mutex() throw()
{
    DeleteCriticalSection(&cs);
}

void Mutex::lock()
{
    EnterCriticalSection(&cs);
}

void Mutex::release()
{
    LeaveCriticalSection(&cs);
}
#endif

#ifdef __NIX__
Mutex::Mutex()
{
    if (pthread_mutex_init(&mutex, NULL)) berr << "pthread_mutex_init() failed";
}

Mutex::~Mutex() throw()
{
    pthread_mutex_destroy(&mutex);
}

void Mutex::lock()
{
    if (pthread_mutex_lock(&mutex)) berr << "pthread_mutex_lock() failed";
}

void Mutex::release()
{
    if (pthread_mutex_unlock(&mutex)) berr << "pthread_mutex_unlock() failed";
}
#endif

#endif // _COMPONENTS_MUTEX_H_

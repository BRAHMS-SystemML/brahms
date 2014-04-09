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

	$Id:: mpi-support.cpp 2419 2009-11-30 18:33:48Z benjmitch  $
	$Rev:: 2419                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)       $
________________________________________________________________

*/



//#include "/usr/local/abrg/mpich2-1.0.8/include/mpi.h"

#include "mpi.h"


/*

string mpiErrorMsg()
{
	char buffer[1024];
	int len = 0;
	MPI::Get_error_string(MPI::mpi_errno, buffer, len);
	buffer[len] = 0;
	return buffer;
}

*/





/*
	Why do we need MPI_THREAD_MULTIPLE?
	----------------------------------------------------------------
	
	* we don't know when our peers will send us messages, in general, because
		PUSHDATA messages need to be sent on potentially complex time intervals.
		we could, in principle, work it out, but that would mean computing the
		timing logic for all voices in each individual voice. booooring.
	
	* when we have a message to send, and nothing waiting to be received, we
		must, of course, send it, because other voices may not send us anything
		until they receive it.
		
	* MPI channels are (effectively) half duplex, such that two send()s
		occurring at either end will both block (each needs recv() to be called
		at the other end). this, with the above BRAHMS logic, means we have to
		call recv() in a separate thread from the one calling send(), or we will
		deadlock on occasion.
		
	* therefore, we have to have send() and recv() happening in different
		threads, and they may call the MPI layer concurrently. that requires
		MPI_THREAD_MULTIPLE. at that point, we can allow as many threads as
		we like, since they add no further overhead. note, then, that MPI_Init()
		and MPI_Term() are called from the caller thread.

	Who offers it?
	----------------------------------------------------------------
	
	* MPI_THREAD_MULTIPLE is available in MPICH2 1.2 (nix) and possibly on
		windows if we built it ourselves, however the standard windows binary
		does not offer it.
		
	* MPICH2 does not offer myrinet support (at time of writing, though i
		haven't checked in 1.2) so we can't use BRAHMS & MPI & MYRINET together
		on ACE until the MPICH2 developers add support.
		
	* As i remember, OpenMPI did not offer MPI_THREAD_MULTIPLE when we looked
		into this (first half 2009), but that may have changed since. In any
		case, that's why we switched to MPICH2 (which lacks Myrinet support).

	Compile model
	----------------------------------------------------------------
	
	* As far as i know, at present, only the MPI so is being compiled with
		mpic++, and everything is working fine. this is the desirable state of
		affairs that we never managed to reach when using OpenMPI (we got all
		kinds of aborts in that configuration).
*/





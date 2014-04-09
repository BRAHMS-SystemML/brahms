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

	$Id:: alternator.h 2283 2009-11-02 00:22:31Z benjmitch     $
	$Rev:: 2283                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-02 00:22:31 +0000 (Mon, 02 Nov 2009)       $
________________________________________________________________

*/




#ifndef INCLUDED_BRAHMS_SYSTEMML_ALTERNATOR
#define INCLUDED_BRAHMS_SYSTEMML_ALTERNATOR



////////////////	NAMESPACE

namespace brahms
{
	namespace systemml
	{



	////////////////	ALTERNATOR

		/*

			The ALTERNATOR protects a read/write buffer from simultaneous
			read/write access. Reader and writer are constrained to
			operate alternately on the buffer. Using init(), you can
			choose whose turn comes first.

		*/

		//#define DEBUG_ALTERNATORS

		class Alternator
		{

			brahms::os::Signal readReady;
			brahms::os::Signal writeReady;

			/*	DOCUMENTATION: INTERTHREAD_SIGNALLING

				if the alternator is formed between two threads that are, in fact, the
				same thread, then it is redundant, and it need not do anything at all :)
			*/

			bool redundant;

		public:

			//	debug data
			char debugState;

			Alternator(UINT32 timeout, const bool* cancel, bool p_redundant)
				:
				readReady(timeout, cancel),
				writeReady(timeout, cancel)

			{
				//	default debugState (never changes if not debugging)
				debugState = '?';

			#ifdef DEBUG_ALTERNATORS
				stringstream ss;
				ss << "#### 0x" << hex << (UINT64)this << " INIT TO " << debugState << "\n";
				cerr << ss.str().c_str();
			#endif

				//	record data
				redundant = p_redundant;
			};

			void init(bool beginAtWriteReady)
			{
				if (beginAtWriteReady)
				{
					writeReady.set();
			#ifdef DEBUG_ALTERNATORS
					debugState = 'w';
			#endif
				}

				else
				{
					readReady.set();
			#ifdef DEBUG_ALTERNATORS
					debugState = 'r';
			#endif
				}
			}

			Alternator(const Alternator& src)
				:
				readReady(0, 0),
				writeReady(0, 0)
			{
				//	let's make this illegal, since with Signals involved
				//	we can't copy it anyway
				ferr << E_INTERNAL << "called copy ctor of Alternator";
			}

			bool getRedundant()
			{
				return redundant;
			}

			Symbol readLock()
			{
				//	redundancy
				if (redundant) return C_OK;

				//	wait for signal
			#ifdef DEBUG_ALTERNATORS
				stringstream ss;
				ss << "#### 0x" << hex << (UINT64)this << " READ LOCK (TRY)" << "\n";
				cerr << ss.str().c_str();
			#endif
				Symbol result = readReady.waitfor();
				if (result == E_SYNC_TIMEOUT)
					ferr << E_INTERNAL << "E_SYNC_TIMEOUT waiting for READ_READY (wait should be infinite)";
			#ifdef DEBUG_ALTERNATORS
				stringstream ss2;
				ss2 << "#### 0x" << hex << (UINT64)this << " READ LOCK (OK)" << "\n";
				cerr << ss2.str().c_str();
			#endif

			#ifdef DEBUG_ALTERNATORS
				debugState = 'R';
			#endif

				return result;
			}

			Symbol readRelease()
			{
				//	redundancy
				if (redundant) return C_OK;

				//	set signal
			#ifdef DEBUG_ALTERNATORS
				stringstream ss;
				ss << "#### 0x" << hex << (UINT64)this << " READ RELEASE" << "\n";
				cerr << ss.str().c_str();
			#endif

			#ifdef DEBUG_ALTERNATORS
				debugState = 'w';
			#endif

				writeReady.set();
				return C_OK;
			}

			Symbol writeLock()
			{
				//	redundancy
				if (redundant) return C_OK;

				//	wait for signal
			#ifdef DEBUG_ALTERNATORS
				stringstream ss;
				ss << "#### 0x" << hex << (UINT64)this << " WRITE LOCK (TRY)" << "\n";
				cerr << ss.str().c_str();
			#endif
				Symbol result = writeReady.waitfor();
				if (result == E_SYNC_TIMEOUT)
					ferr << E_INTERNAL << "E_SYNC_TIMEOUT waiting for WRITE_READY (wait should be infinite)";
			#ifdef DEBUG_ALTERNATORS
				stringstream ss2;
				ss2 << "#### 0x" << hex << (UINT64)this << " WRITE LOCK (OK)" << "\n";
				cerr << ss2.str().c_str();
			#endif

			#ifdef DEBUG_ALTERNATORS
				debugState = 'W';
			#endif

				return result;
			}

			Symbol writeRelease()
			{
				//	redundancy
				if (redundant) return C_OK;

				//	set signal
			#ifdef DEBUG_ALTERNATORS
				stringstream ss;
				ss << "#### 0x" << hex << (UINT64)this << " WRITE RELEASE" << "\n";
				cerr << ss.str().c_str();
			#endif

			#ifdef DEBUG_ALTERNATORS
				debugState = 'r';
			#endif

				readReady.set();
				return C_OK;
			}

		};



////////////////	NAMESPACE

	}
}



////////////////	INCLUSION GUARD

#endif

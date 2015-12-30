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

	$Id:: comm.cpp 2392 2009-11-18 16:24:13Z benjmitch         $
	$Rev:: 2392                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-18 16:24:13 +0000 (Wed, 18 Nov 2009)       $
________________________________________________________________

*/

#ifndef _ENGINE_MAIN_COMM_H_
#define _ENGINE_MAIN_COMM_H_

#include "BrahmsConfig.h"
#include "main/enginedata.h"
#include "channel.h"

#define fout (engineData.core.caller.tout)

using namespace brahms::output;
using namespace brahms::math;



////////////////	NAMESPACE

namespace brahms
{
	namespace comms
	{



	////////////////	MAP FUNCTIONS

		brahms::channel::CommsInitData initChannelModule(
			EngineData& engineData,
			brahms::channel::Protocol protocol,
			brahms::channel::CommsInitFunc*& commsInitFunc,
			brahms::channel::CreateChannelFunc*& createChannelFunc
			)
		{
			//	compress function
			CompressFunction* compressFunction = NULL;

			//	map compress module
#if STANDALONE_INSTALL
			string path = brahms::os::getenv("SYSTEMML_INSTALL_PATH", false);
                        if (path.empty()) {
                            path = INSTALL_PREFIX;
                            path += brahms::os::PATH_SEPARATOR;
                        }
#else
			string path(INSTALL_PREFIX);
                        path += brahms::os::PATH_SEPARATOR;
#endif
			path += brahms::os::COMPRESS_MODULE_FORM;

			//	allow fail
			brahms::module::Module* module = NULL;
			try
			{
				module = engineData.loader.loadModule(path.c_str(), engineData.core.caller.tout, &engineData.systemInfo, NULL, 0);
			}
			catch(brahms::error::Error& e)
			{
				if (e.code == E_FAILED_LOAD_MODULE)
				{
					//	fail silently
					module = NULL;
				}
			}

			//	map compress function
			if (module)
			{
				compressFunction = (CompressFunction*) module->map("Compress");
				if (!compressFunction) ferr << E_INTERNAL << "module \"" << path << "\" did not export \"compressFunction()\"";
			}

			//	map channel module
#if STANDALONE_INSTALL
			path = brahms::os::getenv("SYSTEMML_INSTALL_PATH", false);
                        if (path.empty()) {
                            path = INSTALL_PREFIX;
                            path += brahms::os::PATH_SEPARATOR;
                        }
#else
			path = INSTALL_PREFIX;
                        path += brahms::os::PATH_SEPARATOR;
#endif
			path += brahms::os::CHANNEL_MODULE_FORM;
			switch (protocol)
			{
				case brahms::channel::PROTOCOL_MPI: brahms::text::grep(path, "((PROTOCOL))", "mpich2"); break;
				case brahms::channel::PROTOCOL_SOCKETS: brahms::text::grep(path, "((PROTOCOL))", "sockets"); break;
				default: ferr << E_INTERNAL << "protocol unrecognised";
			}
			module = engineData.loader.loadModule(path.c_str(), engineData.core.caller.tout, &engineData.systemInfo, NULL, 0);

			//	map functions
			commsInitFunc = (brahms::channel::CommsInitFunc*) module->map("commsInit");
			if (!commsInitFunc) ferr << E_INTERNAL << "module \"" << path << "\" did not export \"commsInit()\"";
			createChannelFunc = (brahms::channel::CreateChannelFunc*)module->map("createChannel");
			if (!createChannelFunc) ferr << E_INTERNAL << "module \"" << path << "\" did not export \"createChannel()\"";

			//	init module
			return commsInitFunc(engineData.core, compressFunction);
		}



	////////////////	COMMS LAYER

		Comms::Comms(brahms::EngineData& p_engineData)
			:
			engineData(p_engineData)
		{
			signalledPeers = false;
		}

		Comms::~Comms()
		{
		}

		void Comms::init()
		{
			//	create channels (includes listen)
			for (VoiceIndex remoteVoiceIndex=0; remoteVoiceIndex<engineData.execution.voices.size(); remoteVoiceIndex++)
			{
				//	voice
				const Voice& voice = engineData.execution.voices[remoteVoiceIndex];

				//	NULL channel to ourselves (never used)
				if (remoteVoiceIndex == engineData.core.getVoiceIndex())
				{
					channels.push_back(NULL);
					continue;
				}

				//	init channel module
				brahms::channel::CreateChannelFunc* createChannelFunc = NULL;

				//	have comms module create the channel
				brahms::channel::ChannelInitData initData;
				initData.remoteVoiceIndex = remoteVoiceIndex;
				initData.remoteAddress = voice.address;
				brahms::channel::Channel* channel = createChannelFunc(engineData.core, initData);

				//	store
				channels.push_back(channel);
			}

			//	open channels
			for (VoiceIndex remoteVoiceIndex=0; remoteVoiceIndex<engineData.execution.voices.size(); remoteVoiceIndex++)
			{
				if (remoteVoiceIndex == engineData.core.getVoiceIndex())
					continue; // ignore the dummy channel

				//	report
				channels[remoteVoiceIndex]->open(fout);
			}
		}

		void Comms::term()
		{
			//	report
			fout << "Comms::term() called (" << channels.size() << " channels open)" << D_VERB;

			//	close all channels
			for (VoiceIndex remoteVoiceIndex=0; remoteVoiceIndex<channels.size(); remoteVoiceIndex++)
			{
				if (channels[remoteVoiceIndex])
				{
					try
					{
						channels[remoteVoiceIndex]->close(fout);
					}
					catch(brahms::error::Error e)
					{
						engineData.core.caller.storeError(e, fout);
					}
					catch(...)
					{
						brahms::error::Error e;
						e.code = E_INTERNAL;
						e.msg = "unknown exception whilst closing channel";
						engineData.core.caller.storeError(e, fout);
					}
				}
			}

			//	report
			fout << "Comms::term() channels closed" << D_VERB;

			//	terminate all channels
			for (VoiceIndex remoteVoiceIndex=0; remoteVoiceIndex<channels.size(); remoteVoiceIndex++)
			{
				if (channels[remoteVoiceIndex])
				{
					try
					{
						channels[remoteVoiceIndex]->terminate(fout);
					}
					catch(brahms::error::Error e)
					{
						engineData.core.caller.storeError(e, fout);
					}
					catch(...)
					{
						brahms::error::Error e;
						e.code = E_INTERNAL;
						e.msg = "unknown exception whilst terminating channel";
						engineData.core.caller.storeError(e, fout);
					}
				}
			}

			//	report
			fout << "Comms::term() channels terminated" << D_VERB;

			//	delete channels
			for (VoiceIndex remoteVoiceIndex=0; remoteVoiceIndex<channels.size(); remoteVoiceIndex++)
			{
				if (channels[remoteVoiceIndex])
				{
					try
					{
						delete channels[remoteVoiceIndex];
					}
					catch(...)
					{
						brahms::error::Error e;
						e.code = E_INTERNAL;
						e.msg = "exception whilst delete()ing channel";
						engineData.core.caller.storeError(e, fout);
					}
				}
			}

			//	report
			fout << "Comms::term() channels deleted" << D_VERB;

			//	clear channels
			channels.clear();
		}


		void Comms::signalPeers(UINT8 signal)
		{
			//	exclusive
			{
				brahms::os::MutexLocker locker(signalledPeersMutex);

				//	only ever send one signal (PEER ERROR)
				if (signalledPeers)
				{
					engineData.core.caller.tout << "not signalling peers with " << brahms::base::TranslateIPMTAG(signal) << " (already signalled)" << D_VERB;
					return;
				}
				else signalledPeers = true;
			}

			//	report
			engineData.core.caller.tout << "signalling peers with " << brahms::base::TranslateIPMTAG(signal) << D_VERB;

			//	for each channel
			for (VoiceIndex remoteVoiceIndex=0; remoteVoiceIndex<channels.size(); remoteVoiceIndex++)
			{
				//	except ourselves
				if (!channels[remoteVoiceIndex]) continue;

				//	must not throw here, since this is called when an error
				//	has already occurred, and we want to go down gracefully
				try
				{
					//	send signal
					brahms::base::IPM* ipms = engineData.pool.get(signal, remoteVoiceIndex);
					channels[remoteVoiceIndex]->push(ipms, &fout, true);
				}

				catch(brahms::error::Error& e)
				{
					fout << "ERROR in signalPeers(" << brahms::base::TranslateIPMTAG(signal) << ", " << remoteVoiceIndex << ")\n" << e.format(brahms::FMT_TEXT, true) << D_WARN;
				}

				catch(...)
				{
					fout << "ERROR in signalPeers(" << brahms::base::TranslateIPMTAG(signal) << ", " << remoteVoiceIndex << ")\n" << "(...)" << D_WARN;
				}
			}
		}



		void Comms::addRoutingEntry(UINT32 remoteVoiceIndex, UINT32 msgStreamID, brahms::channel::PushDataHandler pushDataHandler, void* pushDataHandlerArgument)
		{
			//	pass on to channel
			channels[remoteVoiceIndex]->addRoutingEntry(msgStreamID, pushDataHandler, pushDataHandlerArgument);
		}

		void Comms::stopRouting(brahms::output::Source& tout)
		{
			//	for each channel
			for (VoiceIndex remoteVoiceIndex=0; remoteVoiceIndex<channels.size(); remoteVoiceIndex++)
			{
				//	except ourselves
				if (!channels[remoteVoiceIndex]) continue;

				//	pass on
				channels[remoteVoiceIndex]->stopRouting(tout);
			}
		}

		void Comms::push(brahms::base::IPM* ipm, brahms::output::Source& tout)
		{
			//	validate
			if (ipm->getTo() >= channels.size())
			{
				UINT8 tag = ipm->header().tag;
				ferr << E_INTERNAL << "invalid remote index in push(" << brahms::base::TranslateIPMTAG(tag) << "): " << ipm->getTo() << " of " << channels.size();
			}

			//	validate
			if (!channels[ipm->getTo()])
			{
				UINT8 tag = ipm->header().tag;
				ferr << E_INTERNAL << "self-referential remote index in push(" << brahms::base::TranslateIPMTAG(tag) << "): " << ipm->getTo() << " of " << channels.size();
			}

			//	push the message
			channels[ipm->getTo()]->push(ipm, &tout);
		}

		Symbol Comms::pull(UINT32 remote, brahms::base::IPM*& ipm, UINT8 tag, brahms::output::Source& tout, UINT32 timeout, bool throwOnTimeout)
		{
			//	validate
			if (remote >= channels.size())
				ferr << E_INTERNAL << "invalid remote index in pull() (" << remote << " of " << channels.size() << ")";

			//	validate
			if (!channels[remote])
				ferr << E_INTERNAL << "self-referential remote index in pull() (" << remote << " of " << channels.size() << ")";

			//	pull the message and record the bytes
			Symbol result = channels[remote]->pull(ipm, tag, tout, timeout);
			if (result == E_COMMS_TIMEOUT && throwOnTimeout)
				ferr << E_COMMS_TIMEOUT << "waiting for expected message (" << brahms::base::TranslateIPMTAG(ipm->header().tag) + ")";

			//	ok
			return result;
		}

		inline string prettyBytes(UINT64 b)
		{
			stringstream ss;
			ss << b;
			string s = ss.str();

			int l = s.length();
			int o = 0;
			if (s.length() > 3)
			{
				o = l - 3;
				s = s.substr(0,3);
			}

			if (o==0) return s + "B";
			if (o<=2) return s.substr(0,o) + "." + s.substr(o) + "kB";
			if (o==3) return s + "kB";
			o -= 3;
			if (o<=2) return s.substr(0,o) + "." + s.substr(o) + "MB";
			if (o==3) return s + "MB";
			o -= 3;
			if (o<=2) return s.substr(0,o) + "." + s.substr(o) + "GB";
			if (o==3) return s + "GB";
			o -= 3;
			if (o<=2) return s.substr(0,o) + "." + s.substr(o) + "TB";
			if (o==3) return s + "TB";
			o -= 3;
			return s + string(o, '0') + "TB";
		}

		string Comms::getInfo()
		{
			//	collate audit data from all channels
			brahms::channel::ChannelAuditData all;

			for (VoiceIndex remoteVoiceIndex=0; remoteVoiceIndex<channels.size(); remoteVoiceIndex++)
			{
				if (!channels[remoteVoiceIndex]) continue;

				//	if audit() returns true, it's filled in the data for all channels, and we can stop
				if (!channels[remoteVoiceIndex]->audit(all))
					break;
			}

			string ret = "Q:" + brahms::text::n2s(all.send.queue) + "/" + brahms::text::n2s(all.recv.queue) + " ";
			UINT32 p;
			p = all.send.compressed ? floor ( ((DOUBLE)all.send.compressed) / ((DOUBLE)all.send.uncompressed) * 100.0 + 0.5 ) : 100;
			ret += "S: " + prettyBytes(all.send.uncompressed) + " (" + brahms::text::n2s(p) + "%) ";
			p = all.recv.compressed ? ((DOUBLE)all.recv.compressed) / ((DOUBLE)all.recv.uncompressed) * 100.0 : 100;
			ret += "R: " + prettyBytes(all.recv.uncompressed) + " (" + brahms::text::n2s(p) + "%) ";
			ret += "P: " + brahms::text::n2s(all.pool.inuse) + "/" + brahms::text::n2s(all.pool.total);

			return ret;
		}

		void Comms::flush(brahms::output::Source& tout)
		{
			tout << "Comms::flush()" << D_VERB;

			//	flush all channel send queues
			for (INT32 remoteVoiceIndex=0; remoteVoiceIndex<((INT32)channels.size()); remoteVoiceIndex++)
			{
				//	except ourselves
				if (!channels[remoteVoiceIndex]) continue;

				//	flush
				channels[remoteVoiceIndex]->flush(tout);
			}
		}

		void Comms::synchronize(brahms::output::Source& tout)
		{
			tout << "synchronizing with peers..." << D_VERB;

			//	send SYNC to all peers
			for (INT32 remoteVoiceIndex=0; remoteVoiceIndex<((INT32)channels.size()); remoteVoiceIndex++)
			{
				//	except ourselves
				if (!channels[remoteVoiceIndex]) continue;

				//	send signal
				brahms::base::IPM* ipms = engineData.pool.get(brahms::base::IPMTAG_SYNC, remoteVoiceIndex);
				channels[remoteVoiceIndex]->push(ipms, &tout);
			}

			//	recv SYNC from all peers
			for (VoiceIndex remoteVoiceIndex=0; remoteVoiceIndex<channels.size(); remoteVoiceIndex++)
			{
				//	except ourselves
				if (!channels[remoteVoiceIndex]) continue;

				//	receive message
				brahms::base::IPM* ipmr;
				while (channels[remoteVoiceIndex]->pull(ipmr, brahms::base::IPMTAG_UNDEFINED, fout, brahms::channel::COMMS_TIMEOUT_DEFAULT) != C_OK)
				{
					//	call again, until we get a message of some sort
					brahms::os::msleep(1);
				}

				//	get tag
				UINT8 tag = ipmr->header().tag;

				//	discard message
				ipmr->release();

				//	check
				if (tag != brahms::base::IPMTAG_SYNC)
					ferr << E_COMMS << "invalid sync response from peer " << unitIndex(remoteVoiceIndex) << " (" << brahms::base::TranslateIPMTAG(tag) << ")";

				//	ok
				tout << "received IPMTAG_SYNC from  voice " << unitIndex(remoteVoiceIndex) << D_FULL;
			}
		}



////////////////	NAMESPACE

	}
}

#endif // _ENGINE_MAIN_COMM_H_

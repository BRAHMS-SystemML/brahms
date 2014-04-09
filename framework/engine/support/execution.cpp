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

	$Id:: execution.cpp 2451 2010-01-25 16:48:58Z benjmitch    $
	$Rev:: 2451                                                $
	$Author:: benjmitch                                        $
	$Date:: 2010-01-25 16:48:58 +0000 (Mon, 25 Jan 2010)       $
________________________________________________________________

*/



#include "support.h"

using namespace brahms::output; // for D_INFO, etc.
using namespace brahms::xml;
using namespace brahms::math;



////////////////	NAMESPACE

namespace brahms
{



	////////////////    HELPERS

		UINT32 execFileGetUINT32(XMLNode& node, const char* child, bool allowDefault = false, UINT32 defaultValue = 0)
		{
			if (!node.hasChild(child))
			{
				if (allowDefault) return defaultValue;
				ferr << E_EXECUTION_FILE << "node \"" << child << "\" not found in Execution File";
			}

			string s = node.getChild(child)->nodeText();
			if (!s.length())
			{
				if (allowDefault) return defaultValue;
				ferr << E_EXECUTION_FILE << "node \"" << child << "\" not found in Execution File";
			}

			DOUBLE d = brahms::text::s2n(s);
			if (d == brahms::text::S2N_FAILED)
				ferr << E_EXECUTION_FILE << "unable to interpret \"" << child << "\" in Execution File";
			UINT32 u = d;
			if (d != u)
				ferr << E_EXECUTION_FILE << "expected \"" << child << "\" in Execution File to be an unsigned integer";
			return u;
		}

		VUINT32 execFileGetVUINT32(XMLNode& node, const char* child)
		{
			VUINT32 ret;

			if (!node.hasChild(child))
				return ret;

			string s = node.getChild(child)->nodeText();
			if (!s.length())
				return ret;

			//	for now, we only allow scalar seeds, here
			DOUBLE d = brahms::text::s2n(s);
			if (d == brahms::text::S2N_FAILED)
				ferr << E_EXECUTION_FILE << "unable to interpret \"" << child << "\" in Execution File";
			UINT32 u = d;
			if (d != u)
				ferr << E_EXECUTION_FILE << "expected \"" << child << "\" in Execution File to be a UINT32";
			if (u < 1 || u > 0x7FFFFFFF)
				ferr << E_EXECUTION_FILE << "expected \"" << child << "\" in Execution File to be between 1 and 0x7FFFFFFF";
			stringstream ss;
			ss << u;
			if (ss.str() != s)
				ferr << E_EXECUTION_FILE << "unable to interpret \"" << child << "\" in Execution File";

			//	plus 0x80000000, see doc "Seeding"
			ret.push_back(u + 0x80000000);
			return ret;
		}

		DOUBLE execFileGetDOUBLE(XMLNode& node, const char* child, bool allowDefault = false, UINT32 defaultValue = 0.0)
		{
			if (!node.hasChild(child))
			{
				if (allowDefault) return defaultValue;
				ferr << E_EXECUTION_FILE << "node \"" << child << "\" not found in Execution File";
			}

			string s = node.getChild(child)->nodeText();
			if (!s.length())
			{
				if (allowDefault) return defaultValue;
				ferr << E_EXECUTION_FILE << "node \"" << child << "\" not found in Execution File";
			}

			DOUBLE d = brahms::text::s2n(s);
			if (d == brahms::text::S2N_FAILED)
				ferr << E_EXECUTION_FILE << "unable to interpret \"" << child << "\" in Execution File";
			return d;
		}

		string execFileGetSTRING(XMLNode& node, const char* child, bool allowDefault = false, string defaultValue = "")
		{
			string u;

			if (!node.hasChild(child))
			{
				if (allowDefault) return defaultValue;
				ferr << E_EXECUTION_FILE << "node \"" << child << "\" not found in Execution File";
			}

			return node.getChild(child)->nodeText();
		}

		RepeatWindowSeconds translateRepeat(string s)
		{
			//	expect format to be t0:T:t1 or t0:t1 (indicate the latter by setting T=0.0)
			VSTRING ss = explode(":", s);
			if (ss.size() == 2) ss.insert(ss.begin()+1, "0.0");
			RepeatWindowSeconds repeat;
			repeat.t0 = s2n(ss[0]);
			repeat.T = s2n(ss[1]);
			repeat.t1 = s2n(ss[2]);
			if (repeat.t0 == S2N_FAILED || repeat.T == S2N_FAILED || repeat.t1 == S2N_FAILED)
				ferr << "invalid value for \"window\" (the repeat specifier \"" << s << "\" could not be interpreted)";
			if (repeat.t0 < 0.0 || repeat.T < 0.0 || repeat.t1 < 0.0)
				ferr << "invalid value for \"window\" (the repeat specifier \"" << s << "\" included negative values)";
			return repeat;
		}

		vector<LogOriginSeconds> translateWindow(const char* cwindow)
		{
			vector<LogOriginSeconds> origins;
			string swindow = cwindow;

			//	explode into origin sections
			VSTRING sorigins = explode(";", swindow);
			for(UINT32 s=0; s<sorigins.size(); s++)
			{
				//	store
				LogOriginSeconds origin;

				//	extract single origin section
				string sorigin = sorigins[s];
				while (sorigin.length() && sorigin[0] == ' ') sorigin = sorigin.substr(1);
				if (!sorigin.length()) ferr << "invalid value for \"window\" (an origin string was empty)";

				//	explode into windows
				VSTRING swindows = explode(",", sorigin);
				for (UINT32 w=0; w<swindows.size(); w++)
				{
					//	extract single window
					string swindow = swindows[w];
					while (swindow.length() && swindow[0] == ' ') swindow = swindow.substr(1);
					if (!swindow.length()) ferr << "invalid value for \"window\" (a window string was empty)";

					//	check for @ which indicates that this is the origin specified
					if (swindow[0] == '@')
					{
						if (w != 0) ferr << "invalid value for \"window\" (repeat specifier \"" << swindow << "\" must be first item in origin string)";
						swindow = swindow.substr(1);
						origin.repeat = translateRepeat(swindow);
						if (origin.repeat.T == 0.0) ferr << "invalid value for \"window\" (repeat specifier \"" << swindow << "\" should have non-zero step size)";

						//cout << "@ " << origin.repeat.t0 << " : " << origin.repeat.T << " : " << origin.repeat.t1 << endl;
					}

					//	otherwise, try to interpret as an instant or range
					else
					{
						//	apart from first (which can be global) all origin specifiers must start with a window specifier
						if (w == 0 && s != 0) ferr << "invalid value for \"window\" (first item in origin string \"" << swindow << "\" must be repeat specifier)";

						//	has colon? if not, should be an instant
						if (swindow.find(":") == string::npos)
						{
							//	should just be a number
							DOUBLE instant = s2n(swindow);
							if (instant == S2N_FAILED) ferr << "invalid value for \"window\" (unable to interpret instant specified \"" << swindow << "\")";

							//	store
							LogWindowSeconds lws = {instant, -1.0}; // -1.0 indicates instant rather than range
							origin.windows.push_back(lws);

							//cout << "  I " << lws.t0 << endl;
						}

						//	else should be a range
						else
						{
							RepeatWindowSeconds repeat = translateRepeat(swindow);
							if (repeat.T != 0.0) ferr << "invalid value for \"window\" (window specifier \"" << swindow << "\" should have zero, or unspecified, step size)";

							//	store
							LogWindowSeconds lws = {repeat.t0, repeat.t1};
							origin.windows.push_back(lws);

							//cout << "  R " << lws.t0 << " : " << lws.t1 << endl;
						}
					}
				}

				//	store
				origins.push_back(origin);
			}

			return origins;
		}



	////////////////	EXECUTION

		string protocol2string(brahms::channel::Protocol protocol)
		{
			switch(protocol)
			{
				case brahms::channel::PROTOCOL_MPI:
					return "MPI";

				case brahms::channel::PROTOCOL_SOCKETS:
					return "SOCKETS";
					
				default:
					return "<unknown protocol>";
			}
		}

		Execution::Execution()
		{
		}

		Execution::~Execution()
		{
		}

		void Execution::load(string path, VoiceCount* voiceCount, VoiceIndex* voiceIndex, brahms::output::Source& fout)
		{

			//	report
			fout << "Load Execution File \"" << path << "\"" << D_VERB;

		////////////////	PARSE XML

			ifstream fileExecution(path.c_str());
			if (!fileExecution) ferr << E_OS << "error opening \"" << path << "\"";
			try
			{
				nodeExecution.parse(fileExecution);
			}
			CATCH_TRACE_RETHROW("parsing \"" + string(path) + "\"")

			fileExecution.close();

		////////////////	VOICES

			/*

				The remote address for each remote voice is
				the best comms layer available between us and
				that voice (i.e. the best layer for which both
				we and the remote voice have an address)

			*/

			//	set/validate voice count
			XMLNode* nodeVoices = nodeExecution.getChild("Voices");
			const XMLNodeList* nodesVoice = nodeVoices->childNodes();
			UINT32 fileVoiceCount = nodesVoice->size();
			if (*voiceCount)
			{
				//	already set (by MPI) - check it tallies with that specified in file
				if (*voiceCount != fileVoiceCount)
					ferr << E_INVOCATION << "Execution File specified " << fileVoiceCount  << " voices, but " << *voiceCount << " were started by MPI";
			}
			else
			{
				//	not yet set, so use the value in the file
				*voiceCount = fileVoiceCount;
			}

			//	infer voice index (possible for solo only)
			if (*voiceIndex == VOICE_UNDEFINED && *voiceCount == 1)
			{
				*voiceIndex = 0;
				fout << "inferring voice index 1 (because voice count is 1 and voice index was unspecified)" << D_VERB;
			}

			//	we should now have a voice index, either from the command line or from MPI or from the above inference
			if (*voiceIndex == VOICE_UNDEFINED)
				ferr << E_INVOCATION << "voice index undefined (can be specified on the command line)";
			if (*voiceIndex == VOICE_FROM_MPI)
				ferr << E_INTERNAL << "voice from MPI was not set before parsing execution file";

			//	and the voice index should be in range
			if (*voiceIndex >= *voiceCount)
				ferr << E_INVOCATION << "voice index " << unitIndex(*voiceIndex) << " out of range (voice count is " << (*voiceCount) << ")";

			//	check all tags are <Voice>
			for (VoiceIndex v=0; v<(*voiceCount); v++)
			{
				if (nodesVoice->at(v)->nodeName() != string("Voice"))
					ferr << E_EXECUTION_FILE << "All children of <Voices> should be <Voice> tags (found <" << nodesVoice->at(v)->nodeName() << ">)";
			}

			//	get list of locally available comms protocols
			UINT32 protocolsAvailableLocally = 0;
			const XMLNodeList* nodesAddress = nodesVoice->at(*voiceIndex)->childNodes();
			for (UINT32 a=0; a<nodesAddress->size(); a++)
			{
				XMLNode* nodeAddress = nodesAddress->at(a);
				string sprotocol = nodeAddress->getAttribute("protocol");
				brahms::channel::Protocol protocol = brahms::channel::PROTOCOL_NULL;
				if (sprotocol == "mpi") protocol = brahms::channel::PROTOCOL_MPI;
				else if (sprotocol == "sockets") protocol = brahms::channel::PROTOCOL_SOCKETS;
				if (protocol == brahms::channel::PROTOCOL_NULL)
					ferr << E_EXECUTION_FILE << "unrecognised address protocol \"" << sprotocol << "\"";
				protocolsAvailableLocally |= (UINT32)protocol;
			}

			//	for each remote voice
			for (VoiceIndex v=0; v<(*voiceCount); v++)
			{
				//	select best protocol to remote voice
				Voice remote;

				//	get list of remotely available comms protocols
				const XMLNodeList* nodesAddress = nodesVoice->at(v)->childNodes();
				for (UINT32 a=0; a<nodesAddress->size(); a++)
				{
					XMLNode* nodeAddress = nodesAddress->at(a);
					string sprotocol = nodeAddress->getAttribute("protocol");
					brahms::channel::Protocol protocol = brahms::channel::PROTOCOL_NULL;
					if (sprotocol == "mpi") protocol = brahms::channel::PROTOCOL_MPI;
					else if (sprotocol == "sockets") protocol = brahms::channel::PROTOCOL_SOCKETS;
					if (protocol == brahms::channel::PROTOCOL_NULL)
						ferr << E_EXECUTION_FILE << "unrecognised address protocol \"" << sprotocol << "\"";

					//	if not available locally, can't use it
					if (!(protocolsAvailableLocally & protocol))
						continue;

					//	if better than currently selected, use it
					if (
						remote.protocol == brahms::channel::PROTOCOL_NULL /* nothing yet selected */
						||
						remote.protocol > protocol /* better than currently selected */
						)
					{
						remote.protocol = protocol;
						remote.address = nodeAddress->nodeText();
					}
				}

				//	check we got something
				if (remote.protocol == brahms::channel::PROTOCOL_NULL)
				{
					if (v != *voiceIndex)
						ferr << E_EXECUTION_FILE << "no communications route to voice " << unitIndex(v);
					else
					{
						//	it's ok to have no route to ourself!
					}
				}
				else
				{
					fout << "channel to voice " << unitIndex(v) << ": " << protocol2string(remote.protocol) << ":" << remote.address << D_VERB;
				}

				//	store
				voices.push_back(remote);
			}



	////////////////	<WorkingDirectory>

			//	working directory is the directory in which the Execution File is
			//	found, unless specified explicitly otherwise in Execution File
			workingDirectory = nodeExecution.getChild("WorkingDirectory")->nodeText();
			if (!workingDirectory.length())
			{
				string wd = path;
				brahms::text::grep(wd, "\\", "/");
				size_t p = wd.rfind("/");
				if (p == string::npos)
				{
					//	otherwise, use current directory, since this must be where the Execution File is
					workingDirectory = brahms::os::getcurdir();
				}
				else workingDirectory = path.substr(0, p);
			}

			//	other files are in working directory unless explicitly specified
			fileSysIn = nodeExecution.getChild("SystemFileIn")->nodeText();
			if (fileSysIn.find("/") == string::npos && fileSysIn.find("\\") == string::npos)
				fileSysIn = workingDirectory + "/" + fileSysIn;
			fileSysOut = nodeExecution.getChild("SystemFileOut")->nodeText();
			if (fileSysOut.length() && fileSysOut.find("/") == string::npos && fileSysOut.find("\\") == string::npos)
				fileSysOut = workingDirectory + "/" + fileSysOut;
			fileReport = nodeExecution.getChild("ReportFile")->nodeText();
			if (fileReport.find("/") == string::npos && fileReport.find("\\") == string::npos)
				fileReport = workingDirectory + "/" + fileReport;

			//	fix ((VOICE)) token in report file
			ostringstream idxString;
			idxString << unitIndex(*voiceIndex);
			brahms::text::grep(fileReport, "((VOICE))", idxString.str());



	////////////////	SIMPLE PARAMETERS

			//	get simple parameters
			name = execFileGetSTRING(nodeExecution, "Name", true, "");
			executionStop = execFileGetDOUBLE(nodeExecution, "ExecutionStop");
			timing = execFileGetUINT32(nodeExecution, "Timing", true, 0);
			seed = execFileGetVUINT32(nodeExecution, "Seed");



	////////////////	<Affinity>

			//	get Affinity
			XMLNode* nodeAffinity = nodeExecution.getChild("Affinity");
			const XMLNodeList* nodesGroup = nodeAffinity->childNodes();

			//	collect groups
			for (UINT32 n=0; n<nodesGroup->size(); n++)
			{
				XMLNode* nodeGroup = nodesGroup->at(n);
				const XMLNodeList* nodesIdentifier = nodeGroup->childNodes();

				Group grp;
				if (nodeGroup->hasAttribute("Voice"))
				{
					const char* voice = nodeGroup->getAttribute("Voice");
					DOUBLE dvoice = brahms::text::s2n(voice);
					grp.voice = dvoice;
					if (
						dvoice == brahms::text::S2N_FAILED ||
						grp.voice != dvoice ||
						grp.voice < 1 ||
						grp.voice > (*voiceCount)
					)
						ferr << E_EXECUTION_FILE << "malformed or invalid Affinity tag (\"Voice\" attribute should be between 1 and " << *voiceCount << ", it was \"" << voice << "\")";
					grp.voice--; // go to zero-based
				}

				for (UINT32 n=0; n<nodesIdentifier->size(); n++)
					grp.ident.push_back(nodesIdentifier->at(n)->nodeText());

				groups.push_back(grp);
			}



	////////////////	<Logs>

			//	list output names that were requested
			if (nodeExecution.hasChild("Logs"))
			{
				//	get attribute All
				XMLNode* nodeLogs = nodeExecution.getChild("Logs");
				if (nodeLogs->hasAttribute("All"))
				{
					//	do store, but at not set precision
					if (nodeLogs->getAttribute("All") == string("1"))
						defaultLogMode.precision = PRECISION_NOT_SET;
				}

				//	get attribute Precision
				INT32 defaultPrecision = PRECISION_NOT_SET;
				if (nodeLogs->hasAttribute("Precision"))
				{
					//	cannot act as the default precision, since it also acts as "store or not store", so we have to have a separate variable
					defaultPrecision = (INT32) atof(nodeLogs->getAttribute("Precision"));
					if (defaultLogMode.precision != PRECISION_DO_NOT_LOG)
						defaultLogMode.precision = defaultPrecision;
				}

				//	get attribute encapsulated
				if (nodeLogs->hasAttribute("Encapsulated"))
				{
					//	also acts as the default encapsulated
					defaultLogMode.encapsulated = (bool) atof(nodeLogs->getAttribute("Encapsulated"));
				}

				//	get attribute recurse
				if (nodeLogs->hasAttribute("Recurse"))
				{
					//	also acts as the default recurse
					defaultLogMode.recurse = (bool) atof(nodeLogs->getAttribute("Recurse"));
				}

				//	get attribute window
				if (nodeLogs->hasAttribute("Window"))
				{
					//	also acts as the default window
					defaultLogMode.origins = translateWindow(nodeLogs->getAttribute("Window"));
				}

				//	get explicit outputs
				const XMLNodeList* list = nodeLogs->childNodes();
				for (UINT32 o=0; o<list->size(); o++)
				{
					XMLNode* nodeLog = list->at(o);
					INT32 precision = defaultPrecision;
					if (nodeLog->hasAttribute("Precision"))
						precision = (INT32) atof(nodeLog->getAttribute("Precision"));
					bool encapsulated = defaultLogMode.encapsulated;
					if (nodeLog->hasAttribute("Encapsulated"))
						encapsulated = (bool) atof(nodeLog->getAttribute("Encapsulated"));
					vector<LogOriginSeconds> origins = defaultLogMode.origins;
					if (nodeLog->hasAttribute("Window"))
						origins = translateWindow(nodeLog->getAttribute("Window"));
					bool recurse = defaultLogMode.recurse;
					if (nodeLog->hasAttribute("Recurse"))
						recurse = (bool) atof(nodeLog->getAttribute("Recurse"));
					logRules.push_back(LogRule(nodeLog->nodeText(), precision, encapsulated, recurse, origins));
				}
			}
		}

		XMLNode* Execution::prepareReportFile(const char* ExecutionID, const char* VoiceID, INT32 voiceIndex, INT32 voiceCount)
		{
			//	---- ROOT NODE ----

			nodeReport.nodeName("Report");
			nodeReport.setAttribute("Version", "1.0");

			//	---- SKELETON (get things in the right order) ----

			XMLNode* nodeHeader = nodeReport.appendChild(new XMLNode("Header")); // public (defined in SystemML Standard)
			XMLNode* nodeEnvironment = nodeReport.appendChild(new XMLNode("Environment")); // public (defined in SystemML Standard)
			XMLNode* nodeLogs = nodeReport.appendChild(new XMLNode("Logs")); // public (defined in SystemML Standard)

			XMLNode* nodeClient = nodeEnvironment->appendChild(new XMLNode("Client")); // private to client

			//	---- CLIENT ----

			XMLNode* nodeClientID = nodeEnvironment->appendChild(new XMLNode("ClientID"));
			nodeClientID->appendChild(new XMLNode("Name", "BRAHMS"));
			nodeClientID->appendChild(new XMLNode("Version", toString(VERSION_ENGINE).c_str()));
			nodeClientID->appendChild(new XMLNode("BitWidth", n2s(ARCH_BITS).c_str()));

			//	---- HEADER ----

			/*XMLNode* nodeExecutionID = */nodeHeader->appendChild(new XMLNode("ExecutionID", ExecutionID));
//			XMLNode* nodeVoiceID = nodeHeader->appendChild(new XMLNode("VoiceID", VoiceID));

			//	instance data
			stringstream ss;
			ss << (voiceIndex + 1);
			string s = ss.str();
			nodeHeader->appendChild(new XMLNode("PartIndex", s.c_str()));
			ss.str("");
			ss << voiceCount;
			s = ss.str();
			nodeHeader->appendChild(new XMLNode("PartCount", s.c_str()));

			//	---- EXECUTION COPY ----

			//	add copy of Execution to Report
			nodeClient->appendChild(new XMLNode(nodeExecution));

			//	---- OUTPUT LOGS ----

			return nodeLogs;
		}



////////////////	NAMESPACE

}



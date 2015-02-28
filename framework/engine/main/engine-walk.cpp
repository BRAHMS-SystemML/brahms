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

$Id:: engine-walk.cpp 2406 2009-11-19 20:53:19Z benjmitch  $
$Rev:: 2406                                                $
$Author:: benjmitch                                        $
$Date:: 2009-11-19 20:53:19 +0000 (Thu, 19 Nov 2009)       $
________________________________________________________________

*/

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "support/xml.h"
#include "support/os.h"
#include "base/brahms_error.h"
#include "base/constants.h"
#include "base/output.h"
using namespace brahms::output;
#include "main/engine.h"
using brahms::Engine;
#include "base/text.h"
using brahms::text::n2s;

struct KeyValue
{
	string key;
	string value;
};

vector<KeyValue> breakUpAdditionalInfo(const char* c_info)
{
	vector<KeyValue> kvs;
	if (c_info)
	{
		string info(c_info);
		while(info.length())
		{
			size_t p_newline = info.find("\n");
			if (p_newline == string::npos) ferr << E_NOT_COMPLIANT << "malformed additional info string (no newline)";
			string keyval = info.substr(0, p_newline);
			info = info.substr(p_newline + 1);
			size_t p_equals = keyval.find("=");
			if (p_equals == string::npos) ferr << E_NOT_COMPLIANT << "malformed additional info string (no equals)";
			KeyValue kv;
			kv.key = keyval.substr(0, p_equals);
			kv.value = keyval.substr(p_equals + 1);
			if (!kv.key.length()) ferr << E_NOT_COMPLIANT << "malformed additional info string (zero-length key)";
			kvs.push_back(kv);
		}
	}
	return kvs;
}

#define WALK_OUT cout

void Engine::walksub(WalkLevel level, const string& namespaceRootPath, string subpath, string indent)
{
	if (!subpath.length()) WALK_OUT << "  searching \"" << namespaceRootPath << "\"..." << endl;

	//	assume it's a branch unless we can read node.xml
	ObjectType type = CT_NULL;
	string filenameNode = namespaceRootPath + subpath + "/node.xml";

	//	if present
	if (brahms::os::fileexists(filenameNode))
	{
		//	load node file
		ifstream fileNode(filenameNode.c_str());
		if (!fileNode) ferr << E_NAMESPACE << "error opening \"" << filenameNode << "\"";
		brahms::xml::XMLNode nodeNode(fileNode);
		fileNode.close();

		string nodeTypeString = nodeNode.getChild("Type")->nodeText();
		if (nodeTypeString == "Leaf")
		{
			type = CT_PROCESS;
			fout << "node file at \"" << filenameNode << "\" should read <Type>Process</Type> (new syntax)" << D_VERB;
		}
		else if (nodeTypeString == "Process")
			type = CT_PROCESS;
		else if (nodeTypeString == "Data")
			type = CT_DATA;
		else if (nodeTypeString == "Utility")
			type = CT_UTILITY;
		else ferr << E_NAMESPACE << "unrecognised node type string in node file \"" << nodeTypeString << "\"";
	}

	//	if it's a leaf, we can check for a module
	if (type != CT_NULL)
	{
		//	can't be at root
		if (!subpath.length()) ferr << E_NAMESPACE << "root of tree cannot be a leaf";

		//	fout
		string className = subpath.substr(1);
		brahms::text::grep(className, "\\", "/");
		string indentx = indent + "  * ";

		//	get list of modules here
//		vector<WalkedModule> modules = getModules(namespaceRootPath + subpath);

		//	for each module
		for (UINT32 release=0; ; release++)
		{
			//	check if exists
			string releaseFolder = namespaceRootPath + subpath + "/brahms/" + n2s(release);
			if (!brahms::os::fileexists(releaseFolder)) break;

			//	load implementation file
			string filename = releaseFolder + "/release.xml";
			if (!brahms::os::fileexists(filename))
			{
				fout << "no release.xml file found at \"" << filename << "\"" << D_WARN;
				continue;
			}

			//	load it
			brahms::xml::XMLNode releaseFile;
			ifstream file(filename.c_str());
			file.seekg(0);
			if (!file)
				ferr << E_NAMESPACE << "release file not readable \"" << filename << "\"";
			releaseFile.parse(file);
			file.close();

			//	extract
			string language = releaseFile.getChild("Language")->nodeText();

			//	component version
			ComponentVersion v;
			v.release = release;
			v.revision = 0xFFFF;

			//	additional information
			brahms::error::Error err;
			string xinfo;

			//	check mode
			bool deep = level == WL_LOAD_ALL;
			if (level == WL_LOAD_NATIVE && (language == "1065" || language == "1199" || language == "1266"))
				deep = true;

			//	load module
			if (deep)
			{
				//	catch errors to display module that failed
				try
				{
					//	load component
					brahms::systemml::Component* component = NULL;
					string name = "walk_instance";
					SampleRate sampleRate = {1, 1};
					VUINT32 seed; // empty

					switch(type)
					{
						case CT_PROCESS:
						{
							//	create
							component = new brahms::systemml::Process(
								name.c_str(),
								engineData,
								&fout,
								className,
								sampleRate,
								NULL,
								seed,
								0
							);

							//	ok
							break;
						}

						case CT_DATA:
						{
							//	create
							component = new brahms::systemml::Data(
								name.c_str(),
								engineData,
								&fout,
								className,
								0,
								sampleRate
							);

							//	ok
							break;
						}

						case CT_UTILITY:
						{
							//	create
							component = new brahms::systemml::Utility(
								name.c_str(),
								engineData,
								&fout,
								className,
								NULL
							);

							//	ok
							break;
						}

						default:
						{
							ferr << E_INTERNAL;
						}
					}

					//	load
					component->load(fout);

					//	instantiate
					ObjectType type = component->instantiate(&fout);

					//	examine info
					const ComponentInfo* info = component->getComponentInfo();
					const ModuleInfo* minfo = component->module->getInfo();
					if (type == CT_DATA) xinfo += indentx + "is a data component\n";
					if (type == CT_UTILITY) xinfo += indentx + "is a utility component\n";
					if (info->componentVersion) v.revision = info->componentVersion->revision;
					else WALK_OUT << "ERROR: component did not return its version" << endl;
					if (minfo->engineVersion) xinfo += indentx + "built against engine version " + brahms::text::toString(*minfo->engineVersion) + "\n";
					else WALK_OUT << "ERROR: component did not return its engine version" << endl;

					//	additional info
					vector<KeyValue> additional = breakUpAdditionalInfo(info->additional);
					for (UINT32 e=0; e<additional.size(); e++)
						xinfo += indentx + additional[e].key + ": " + additional[e].value + "\n";

					//	external libraries
					additional = breakUpAdditionalInfo(info->libraries);
					for (UINT32 e=0; e<additional.size(); e++)
						xinfo += indentx + additional[e].key + ": " + additional[e].value + "\n";

					//	destroy
					component->destroy(&fout);

					//	delete component
					delete component;
				}

				catch(brahms::error::Error& e)
				{
					err = e;
				}
			}

			//	display
			WALK_OUT << indent << className << "-" << release << " (L" << language << ", R" << (v.revision == 0xFFFF ? string("?") : n2s(v.revision)) << ")" << endl;
			WALK_OUT << xinfo;

			if (err.code != S_NULL)
			{
				WALK_OUT << "________________________________________________________________" << endl << endl;
				WALK_OUT << err.format(brahms::FMT_TEXT, true) << endl;
				WALK_OUT << "________________________________________________________________" << endl << endl;

			}
		}

	}

	//	if not, we can recurse into any folders we find
	else
	{
		//	fout (except at root)
		if (subpath.length())
		{
			string nodeName = subpath.substr(1);
			brahms::text::grep(nodeName, "\\", "/");
			WALK_OUT << indent << "(" << nodeName << ")"  << endl;
		}

		//	collect names of all sub-folders
		vector<string> subs;
		brahms::os::Directory dir(namespaceRootPath + subpath);
		while(true)
		{
			string filename = dir.getNextFile();
			if (!filename.length()) break;
			if(dir.wasDirectory()) subs.push_back(filename);
		}
		dir.close();

		//	recurse them
		for (UINT32 s=0; s<subs.size(); s++)
			walksub(level, namespaceRootPath, subpath + "/" + subs[s], indent + "  ");
	}
}

void Engine::walk(WalkLevel level)
{
	//	for each namespace root path
	VSTRING namespaceRoots = engineData.loader.namespaceRoots;
	for (UINT32 n=0; n<namespaceRoots.size(); n++)
		walksub(level, namespaceRoots[n], "", "");
}

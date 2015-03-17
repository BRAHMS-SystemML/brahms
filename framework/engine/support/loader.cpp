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

	$Id:: loader.cpp 2410 2009-11-20 10:18:18Z benjmitch       $
	$Rev:: 2410                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-20 10:18:18 +0000 (Fri, 20 Nov 2009)       $
________________________________________________________________

*/



#include "support.h"

using namespace brahms::module;
using namespace brahms::text;   // grep
using namespace brahms::output; // D_WARN



////////////////	NAMESPACE

namespace brahms
{


////////////////	LOADER

	Loader::Loader()
	{
	}

	Loader::~Loader()
	{
	}

	void Loader::init(const brahms::Environment& environment)
	{
		//	The bindings are accessed via the INSTALL_PREFIX.
		installPrefix = INSTALL_PREFIX;

		//	get path from Execution Parameters
		string sNamespaceRoots = environment.gets("NamespaceRoots");

		//	break out separate paths
		VSTRING roots = brahms::text::explode(brahms::os::ENV_SEPARATOR, sNamespaceRoots);

		//	trim any whitespace from them
		for (UINT32 i=0; i<roots.size(); i++)
		{
			string root = roots[i];
			brahms::text::trim(root);
			if (root.length())
				namespaceRoots.push_back(root);
		}

		//	check we got some
		if (!namespaceRoots.size())
			ferr << E_EXECUTION_PARAMETERS << "malformed execution parameter NamespaceRoots (no entries)";
	}

	Module* Loader::loadComponent(ComponentCreateS& componentCreateS, brahms::output::Source& fout, ExecutionInfo* systemInfo, UINT16 release, UINT32 language, INT32 rootPathIndex)
	{
		//	validate
		if (release != 0)
			ferr << E_SYSTEM_FILE << "attempt to load \"" << componentCreateS.componentClass << "\" release " << release << ": currently only release zero is supported";

		//	get release folder form
		string releaseFolderForm = brahms::os::COMPONENT_RELEASE_FOLDER_FORM;
		grep(releaseFolderForm, "((RELEASE))", "0");

		//	for each namespace root path
		for (UINT32 n=0; n<namespaceRoots.size(); n++)
		{
			//	if rootPathIndex specified, only allow that one
			if (rootPathIndex != -1 && n != ((UINT32)rootPathIndex))
				continue;

			//	path to node folder
			string nodeFolder = namespaceRoots[n] + "/" + componentCreateS.componentClass;

			//	path to release folder
			string releaseFolder = nodeFolder + "/" + releaseFolderForm;

			//	other data
			brahms::xml::XMLNode releaseFile;
			componentCreateS.moduleFilename = "component";

			//	HANDLE LEGACY FOLDER FORM
			bool seeking_legacy = false;
			int lang = 0;
			if (!brahms::os::fileexists(releaseFolder))
			{
				if (brahms::os::fileexists(nodeFolder + "/brahms/imp/1199/0"))
				{
					releaseFolderForm = "brahms/imp/1199/0";
					releaseFolder = nodeFolder + "/" + releaseFolderForm;
					seeking_legacy = true;
					lang = 1199;
					string us = componentCreateS.componentClass;
					grep(us, "/", "_");
					componentCreateS.moduleFilename = brahms::os::COMPONENT_MODULE_FORM_OLD;
					grep(componentCreateS.moduleFilename, "((CLASS_US))", us);
				}

				else if (brahms::os::fileexists(nodeFolder + "/brahms/imp/1258/0"))
				{
					releaseFolderForm = "brahms/imp/1258/0";
					releaseFolder = nodeFolder + "/" + releaseFolderForm;
					seeking_legacy = true;
					lang = 1258;
					componentCreateS.moduleFilename = "brahms_process";
				}

				else if (brahms::os::fileexists(nodeFolder + "/brahms/imp/1262/0"))
				{
					releaseFolderForm = "brahms/imp/1262/0";
					releaseFolder = nodeFolder + "/" + releaseFolderForm;
					seeking_legacy = true;
					lang = 1262;
					componentCreateS.moduleFilename = "brahms_process";
				}
			}

			//	if exists
			if (brahms::os::fileexists(releaseFolder))
			{
				if (seeking_legacy)
				{
					____WARN("Namespace layout from version 0.7.2 is used at:\n\t\"" << releaseFolder << "\"\n\tsee RELEASE NOTES for information on updating to the new layout\n\t(legacy support will be removed in a future release)");
				}

				//	load release file (allow absent if finding legacy release)
				string filename = releaseFolder + "/release.xml";
				if (brahms::os::fileexists(filename))
				{
					//	load it
					ifstream file(filename.c_str());
					file.seekg(0);
					if (!file)
						ferr << E_NAMESPACE << "release file not readable \"" << filename << "\"";
					releaseFile.parse(file);
					file.close();

					//	find language
					string language = releaseFile.getChild("Language")->nodeText();
					if (language == "1065") lang = 1065;
					if (language == "1199") lang = 1199;
					if (language == "1258") lang = 1258;
					if (language == "1262") lang = 1262;
					if (language == "1266") lang = 1266;
				}

				else
				{
					if (seeking_legacy)
					{
						//	lang was set above
					}

					else
					{
						fout << "no release.xml file found at \"" << filename << "\"" << D_WARN;
						continue;
					}
				}

				//	check
				if (!lang)
					ferr << E_NAMESPACE << "unrecognised Language in release file \"" << filename << "\"";

				//	supply module paths
				ModulePaths modulePaths;
				modulePaths.root = namespaceRoots[n];
				modulePaths.node = nodeFolder;
				modulePaths.release = releaseFolder;

				//	return component info
				componentCreateS.namespaceRootPath = namespaceRoots[n];
				componentCreateS.releasePath = releaseFolderForm;

				//	switch language
				switch(lang)
				{
					case 1065:
					case 1199:
					case 1266:
					{
						if (releaseFile.hasChild("Filename"))
							componentCreateS.moduleFilename = releaseFile.getChild("Filename")->nodeText();
						filename = releaseFolder + "/" + componentCreateS.moduleFilename;
						if (!seeking_legacy) filename += string(".") + brahms::os::COMPONENT_DLL_EXT;

						componentCreateS.isBinding = false; // don't send extra info to EVENT_MODULE_CREATE

						return loadModule(filename, fout, systemInfo, &modulePaths,
							(seeking_legacy ? F_NO_QUERY_VALIDATE : 0));
					}

					case 1258:
					{
						if (releaseFile.hasChild("Filename"))
							componentCreateS.moduleFilename = releaseFile.getChild("Filename")->nodeText();

						string bindingFilename = installPrefix + brahms::os::M_BINDING_MODULE_FORM;

						componentCreateS.isBinding = true; // do send extra info to EVENT_MODULE_CREATE

						return loadModule(bindingFilename, fout, systemInfo, &modulePaths,
							(seeking_legacy ? F_NO_QUERY_VALIDATE : 0));
					}

					case 1262:
					{
						if (releaseFile.hasChild("Filename"))
							componentCreateS.moduleFilename = releaseFile.getChild("Filename")->nodeText();

						string bindingFilename = installPrefix + brahms::os::PY_BINDING_MODULE_FORM;

						componentCreateS.isBinding = true; // do send extra info to EVENT_MODULE_CREATE

						return loadModule(bindingFilename, fout, systemInfo, &modulePaths,
							F_USE_RTLD_GLOBAL | (seeking_legacy ? F_NO_QUERY_VALIDATE : 0)); // need this, or python module aborts at run-time - don't know why
					}
				}
			}
		}

		//	not found
		ferr << E_NAMESPACE <<
			"no BRAHMS implementations of SystemML class " << componentCreateS.componentClass << " are available";

		//	avoid warning...
		return 0;
	}

	Module* Loader::loadModule(string path, brahms::output::Source& fout, ExecutionInfo* systemInfo, const ModulePaths* modulePaths, UINT32 flags)
	{
		//	if already loaded, return it directly
		for (UINT32 m=0; m<modules.size(); m++)
			if (modules[m]->getPath() == path) return modules[m];

		//	handle NULL
		ModulePaths localModulePaths; // empty
		if (!modulePaths)
			modulePaths = &localModulePaths;

		//	check it exists
		if (!brahms::os::fileexists(path))
			ferr << E_MODULE_NOT_FOUND << "module not found \"" << path << "\"";

		//	have the os load the module
		Module* module = new Module(flags);
		if (!module->load(path, *modulePaths, systemInfo))
		{
			//	delete object
			delete module;

			//	construct error
			brahms::error::Error e(E_FAILED_LOAD_MODULE, path);

			//	add more information if possible
			UINT32 ierr;
			string serr = brahms::os::getlasterror(&ierr);

		#ifdef __WIN__
			if (ierr == 126)
				e.trace("**** Windows Error 126 **** - see documentation for E_FAILED_LOAD_MODULE for probable cause");
		#endif

			serr = "OS last error \"" + serr + "\"";
			e.trace(serr.c_str());

			//	raise error
			throw e;
		}

		//	ok, add it to the stack
		modules.push_back(module);

		//	report
		//	no, because we might be loading the MPI module (to get the rank) before the log is open
//		fout << "loaded module \"" << path << "\"" << brahms::output::D_VERB;

		//	return the index of the new one
		return module;
	}

	void Loader::terminate(brahms::thread::ThreadBase& thread)
	{
		for (UINT32 m=0; m<modules.size(); m++)
		{
			//	if loaded
			if (modules[m])
			{
				//	terminate
				try
				{
					modules[m]->terminate();
				}
				catch(brahms::error::Error e)
				{
					thread.storeError(e, thread.tout);
				}

				//	delete
				delete modules[m];
				modules[m] = NULL;
			}
		}

	}



////////////////	NAMESPACE

}

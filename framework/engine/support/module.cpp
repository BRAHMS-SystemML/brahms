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

	$Id:: module.cpp 2410 2009-11-20 10:18:18Z benjmitch       $
	$Rev:: 2410                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-20 10:18:18 +0000 (Fri, 20 Nov 2009)       $
________________________________________________________________

*/



#include "support.h"

using namespace brahms::xml;


#ifdef __NIX__
#include <dlfcn.h>
#endif


////////////////	NAMESPACE

namespace brahms
{
	namespace module
	{



	////////////////	MODULE

		Module::Module(UINT32 p_flags)
		{
			//	module fields
			hModule = NULL;
			flags = p_flags;

			//	component module fields
			eventHandler = NULL;
			moduleInfo = NULL;
		}

		Module::Module(const Module& src)
		{
			/*

				Copying this object would require us, at least, to
				increment the reference count of the OS module that
				is associated. However, we would not want to call
				EVENT_MODULE_INIT again! In any case, we avoid the
				issue by just not doing it.

			*/

			ferr << E_INTERNAL << "Module copy constructor called - see notes at " << __FILE__ << ":" << __LINE__;
		}

		Module::~Module()
		{
			//	unload module if loaded
			if (hModule)
			{
			#ifdef __WIN__

				//	FreeLibrary returns non-zero on success
				if (!FreeLibrary((HMODULE)hModule))
					____WARN("failed to unload module \"" << path << "\"");

			#endif

			#ifdef __NIX__

				//	dlclose returns zero on success (reverse logic)
				if (dlclose(hModule))
					____WARN("failed to unload module \"" << path << "\"");

			#endif
			}
		}

		bool Module::load(string path, const ModulePaths& p_modulePaths, const ExecutionInfo* systemInfo)
		{
			//	sanity check
			if (this->path.length())
				ferr << E_INTERNAL << "path already set (\"" << this->path << "\") on Module::load()";

			//	store path
			this->path = path;

		#ifdef __WIN__

			//	LoadLibrary returns a handle to the newly loaded module
			hModule = LoadLibrary(path.c_str());

		#endif

		#ifdef __NIX__

		#ifdef __GLN__

			//	dlopen returns a handle to the newly loaded module
			//
			//	why exceptions won't get caught across DSO boundaries? search gnu.g++.help
			//	for "exceptions across DSO boundary - unrecognised when caught"
			//
			//	given the above, we don't use RTLD_DEEPBIND. there seems no reason to
			//	in any case.
			//
			//	RTLD_GLOBAL: The symbols defined by this library will be made
			//		available for symbol resolution of subsequently loaded libraries. 
			//
			//	oh. ok, i had to put it back in a few Rs later, because python
			//	bindings don't work without it. presumably there is some implicit
			//	symbol linking going on (and it appears to be JIT linking because
			//	it doesn't complain until run-time). i don't like this solution
			//	much - in fact, i'll turn it on only for python, and see how that
			//	works.
			
			if (flags & F_USE_RTLD_GLOBAL)
			{
				hModule = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
			}
			else
			{
				hModule = dlopen(path.c_str(), RTLD_NOW);
			}

		#else

			//	dlopen returns a handle to the newly loaded module
			hModule = dlopen(path.c_str(), RTLD_NOW); // think this other flag is not needed? | RTLD_GLOBAL);

		#endif

            /*
			//	store last error
			if (!hModule)
			{
				char* le = dlerror();
				if (le) fout << "dlopen() failed with message \"" << le << "\"" << brahms::output::D_WARN;
			}
			*/
			
		#endif

			//	if failed load, give up now
			if (!hModule) return false;

			//	if component module, namespaceRootPath will have been supplied
			if (p_modulePaths.root.length())
			{
				//	store
				modulePaths = p_modulePaths;

				//	get pointer to moduleEventFunction
				eventHandler = (EventHandlerFunction*) map("EventHandler");
				if (!eventHandler)
					ferr << E_NOT_COMPLIANT << "module \"" << path << "\" does not export \"EventHandler\"";

				//	fire EVENT_MODULE_INIT
				eventINIT(systemInfo);

				//	load nodefile
				parseNodeFile((modulePaths.node + "/node.xml").c_str());

				//	look for connectivity
				XMLNode* node = &nodeFile;
				node = node->getChildOrNull("Specification");
				if (node)
				{
					node = node->getChildOrNull("Connectivity");
					if (node)
					{
						XMLNode* snode = node->getChildOrNull("InputSets");
						if (snode)
						{
							const XMLNodeList* kids = snode->childNodes();
							for (UINT32 c=0; c<kids->size(); c++)
								inputSets.push_back(kids->at(c)->nodeText());
						}
						snode = node->getChildOrNull("OutputSets");
						if (snode)
						{
							const XMLNodeList* kids = snode->childNodes();
							for (UINT32 c=0; c<kids->size(); c++)
								outputSets.push_back(kids->at(c)->nodeText());
						}
					}
				}
			}

			//	ok
			return true;
		}

		void Module::parseNodeFile(const char* filename)
		{
			//	load nodefile
			if (!brahms::os::fileexists(filename))
				ferr << E_NAMESPACE << "node file not found \"" << filename << "\"";
			ifstream file(filename);
			file.seekg(0);
			if (!file)
				ferr << E_NAMESPACE << "node file not readable \"" << filename << "\"";
			nodeFile.parse(file);
			file.close();
		}

		void* Module::map(string name)
		{

		#ifdef __WIN__

			//	GetProcAddress returns the address of the function
			return GetProcAddress((HMODULE)hModule, name.c_str());

		#endif

		#ifdef __NIX__

			//	dlsym returns the address of the function
			return dlsym(hModule, name.c_str());

		#endif

		}

		Symbol Module::eventINIT(const ExecutionInfo* systemInfo)
		{
			//	must be a component module
			if (!eventHandler)
				ferr << E_INTERNAL << "Module::fireEvent() on non-component module";

			//	query interface first
			if (!(flags & F_NO_QUERY_VALIDATE))
			{
				//	we don't query legacy modules (to ease the transition to 0.7.3)
				EventModuleQuery query;
				query.interfaceID = 0;
				Event eventQuery;
				eventQuery.type = EVENT_MODULE_QUERY;
				eventQuery.flags = 0;
				eventQuery.object = NULL;
				eventQuery.data = &query;
				Symbol result = eventHandler(&eventQuery);
				if (S_ERROR(result)) throw result;
				if (query.interfaceID != N_BRAHMS_INTERFACE)
					ferr << E_NOT_COMPLIANT << "module \"" << path << "\" is not a BRAHMS module";
			}

			//	event data
			EventModuleInit data;
			____CLEAR(data);
			data.executionInfo = systemInfo;

			//	event
			Event event;
			event.type = EVENT_MODULE_INIT;
			event.flags = 0;
			event.object = NULL;
			event.data = &data;

			//	fire
			Symbol result = eventHandler(&event);
			if (S_ERROR(result)) throw result;

			//	check info is valid here, to avoid seg faults later
			if (!data.moduleInfo) ferr << E_NOT_COMPLIANT << "\"" << path << "\" did not return ModuleInfo";
			if (!data.moduleInfo->engineVersion) ferr << E_NOT_COMPLIANT << "\"" << path << "\" did not set \"engineVersion\" in ModuleInfo";
			if (data.moduleInfo->archBits != 32 && data.moduleInfo->archBits != 64) ferr << E_NOT_COMPLIANT << "\"" << path << "\" did not return a valid bit-width in ModuleInfo";

			//	must be a recognised language (this allows us to handle interface changes)
			switch(data.moduleInfo->binding)
			{
				case 1065:
				case 1199:
				case 1258:
				case 1262:
				case 1266:
					break;

				default:
					ferr << E_NOT_COMPLIANT << "\"" << path << "\" returned an unrecognised value for \"binding\"";
			}

			//	store info in module object
			moduleInfo = data.moduleInfo;

			//	ok
			return result;
		}

		Symbol Module::terminate()
		{
			//	if it's got a NULL eventHandler, it's not a component module
			if (!eventHandler) return C_OK;

			//	event data
			EventModuleTerm data;
			____CLEAR(data);

			//	event
			Event event;
			event.type = EVENT_MODULE_TERM;
			event.flags = 0;
			event.object = NULL;
			event.data = &data;

			//	fire
			Symbol result = eventHandler(&event);

			//	handle error (don't throw - we're closing down anyway so there's no point and we might cause trouble elsewhere!)
			if (S_ERROR(result))
			{
				brahms::error::Error e = brahms::error::globalErrorRegister.pull(result);
				e.trace("whilst firing EVENT_MODULE_TERM on module \"" + path + "\"");
				throw e;
			}

			//	ok
			return result;
		}

		const string& Module::getPath()
		{
			return path;
		}

		const ModuleInfo* Module::getInfo()
		{
			return moduleInfo;
		}

		EventHandlerFunction* Module::getHandler()
		{
			return eventHandler;
		}

		const string& Module::getNamespaceRootPath()
		{
			return modulePaths.root;
		}



////////////////	NAMESPACE

	}
}



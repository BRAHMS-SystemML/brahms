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

	$Id:: loader.h 2410 2009-11-20 10:18:18Z benjmitch         $
	$Rev:: 2410                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-20 10:18:18 +0000 (Fri, 20 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_ENGINE_LOADER
#define INCLUDED_ENGINE_LOADER




////////////////	NAMESPACE

namespace brahms
{



////////////////	LOADER

	struct ComponentCreateS
	{
		string namespaceRootPath;
		string componentClass;
		string releasePath;
		string moduleFilename;
		bool isBinding;
	};

	class Loader
	{

	public:

		Loader();
		~Loader();

		//	when execution parameters are finalised
		void init(const brahms::Environment& environment);

		//	return Module found in one of the Namespace roots
		brahms::module::Module* loadComponent(ComponentCreateS& componentCreateS, brahms::output::Source& fout, ExecutionInfo* systemInfo, UINT16 release, UINT32 language = LANGUAGE_UNSPECIFIED, INT32 rootPathIndex = -1);

		//	return index of existing or newly-loaded module
		//	and, if component is true, map the handler and
		//	fire EVENT_MODULE_INIT to get the ModuleInfo
		brahms::module::Module* loadModule(string path, brahms::output::Source& fout, ExecutionInfo* systemInfo, const brahms::module::ModulePaths* modulePaths, UINT32 flags);

		//	unload all modules
		void terminate(brahms::thread::ThreadBase& thread);

		//	other data
		string systemMLInstallPath;
		VSTRING namespaceRoots;

		//	list of modules in-memory
		vector<brahms::module::Module*> modules;
	};


}



////////////////	INCLUSION GUARD

#endif



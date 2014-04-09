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

	$Id:: module.h 2410 2009-11-20 10:18:18Z benjmitch         $
	$Rev:: 2410                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-20 10:18:18 +0000 (Fri, 20 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_ENGINE_MODULE
#define INCLUDED_ENGINE_MODULE



////////////////	NAMESPACE

namespace brahms
{

	namespace module
	{



	////////////////	MODULE

		/*

			A Module can represent a loaded module of any sort, but
			if it is a component module it will have the "component
			module fields" filled in.

		*/

		struct ModulePaths
		{
			string root;		//	namespace root folder
			string node;		//	node folder
			string release;		//	release folder
		};
		
		const UINT32 F_USE_RTLD_GLOBAL = 0x00000001;
		const UINT32 F_NO_QUERY_VALIDATE = 0x00000002;

		class Module
		{

		public:

			Module(UINT32 flags);
			Module(const Module& src);
			~Module();

			//	interface
			bool load(string path, const ModulePaths& modulePaths, const ExecutionInfo* systemInfo);
			void* map(string name);
			void parseNodeFile(const char* filename);

			//	interface (events)
			Symbol eventINIT(const ExecutionInfo* systemInfo);
			Symbol terminate();

			//	accessors
			const string& getPath();
			EventHandlerFunction* getHandler();
			const ModuleInfo* getInfo();
			const string& getNamespaceRootPath();

			//	just for process modules, this data is read from node.xml
			vector<string> inputSets;
			vector<string> outputSets;


		private:

			//	data common to all modules (component or otherwise)
			string path;							//	absolute path to module file
			void* hModule;							//	OS module handle
			UINT32 flags;

			//	module information exclusive to component modules
			ModulePaths modulePaths;				//	namespace root under which module is found, plus some other stuff
			EventHandlerFunction* eventHandler;		//	pointer to EventHandler function, obtained at load time
			const ModuleInfo* moduleInfo;			//	module info returned from EVENT_MODULE_INIT
			brahms::xml::XMLNode nodeFile;			//	parsed node file, parsed at load time

		};



////////////////	NAMESPACE

	}
}



////////////////	INCLUSION GUARD

#endif


